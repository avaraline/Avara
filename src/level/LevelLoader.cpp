/*
    Copyright ©1992-1999, Juri Munkki
    All rights reserved.

    File: LevelLoader.c
    Created: Saturday, December 5, 1992, 13:29
    Modified: Tuesday, October 19, 1999, 00:59
*/

#include "LevelLoader.h"

#include "AssetManager.h"
#include "AvaraGL.h"
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

#define POINTTOUNIT(pt) (pt * 20480 / 9)
#define UNITPOINTS (double)14.4 // 72 / 5

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

static ARGBColor palette[4] = {
    0,
    0,
    (*ColorManager::getMarkerColor(2)).WithA(0xff),
    (*ColorManager::getMarkerColor(3)).WithA(0xff)
};


Fixed GetDome(Fixed *theLoc, Fixed *startAngle, Fixed *spanAngle) {
    theLoc[0] = lastDomeCenter.h;
    theLoc[2] = lastDomeCenter.v;
    theLoc[3] = 0;
    *startAngle = FDegToOne(FIX(lastDomeAngle));
    *spanAngle = FDegToOne(FIX(lastDomeSpan));

    return lastDomeRadius;
}

ARGBColor GetPixelColor() {
    return palette[0];
}

ARGBColor GetOtherPixelColor() {
    return palette[1];
}

ARGBColor GetTertiaryColor() {
    return palette[2];
}

ARGBColor GetQuaternaryColor() {
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
            (attr.size() > 2 && attr.compare(attr.size() - 2, 2, ".c") == 0)
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
        // unless it's a 'unique' and then it doesn't make any sense. If
        // it's a 'TeamColor' (ScoreKeeper plugin) we also want to ignore
        // it just to make sure it doesn't interfere with anything.
        if (name.compare("unique") != 0 && name.compare("TeamColor") != 0) {
            handle_set(node);
        }

        // Read any global state we can from the element, unless it's
        // 'TeamColor' in which case we still want to keep it separate.
        if (name.compare("TeamColor") != 0) {
            read_context(node);
        }

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
        else if (name.compare("TeamColor") == 0) handle_teamcolor(node);
        else if (name.compare("include") == 0) handle_include(node);
        else handle_object(node, name);
    }

    bool read_context(pugi::xml_node& node) {
        gLastBoxRounding = node.attribute("h").empty() ? 0 : ReadFixedVar("h");

        if (!node.attribute("color").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar("color[0]");
            if (color) {
                palette[0] = *color;
            }
        }

        if (!node.attribute("color.0").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar("color[0]");
            if (color) {
                palette[0] = *color;
            }
        }

        if (!node.attribute("color.1").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar("color[1]");
            if (color) {
                palette[1] = *color;
            }
        }

        if (!node.attribute("color.2").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar("color[2]");
            if (color) {
                palette[2] = *color;
            }
        }

        if (!node.attribute("color.3").empty()) {
            const std::optional<ARGBColor> color = ReadColorVar("color[3]");
            if (color) {
                palette[3] = *color;
            }
        }

        if (!node.attribute("x").empty() && !node.attribute("z").empty() &&
            !node.attribute("w").empty() && !node.attribute("d").empty()) {
            double boxCenterX = ReadDoubleVar("x") * UNITPOINTS,
                   boxCenterZ = ReadDoubleVar("z") * UNITPOINTS,
                   boxWidth = ReadDoubleVar("w") * UNITPOINTS,
                   boxDepth = ReadDoubleVar("d") * UNITPOINTS;
            double boxLeft = boxCenterX - (boxWidth / 2.0),
                   boxRight = boxLeft + boxWidth,
                   boxTop = boxCenterZ - (boxDepth / 2.0),
                   boxBottom = boxTop + boxDepth;
            gLastBoxRect.top = std::lround(boxTop);
            gLastBoxRect.left = std::lround(boxLeft);
            gLastBoxRect.bottom = std::lround(boxBottom);
            gLastBoxRect.right = std::lround(boxRight);
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
        std::string y = node.attribute("y").value();
        if (!y.empty()) {
            std::stringstream script;
            script << "wa = " << y << "\n";
            RunThis(script.str());
        }
        CWallActor *theWall = new CWallActor;
        theWall->MakeWallFromRect(&gLastBoxRect, gLastBoxRounding, 0, true);
    }

    void handle_teamcolor(pugi::xml_node& node) {
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
    AvaraGLLightDefaults();
    InitParser();

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(levelPath.c_str());

    ALFWalker walker;
    doc.traverse(walker);

    FreshCalc();
    gCurrentGame->EndScript();
    return (bool)result;
}
