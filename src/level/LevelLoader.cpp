/*
    Copyright ©1992-1999, Juri Munkki
    All rights reserved.

    File: LevelLoader.c
    Created: Saturday, December 5, 1992, 13:29
    Modified: Tuesday, October 19, 1999, 00:59
*/

#include "LevelLoader.h"

#include "AbstractRenderer.h"
#include "AssetManager.h"
#include "BasePath.h"
#include "CAvaraGame.h"
#include "CWallActor.h"
#include "FastMat.h"
#include "Memory.h"
#include "Parser.h"
#include "pugixml.hpp"

#include <SDL2/SDL.h>

#include <algorithm>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <regex>

#ifdef __has_include
#  if __has_include(<optional>)                // Check for a standard library
#    include <optional>
#  elif __has_include(<experimental/optional>) // Check for an experimental version
#    include <experimental/optional>           // Check if __has_include is present
#  else                                        // Not found at all
#     error "Missing <optional>"
#  endif
#endif

#define POINTTOUNIT(pt) (pt * 20480 / 9)

typedef struct {
    Fixed v;
    Fixed h;
} FixedPoint2D;

static FixedPoint2D lastArcPoint;
static short lastArcAngle;

static FixedPoint2D lastOvalPoint;
static Fixed lastOvalRadius;

Rect gLastBoxRect;
Fixed gLastBoxRounding;

static FixedPoint2D lastDomeCenter;
static short lastDomeAngle;
static short lastDomeSpan;
static Fixed lastDomeRadius;

static Material palette[4];

Fixed GetDome(Fixed *theLoc, Fixed *startAngle, Fixed *spanAngle) {
    theLoc[0] = lastDomeCenter.h;
    theLoc[2] = lastDomeCenter.v;
    theLoc[3] = 0;
    *startAngle = FDegToOne(FIX(lastDomeAngle));
    *spanAngle = FDegToOne(FIX(lastDomeSpan));

    return lastDomeRadius;
}

Material GetDefaultMaterial() {
    Material defaultMaterial = Material();
    std::optional<ARGBColor> specular = ReadColorVar(iDefaultMaterialSpecular);
    if (specular) {
        defaultMaterial = defaultMaterial.WithSpecular(*specular);
    }
    defaultMaterial = defaultMaterial.WithShininessVar(iDefaultMaterialShininess);
    defaultMaterial = defaultMaterial.WithGlowVar(iDefaultMaterialGlow);
    return defaultMaterial;
}

Material GetBaseMaterial() {
    Material baseMaterial = Material();
    std::optional<ARGBColor> specular = ReadColorVar(iBaseMaterialSpecular);
    if (specular) {
        baseMaterial = baseMaterial.WithSpecular(*specular);
    }
    baseMaterial = baseMaterial.WithShininessVar(iBaseMaterialShininess);
    baseMaterial = baseMaterial.WithGlowVar(iBaseMaterialGlow);
    return baseMaterial;
}

Material GetPixelMaterial() {
    return palette[0];
}

Material GetOtherPixelMaterial() {
    return palette[1];
}

Material GetTertiaryMaterial() {
    return palette[2];
}

Material GetQuaternaryMaterial() {
    return palette[3];
}

void GetLastArcLocation(Fixed *theLoc) {
    theLoc[0] = lastArcPoint.h;
    theLoc[1] = 0;
    theLoc[2] = lastArcPoint.v;
    theLoc[3] = 0;
}

Fixed GetLastOval(Fixed *theLoc) {
    theLoc[0] = lastOvalPoint.h;
    theLoc[1] = 0;
    theLoc[2] = lastOvalPoint.v;
    theLoc[3] = 0;

    // Instead of doubling the radius when parsing ovals, which we don't really do
    // anymore, double it here.
    return FMul(lastOvalRadius, FIX(2));
}

Fixed GetLastArcDirection() {
    return FDegToOne(FIX(lastArcAngle));
}

struct ALFWalker: pugi::xml_tree_walker {
    ALFWalker(uint8_t depth = 0): depth(depth) {};

    virtual bool for_each(pugi::xml_node& node) {
        std::string tag = node.name();
        std::string val;

        switch (node.type()){
            case pugi::node_element:
                handle_element(node, tag);
                break;
            default:
                break;
        }

        return true;
    }

