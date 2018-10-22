#include "SVGParser.h"
#include "Types.h"
#include "pugixml.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <stdlib.h>

#include <SDL2/SDL.h>

using std::string;
using std::cout;
using std::stringstream;

const char* node_types[] =
{
    "null", "document", "element", "pcdata", "cdata", "comment", "pi", "declaration"
};

SVGParser::SVGParser() {
    context.pnLoc.v = 0;
    context.pnLoc.h = 0;
    context.pnSize.v = 1;
    context.pnSize.h = 1;

    callbacks.rectProc = NULL;
};

struct simple_walker: pugi::xml_tree_walker {
    SVGProcs* callbacks;
    SVGContext context;

    virtual bool for_each(pugi::xml_node& node) {
        //for (int i = 0; i < depth(); ++i)
        //  cout << " ";

        int node_type = node.type();
        string name = node.name();
        string value = node.value();

        //cout << node_types[node_type] << ": name=" << name << " value=" << value << "\n";

        switch (node_type){
            case 2: // element
                handle_element(node, name);
                break;
            case 3: // pcdata (text)
                handle_pcdata(value);
                break;
        }
        return true;
    }


    void handle_element(pugi::xml_node& node, string& name) {
        if(name == "path") {

            string d = node.attribute("d").value();
            string type = node.attribute("sodipodi:type").value();

            //SDL_Log("path style: %s", style.c_str());
            //SDL_Log("path d: %s", d.c_str());
            //SDL_Log("path type: %s", type.c_str());
            if (type.compare("arc") == 0) { 
                handle_arc(node);
            }
        }
        if(name == "rect") {
            handle_rect(node);
        }

        if (name == "text") {
            //SDL_Log("Text Group");
            //for (pugi::xml_node tspan: node.children("tspan")) {
            //    SDL_Log("%s", tspan.value());
            //}
            //SDL_Log("End Text Group");
        }
    }

    void handle_arc(pugi::xml_node& node) {
        string style, cx, cy, rx, ry, start, end;

        style = node.attribute("style").value();
        cx = node.attribute("sodipodi:cx").value();
        cy = node.attribute("sodipodi:cy").value();
        rx = node.attribute("sodipodi:rx").value();
        ry = node.attribute("sodipodi:ry").value();
        start = node.attribute("sodipodi:start").value();
        end = node.attribute("sodipodi:end").value();

        //SDL_Log("arc info: %s %s %s %s %s %s", 
        //    cx.c_str(), cy.c_str(), rx.c_str(), ry.c_str(), start.c_str(), end.c_str());
    }

    void handle_rect(pugi::xml_node& node) {

        string style = node.attribute("style").value();
        string x =  node.attribute("x").value();
        string y = node.attribute("y").value();
        string width = node.attribute("width").value();
        string height = node.attribute("height").value();
        string ry = node.attribute("ry").value();
        //SDL_Log("rect style: %s", style.c_str());
        handle_style(style);
        //SDL_Log("rect info: %s %s %s %s %s", x.c_str(), y.c_str(), width.c_str(), height.c_str(), ry.c_str());
        //SDL_Log("");
        struct Rect r = {
            (short)stoi(y),
            (short)stoi(x),
            (short)(stoi(y) + stoi(height)),
            (short)(stoi(x) + stoi(width))
        };
        callbacks->rectProc(&context, &r, stoi(ry));
    }

    void handle_style(const string& style) {
        vector<string> decls = split(style, ';');
        for(string decl : decls) {
            //SDL_Log("%s", decl.c_str());
            vector<string> key_value = split(decl, ':');
            string key = key_value[0];
            string value = key_value[1];
            if (key.compare("fill") == 0) {
                //RGBColor *fg_color = 
                hex_to_rgbcolor(value, true);
                //SDL_Log("New fgColor");
            }
            if (key.compare("stroke") == 0) {
                hex_to_rgbcolor(value, false);
                //SDL_Log("New bgColor");
            }
            if (key.compare("stroke-width") == 0) {
                unsigned short thickness = 
                    (unsigned short)strtol(value.c_str(), NULL, 10);
            }
            SDL_Log("%s : %s", key.c_str(), value.c_str());
        }
    }

    void hex_to_rgbcolor(string& hex, bool fg) {
        //SDL_Log("HexWhole: %s", hex.c_str());
        unsigned short r, g, b;

        string rhex = hex.substr(1, 2);
        string ghex = hex.substr(3, 2);
        string bhex = hex.substr(5, 2);

        //SDL_Log("Hex: %s %s %s", rhex.c_str(), ghex.c_str(), bhex.c_str());

        r = (unsigned short)strtol(rhex.c_str(), NULL, 16) / 255 * 65535;
        g = (unsigned short)strtol(ghex.c_str(), NULL, 16) / 255 * 65535;
        b = (unsigned short)strtol(bhex.c_str(), NULL, 16) / 255 * 65535;

        //SDL_Log("RGB: %d %d %d", r, g, b);

        callbacks->colorProc(r, g, b, fg);
    }

    void handle_pcdata(string& value) {
        //SDL_Log("%s", value.c_str());
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
    pugi::xml_parse_result result = doc.load_file("levels/blockparty_svg/1500_ICE2_iceboxClassic.pict.svg");
    SDL_Log("Load result: %s", result.description());

    simple_walker walker;
    walker.callbacks = &callbacks;
    doc.traverse(walker);
    //doc.child("svg")

};





