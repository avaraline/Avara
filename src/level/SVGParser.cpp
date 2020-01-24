#include "SVGParser.h"
#include "FastMat.h"
#include "Types.h"
#include "pugixml.hpp"
#include "csscolorparser.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <stdlib.h>

#include <SDL2/SDL.h>

using std::string;
using std::stol;
using std::stof;
using std::cout;
using std::stringstream;

SVGParser::SVGParser() {
    callbacks.rectProc = NULL;
    callbacks.colorProc = NULL;
    callbacks.textProc = NULL;
    callbacks.arcProc = NULL;
};

struct simple_walker: pugi::xml_tree_walker {
    SVGProcs* callbacks;
    unsigned short thiccness;

    virtual bool for_each(pugi::xml_node& node) {
        int node_type = node.type();
        string name = node.name();

        switch (node_type){
            case 2: // element
                handle_element(node, name);
                break;
        }
        return true;
    }


    void handle_element(pugi::xml_node& node, string& name) {
        if(name.compare("path") == 0) {

            string d = node.attribute("d").value();
            string type = node.attribute("sodipodi:type").value();

            //SDL_Log("path style: %s", style.c_str());
            //SDL_Log("path d: %s", d.c_str());
            //SDL_Log("path type: %s", type.c_str());
            if (type.compare("arc") == 0) { 
                handle_arc(node);
            }
        }
        else if(name.compare("ellipse") == 0) {
            handle_ellipse(node);
        }
        else if(name.compare("rect") == 0) {
            handle_rect(node);
        }
        else if (name.compare("text") == 0) {
            std::stringstream buffa;
            for (pugi::xml_node tspan: node.children("tspan")) {
                buffa << tspan.child_value() << (char)13;
                //SDL_Log(tspan.child_value());
            }
            callbacks->textProc((unsigned char*)buffa.str().c_str());
        }
        else {
            //SDL_Log("Name: %s", name.c_str());
        }
    }

    int safe_int(string value) {
        try { return stoi(value); }
        catch (std::exception e) { return 0; }
    }

    long safe_long(string value) {
        try { return stol(value); }
        catch (std::exception e) { return 0; }
    }

    float safe_float(string value) {
        try { return stof(value); }
        catch (std::exception e) { return 0; }
    }

    void handle_arc(pugi::xml_node& node) {
        string style;
        float cx, cy, rx, ry;
        float start, end;

        style = node.attribute("style").value();
        cx = safe_float(node.attribute("sodipodi:cx").value());
        cy = safe_float(node.attribute("sodipodi:cy").value());
        rx = safe_float(node.attribute("sodipodi:rx").value());
        ry = safe_float(node.attribute("sodipodi:ry").value());
        start = safe_float(node.attribute("sodipodi:start").value());
        end = safe_float(node.attribute("sodipodi:end").value());

        handle_style(style);

        short start_angle = (short)(start * (180.0/PI));
        short angle_length = (short)abs(start_angle - (end* (180.0/PI)));
        //SDL_Log("start_angle: %d angle_length: %d", start_angle, angle_length);
        //SDL_Log("cx: %d cy: %d", cx, cy);
        callbacks->arcProc(cx, cy, start_angle, angle_length, rx > ry ? rx : ry);

        //SDL_Log("arc info: %s %s %s %s %s %s", 
        //    cx.c_str(), cy.c_str(), rx.c_str(), ry.c_str(), start.c_str(), end.c_str());
    }

    void handle_ellipse(pugi::xml_node& node) {
        string style;
        long cx, cy, rx, ry;

        style = node.attribute("style").value();
        cx = safe_long(node.attribute("sodipodi:cx").value());
        cy = safe_long(node.attribute("sodipodi:cy").value());
        rx = safe_long(node.attribute("sodipodi:rx").value());
        ry = safe_long(node.attribute("sodipodi:ry").value());

        handle_style(style);
        callbacks->ellipseProc(cx, cy, rx > ry ? rx : ry);
    }

    void handle_rect(pugi::xml_node& node) {

        string style = node.attribute("style").value();
        int x = safe_int(node.attribute("x").value());
        int y = safe_int(node.attribute("y").value());
        int ry = safe_int(node.attribute("ry").value());
        int width = safe_int(node.attribute("width").value());
        int height = safe_int(node.attribute("height").value());
        //SDL_Log("rect style: %s", style.c_str());
        handle_style(style);
        //SDL_Log("rect info: %s %s %s %s %s", x.c_str(), y.c_str(), width.c_str(), height.c_str(), ry.c_str());
        //SDL_Log("");
        struct Rect r = {
            (short)y,
            (short)x,
            (short)(y + height),
            (short)(x + width)
        };
        callbacks->rectProc(&r, ry, thiccness);
    }

    void handle_style(const string& style) {
        vector<string> decls = split(style, ';');

        for(string decl : decls) {
            //SDL_Log("%s", decl.c_str());
            vector<string> key_value = split(decl, ':');
            string key = key_value[0];
            string value = key_value[1];
            //SDL_Log("%s : %s", key.c_str(), value.c_str());
            if (key.compare("fill") == 0) {
                css_to_rgbcolor(value, true);
                //SDL_Log("New fgColor");
            }
            if (key.compare("stroke") == 0) {
                css_to_rgbcolor(value, false);
                //SDL_Log("New bgColor");
            }
            if (key.compare("stroke-width") == 0) {
                thiccness = (unsigned short)safe_long(value);
            }
        }
    }

    void css_to_rgbcolor(string& css_color, bool fg) {
        const auto color = CSSColorParser::parse(css_color);
        if (color)
        callbacks->colorProc(color->r * 257, color->g * 257, color->b * 257, fg);
    }

    vector<string> split (const string &s, char delim) {
        vector<string> result;
        std::stringstream ss (s);
        string item;

        while (std::getline (ss, item, delim)) {
            trim(item);
            if (item.size() > 0)
            result.push_back(item);
        }

        return result;
    }

    // trim from start (in place)
    static inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
        }));
    }

    // trim from end (in place)
    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    // trim from both ends (in place)
    static inline void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }
};


void SVGParser::Parse() {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("test_pict2svgupdates.svg");
    //pugi::xml_parse_result result = doc.load_file("levels/classic-mix-up_svg/1004_Grim_Grimoire.svg");
    SDL_Log("SVG load result: %s", result.description());

    simple_walker walker;
    walker.callbacks = &callbacks;
    doc.traverse(walker);
    //doc.child("svg")

};