    std::string fix_attr(std::string attr) {
        if (attr.compare("color") == 0) {
            attr = "color.0";
        } else if (attr.compare("material.specular") == 0) {
            attr = "material.0.specular";
        } else if (attr.compare("material.shininess") == 0) {
            attr = "material.0.shininess";
        } else if (attr.compare("material.glow") == 0) {
            attr = "material.0.glow";
        }
        // XML attributes can't have brackets, so we turn light.0.i into light[0].i
        std::regex subscript("\\.(\\d+)");
        return std::regex_replace(attr, subscript, "[$1]");
    }

    std::string quote_attr(std::string attr, std::string value) {
        if (
            attr.compare("text") == 0 ||
            attr.compare("designer") == 0 ||
            attr.compare("information") == 0 ||
            attr.compare("color") == 0 ||
            attr.compare("color[0]") == 0 ||
            attr.compare("color[1]") == 0 ||
            attr.compare("color[2]") == 0 ||
            attr.compare("color[3]") == 0 ||
            (attr.size() > 2 && attr.compare(attr.size() - 2, 2, ".c") == 0) ||
            (attr.size() > 9 && attr.compare(attr.size() - 9, 9, ".specular") == 0)
        ) {
            if (value[0] == '$') {
                return value.substr(1);
            }
            else {
                return std::string("\"") + value + std::string("\"");
            }
        }
        return value;
    }

    void handle_element(pugi::xml_node& node, std::string& name) {
        // Eval ALL node attributes and put them into the symbol table
        // unless it's a 'unique' and then it doesn't make any sense.
        if (name.compare("unique") != 0) {
            handle_set(node);
        }

        read_context(node);

        if (name.compare("map") == 0) handle_map(node);
        else if (name.compare("enum") == 0) handle_enum(node);
        else if (name.compare("unique") == 0) handle_unique(node);
        else if (name.compare("set") == 0) handle_set(node);
        else if (name.compare("script") == 0) handle_script(node);
        else if (name.compare("Dome") == 0) {
            handle_dome(node);
            handle_object(node, name);
        }
        else if (name.compare("WallDoor") == 0 || name.compare("WallSolid") == 0) {
            // WallDoors and WallSolids always use the last Wall.
            handle_wall(node);
            handle_object(node, name);
        }
        else if (name.compare("FreeSolid") == 0 || name.compare("Field") == 0) {
            // FreeSolids and Fields with no custom shape expect/use the last Wall.
            if (node.attribute("shape").empty()) {
                handle_wall(node);
            }
            handle_object(node, name);
        }
        else if (name.compare("Wall") == 0) handle_wall(node);
        else if (name.compare("include") == 0) handle_include(node);
        else handle_object(node, name);
    }

