/*
    Copyright ©1992-1999, Juri Munkki
    All rights reserved.

    File: LevelLoader.c
    Created: Saturday, December 5, 1992, 13:29
    Modified: Tuesday, October 19, 1999, 00:59
*/

#include "LevelLoader.h"

#include "AvaraGL.h"
#include "CAvaraGame.h"
#include "CWallActor.h"
#include "FastMat.h"
#include "Memory.h"
#include "Parser.h"
#include "Resource.h"
#include "pugixml.hpp"
#include "RGBAColor.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <sstream>
#include <fstream>
#include <optional>
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

static uint32_t fillColor, frameColor;


Fixed GetDome(Fixed *theLoc, Fixed *startAngle, Fixed *spanAngle) {
    theLoc[0] = lastDomeCenter.h;
    theLoc[2] = lastDomeCenter.v;
    theLoc[3] = 0;
    *startAngle = FDegToOne(((long)lastDomeAngle) << 16);
    *spanAngle = FDegToOne(((long)lastDomeSpan) << 16);

    return lastDomeRadius;
}

uint32_t GetPixelColor() {
    return fillColor;
}

uint32_t GetOtherPixelColor() {
    return frameColor;
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
    return FDegToOne(((long)lastArcAngle) << 16);
}

struct ALFWalker: pugi::xml_tree_walker {
    virtual bool for_each(pugi::xml_node& node) {
        std::string tag = node.name();

        switch (node.type()){
            case pugi::node_element:
                handle_element(node, tag);
                RunThis((StringPtr)node.child_value());
                break;
            default:
                break;
        }

        return true;
    }

    std::string fix_attr(std::string attr) {
        // XML attributes can't have brackets, so we turn light.0.i into light[0].i
        std::regex subscript("\\.(\\d+)");
        return std::regex_replace(attr, subscript, "[$1]");
    }

