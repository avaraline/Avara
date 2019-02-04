#include "SVGParser.h"
#include "Types.h"
#include "pugixml.hpp"
#include <iostream>
#include <string>

#include <SDL2/SDL.h>

using std::string;
using std::cout;

const char* node_types[] =
{
    "null", "document", "element", "pcdata", "cdata", "comment", "pi", "declaration"
};

SVGParser::SVGParser() {
    context.pnLoc.v = 0;
    context.pnLoc.h = 0;
    context.pnSize.v = 1;
    context.pnSize.h = 1;

    context.fgColor.red = 0;
    context.fgColor.green = 0;
    context.fgColor.blue = 0;

    context.bgColor.red = 0;
    context.bgColor.green = 0;
    context.bgColor.red = 0;

    callbacks.rectProc = NULL;
};

struct simple_walker: pugi::xml_tree_walker {
    SVGProcs callbacks;

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

            string style = node.attribute("style").value();
            string d = node.attribute("d").value();
            string type = node.attribute("sodipodi:type").value();

            SDL_Log("path style: %s", style.c_str());
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
            for (pugi::xml_node tspan: node.children("tspan")) {

            }
        }
    }

    void handle_arc(pugi::xml_node& node) {
        string cx, cy, rx, ry, start, end;

        cx = node.attribute("sodipodi:cx").value();
        cy = node.attribute("sodipodi:cy").value();
        rx = node.attribute("sodipodi:rx").value();
        ry = node.attribute("sodipodi:ry").value();
        start = node.attribute("sodipodi:start").value();
        end = node.attribute("sodipodi:end").value();

        SDL_Log("arc info: %s %s %s %s %s %s", 
            cx.c_str(), cy.c_str(), rx.c_str(), ry.c_str(), start.c_str(), end.c_str());
    }

    void handle_rect(pugi::xml_node& node) {

        string style = node.attribute("style").value();
        string x =  node.attribute("x").value();
        string y = node.attribute("y").value();
        string width = node.attribute("width").value();
        string height = node.attribute("height").value();
        string ry = node.attribute("ry").value();
        SDL_Log("rect style: %s", style.c_str());
        SDL_Log("rect info: %s %s %s %s %s", x.c_str(), y.c_str(), width.c_str(), height.c_str(), ry.c_str());
        //SDL_Log("");
        struct Rect r = {
            (short)stoi(y),
            (short)stoi(x),
            (short)(stoi(y) + stoi(height)),
            (short)(stoi(x) + stoi(width))
        };
        callbacks.rectProc(&r, stoi(ry));
    }

    void handle_style(const string& style) {
        vector<string> decls = split(style, ";");
    }

    void handle_pcdata(string& value) {
        //SDL_Log("%s", value.c_str());
    }
};


void SVGParser::Parse() {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("levels/blockparty_svg/1500_ICE2_iceboxClassic.pict.svg");
    SDL_Log("Load result: %s", result.description());

    simple_walker walker;
    walker.callbacks = callbacks;
    doc.traverse(walker);
    //doc.child("svg")

};

vector<string> SVGParser::split (const string &s, char delim) {
    vector<string> result;
    stringstream ss (s);
    string item;

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}