    bool read_context(pugi::xml_node& node) {
        gLastBoxRounding = node.attribute("h").empty() ? 0 : ReadFixedVar("h");

        if (!node.attribute("color").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar("color[0]");
            if (color) {
                palette[0] = palette[0].WithColor(*color);
            }
        }

        if (!node.attribute("color.0").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar("color[0]");
            if (color) {
                palette[0] = palette[0].WithColor(*color);
            }
        }

        if (!node.attribute("color.1").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar("color[1]");
            if (color) {
                palette[1] = palette[1].WithColor(*color);
            }
        }

        if (!node.attribute("color.2").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar("color[2]");
            if (color) {
                palette[2] = palette[2].WithColor(*color);
            }
        }

        if (!node.attribute("color.3").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar("color[3]");
            if (color) {
                palette[3] = palette[3].WithColor(*color);
            }
        }
        
        if (!node.attribute("baseMaterial.specular").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar(iBaseMaterialSpecular);
            if (color) {
                // When baseMaterial properties are set, apply it to all mats.
                palette[0] = palette[0].WithSpecular(*color);
                palette[1] = palette[1].WithSpecular(*color);
                palette[2] = palette[2].WithSpecular(*color);
                palette[3] = palette[3].WithSpecular(*color);
            }
        }
        
        if (!node.attribute("baseMaterial.shininess").empty()) {
            // When baseMaterial properties are set, apply it to all mats.
            palette[0] = palette[0].WithShininessVar(iBaseMaterialShininess);
            palette[1] = palette[1].WithShininessVar(iBaseMaterialShininess);
            palette[2] = palette[2].WithShininessVar(iBaseMaterialShininess);
            palette[3] = palette[3].WithShininessVar(iBaseMaterialShininess);
        }
        
        if (!node.attribute("baseMaterial.glow").empty()) {
            // When baseMaterial properties are set, apply it to all mats.
            palette[0] = palette[0].WithGlowVar(iBaseMaterialGlow);
            palette[1] = palette[1].WithGlowVar(iBaseMaterialGlow);
            palette[2] = palette[2].WithGlowVar(iBaseMaterialGlow);
            palette[3] = palette[3].WithGlowVar(iBaseMaterialGlow);
        }
        
        if (!node.attribute("material.specular").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar("material[0].specular");
            if (color) {
                palette[0] = palette[0].WithSpecular(*color);
            }
        }

        if (!node.attribute("material.0.specular").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar("material[0].specular");
            if (color) {
                palette[0] = palette[0].WithSpecular(*color);
            }
        }

        if (!node.attribute("material.1.specular").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar("material[1].specular");
            if (color) {
                palette[1] = palette[1].WithSpecular(*color);
            }
        }

        if (!node.attribute("material.2.specular").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar("material[2].specular");
            if (color) {
                palette[2] = palette[2].WithSpecular(*color);
            }
        }

        if (!node.attribute("material.3.specular").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar("material[3].specular");
            if (color) {
                palette[3] = palette[3].WithSpecular(*color);
            }
        }
        
        if (!node.attribute("material.shininess").empty()) {
            palette[0] = palette[0].WithShininessVar("material[0].shininess");
        }
        
        if (!node.attribute("material.0.shininess").empty()) {
            palette[0] = palette[0].WithShininessVar("material[0].shininess");
        }
        
        if (!node.attribute("material.1.shininess").empty()) {
            palette[1] = palette[1].WithShininessVar("material[1].shininess");
        }
        
        if (!node.attribute("material.2.shininess").empty()) {
            palette[2] = palette[2].WithShininessVar("material[2].shininess");
        }
        
        if (!node.attribute("material.3.shininess").empty()) {
            palette[3] = palette[3].WithShininessVar("material[3].shininess");
        }
        
        if (!node.attribute("material.glow").empty()) {
            palette[0] = palette[0].WithGlowVar("material[0].glow");
        }
        
        if (!node.attribute("material.0.glow").empty()) {
            palette[0] = palette[0].WithGlowVar("material[0].glow");
        }
        
        if (!node.attribute("material.1.glow").empty()) {
            palette[1] = palette[1].WithGlowVar("material[1].glow");
        }
        
        if (!node.attribute("material.2.glow").empty()) {
            palette[2] = palette[2].WithGlowVar("material[2].glow");
        }
        
        if (!node.attribute("material.3.glow").empty()) {
            palette[3] = palette[3].WithGlowVar("material[3].glow");
        }

        if (!node.attribute("x").empty() && !node.attribute("z").empty() &&
            !node.attribute("w").empty() && !node.attribute("d").empty()) {
            Fixed boxCenterX = ToFixed(ReadDoubleVar("x")),
                  boxCenterZ = ToFixed(ReadDoubleVar("z")),
                  boxWidth = ToFixed(ReadDoubleVar("w")),
                  boxDepth = ToFixed(ReadDoubleVar("d"));
            Fixed boxLeft = boxCenterX - (boxWidth / 2),
                  boxRight = boxLeft + boxWidth,
                  boxTop = boxCenterZ - (boxDepth / 2),
                  boxBottom = boxTop + boxDepth;
            gLastBoxRect.top = boxTop;
            gLastBoxRect.left = boxLeft;
            gLastBoxRect.bottom = boxBottom;
            gLastBoxRect.right = boxRight;
        }

        if (!node.attribute("cx").empty() && !node.attribute("cz").empty()) {
            Fixed centerX = ReadFixedVar("cx"),
                  centerZ = ReadFixedVar("cz");

            lastArcPoint.h = centerX;
            lastArcPoint.v = centerZ;

            lastDomeCenter.h = centerX;
            lastDomeCenter.v = centerZ;

            lastOvalPoint.h = centerX;
            lastOvalPoint.v = centerZ;
        }

        if (!node.attribute("r").empty()) {
            Fixed radius = ReadFixedVar("r");
            lastDomeRadius = radius;
            lastOvalRadius = radius;
        }

        if (!node.attribute("angle").empty()) {
            short arcAngle = ReadShortVar("angle");

            lastArcAngle = (900 - arcAngle) % 360;
            lastDomeAngle = 360 - arcAngle;
        } else {
            lastArcAngle = 0;
            lastDomeAngle = 0;
        }

        return true;
    }