    std::string quote_attr(std::string attr, std::string value) {
        if (
            attr.compare("text") == 0 ||
            attr.compare("designer") == 0 ||
            attr.compare("information") == 0 ||
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
        // Read any global state we can from the element.
        read_context(node);

        if (name.compare("map") == 0) handle_map(node);
        else if (name.compare("enum") == 0) handle_enum(node);
        else if (name.compare("unique") == 0) handle_unique(node);
        else if (name.compare("set") == 0) handle_set(node);
        else if (name.compare("script") == 0) handle_script(node);
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
        else handle_object(node, name);
    }

    bool read_context(pugi::xml_node& node) {
        std::string fill = node.attribute("fill").value(),
               frame = node.attribute("frame").value(),
               x = node.attribute("x").value(),
               // y = node.attribute("y").value(),
               z = node.attribute("z").value(),
               w = node.attribute("w").value(),
               h = node.attribute("h").value(),
               d = node.attribute("d").value(),
               cx = node.attribute("cx").value(),
               cz = node.attribute("cz").value(),
               r = node.attribute("r").value(),
               angle = node.attribute("angle").value(),
               extent = node.attribute("extent").value();

        gLastBoxRounding = h.empty() ? 0 : ToFixed(std::stod(h));

        if (!fill.empty()) {
            const std::optional<uint32_t> color = ParseColor(fill);
            if (color) {
                fillColor = color.value();
            }
        }

        if (!frame.empty()) {
            const std::optional<uint32_t> color = ParseColor(frame);
            if (color) {
                frameColor = color.value();
            }
        }

        if (!x.empty() && !z.empty() && !w.empty() && !d.empty()) {
            double boxCenterX = std::stod(x) * UNITPOINTS,
                   boxCenterZ = std::stod(z) * UNITPOINTS,
                   boxWidth = std::stod(w) * UNITPOINTS,
                   boxDepth = std::stod(d) * UNITPOINTS,
                   boxLeft = boxCenterX - (boxWidth / 2.0),
                   boxRight = boxLeft + boxWidth,
                   boxTop = boxCenterZ - (boxDepth / 2.0),
                   boxBottom = boxTop + boxDepth;
            gLastBoxRect.top = std::lround(boxTop);
            gLastBoxRect.left = std::lround(boxLeft);
            gLastBoxRect.bottom = std::lround(boxBottom);
            gLastBoxRect.right = std::lround(boxRight);
        }

        if (!cx.empty() && !cz.empty()) {
            Fixed centerX = ToFixed(std::stod(cx)),
                  centerZ = ToFixed(std::stod(cz));

            lastArcPoint.h = centerX;
            lastArcPoint.v = centerZ;

            lastDomeCenter.h = centerX;
            lastDomeCenter.v = centerZ;

            lastOvalPoint.h = centerX;
            lastOvalPoint.v = centerZ;
        }

        if (!r.empty()) {
            Fixed radius = ToFixed(std::stod(r));
            lastDomeRadius = radius;
            lastOvalRadius = radius;
        }

        if (!angle.empty() && !extent.empty()) {
            int arcStart = std::stoi(angle),
                arcExtent = std::stoi(extent);

            lastArcAngle = (720 - (arcStart + arcExtent / 2)) % 360;
            lastDomeAngle = 360 - arcStart;
            lastDomeSpan = arcExtent;
        }

        return true;
    }

    void handle_map(pugi::xml_node& node) {
    }

    void handle_enum(pugi::xml_node& node) {
        std::stringstream script;
        script << "enum " << node.attribute("start").value() << " " << node.attribute("vars").value() << " end";
        RunThis((StringPtr)script.str().c_str());
    }

    void handle_unique(pugi::xml_node& node) {
        std::stringstream script;
        script << "unique " << node.attribute("vars").value() << " end";
        RunThis((StringPtr)script.str().c_str());
    }

    void handle_set(pugi::xml_node& node) {
        std::stringstream script;
        for (pugi::xml_attribute_iterator ait = node.attributes_begin(); ait != node.attributes_end(); ++ait) {
            std::string attr = fix_attr(ait->name());
            std::string value = quote_attr(attr, ait->value());
            script << attr << " = " << value << "\r";
        }
        RunThis((StringPtr)script.str().c_str());
    }

    void handle_script(pugi::xml_node& node) {
        RunThis((StringPtr)node.child_value());
    }

    void handle_wall(pugi::xml_node& node) {
        std::string y = node.attribute("y").value();
        if (!y.empty()) {
            std::stringstream script;
            script << "wa = " << y << "\n";
            RunThis((StringPtr)script.str().c_str());
        }
        CWallActor *theWall = new CWallActor;
        theWall->IAbstractActor();
        theWall->MakeWallFromRect(&gLastBoxRect, gLastBoxRounding, 0, true);
    }

    void handle_object(pugi::xml_node& node, std::string& name) {
        std::stringstream script;
        script << "object " << name << "\n";
        for (pugi::xml_attribute_iterator ait = node.attributes_begin(); ait != node.attributes_end(); ++ait) {
            std::string attr = fix_attr(ait->name());
            std::string value = quote_attr(attr, ait->value());
            // Ignore all the standard context attributes when parsing objects.
            if (
                attr.compare("fill") == 0 ||
                attr.compare("frame") == 0 ||
                attr.compare("x") == 0 ||
                //attr.compare("y") == 0 ||
                attr.compare("z") == 0 ||
                attr.compare("w") == 0 ||
                attr.compare("h") == 0 ||
                attr.compare("d") == 0 ||
                attr.compare("cx") == 0 ||
                attr.compare("cz") == 0 ||
                attr.compare("r") == 0 ||
                attr.compare("angle") == 0 ||
                attr.compare("extent") == 0
            ) continue;
            script << attr << " = " << value << "\n";
        }
        script << "end";
        RunThis((StringPtr)script.str().c_str());
    }
};

bool LoadALF(std::string levelName) {
    AvaraGLLightDefaults();
    InitParser();

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(levelName.c_str());

    ALFWalker walker;
    doc.traverse(walker);

    FreshCalc();
    gCurrentGame->EndScript();
    return (bool)result;
}