    void handle_map(pugi::xml_node& node) {
    }

    void handle_dome(pugi::xml_node& node) {
        if (!node.attribute("quarters").empty()) {
            short extent = ReadShortVar("quarters") * 90;
            lastDomeSpan = (extent >= 90 && extent <= 360)
                ? extent
                : 360;
        } else {
            lastDomeSpan = 360;
        }
    }

    void handle_enum(pugi::xml_node& node) {
        std::stringstream script;
        script << "enum " << node.attribute("start").value() << " " << node.attribute("vars").value() << " end";
        RunThis(script.str());
    }

    void handle_unique(pugi::xml_node& node) {
        std::stringstream script;
        script << "unique " << node.attribute("vars").value() << " end";
        RunThis(script.str());
    }

    void handle_set(pugi::xml_node& node) {
        std::stringstream script;
        bool wrote = false;
        for (pugi::xml_attribute_iterator ait = node.attributes_begin(); ait != node.attributes_end(); ++ait) {
            std::string attr = fix_attr(ait->name());
            std::string value = quote_attr(attr, ait->value());
            script << attr << " = " << value << "\n";
            wrote = true;
        }
        std::string result = script.str();
        if (wrote && result.length() > 0)
            RunThis(result);
    }

    void handle_script(pugi::xml_node& node) {
        RunThis(std::string(node.child_value()));
    }

    void handle_wall(pugi::xml_node& node) {
        Fixed y_alt = 0;
        std::string y = node.attribute("y").value();
        if (!y.empty()) {
            try {
                double yvalue = std::stod(y);
                y_alt = ToFixed(yvalue);
            }
            catch (const std::invalid_argument& e) {
                // temporary script to evaluate expression
                std::stringstream script;
                std::string key = "privateAlf.tmpY";
                script << key << " = " << y << "\n";
                RunThis(script.str());
                auto index = IndexForEntry(key.c_str());
                // immediately evaluate temporary variable
                y_alt = ToFixed(EvalVariable(index + firstVariable, true));
            }
            catch (const std::out_of_range& e) { }
        }
        CWallActor *theWall = new CWallActor;
        theWall->MakeWallFromRect(&gLastBoxRect, gLastBoxRounding, y_alt, 0, true);
    }

    void handle_include(pugi::xml_node& node) {
        std::string path = node.attribute("alf").value();
        if (depth < 5 &&
            // Relative paths only.
            !path.empty() &&
            path.rfind("..", 1) != 0 &&
            path.rfind("/", 0) != 0 &&
            path.rfind("./", 1) != 0 &&
            path.rfind("\\", 0) != 0 &&
            path.rfind(".\\", 1) != 0) {
            // Ensure path separators are appropriate for the current platform.
            std::regex pattern("\\\\|/");
            path = std::regex_replace(path, pattern, PATHSEP);

            std::optional<std::string> maybePath = AssetManager::GetResolvedAlfPath(path);
            if (maybePath) {
                pugi::xml_document shard;
                pugi::xml_parse_result result = shard.load_file(maybePath->c_str());
                if (result) {
                    ALFWalker includeWalker(depth + 1);
                    shard.traverse(includeWalker);
                }
            }
        }
    }

    void handle_object(pugi::xml_node& node, std::string& name) {
        std::stringstream script;
        script << "object " << name << "\n";
        for (pugi::xml_attribute_iterator ait = node.attributes_begin(); ait != node.attributes_end(); ++ait) {
            std::string attr = fix_attr(ait->name());
            std::string value = quote_attr(attr, ait->value());
            script << attr << " = " << value << "\n";
        }
        script << "end";
        RunThis(script.str());
    }

private:
    uint8_t depth;
};

bool LoadALF(std::string levelPath) {
    gRenderer->ResetLights();
    InitParser();
    palette[0] = GetDefaultMaterial();
    palette[1] = GetDefaultMaterial();
    palette[2] = Material((*ColorManager::getMarkerColor(2)).WithA(0xff));
    palette[3] = Material((*ColorManager::getMarkerColor(3)).WithA(0xff));

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(levelPath.c_str());

    ALFWalker walker;
    doc.traverse(walker);

    FreshCalc();
    gCurrentGame->EndScript();
    return (bool)result;
}
