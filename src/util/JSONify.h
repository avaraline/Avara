//
//  JsonPref.h
//  Avara
//
//  Created by Tom on 5/18/23.
//

#pragma once

#ifndef JSONify_h
#define JSONify_h

#include <SDL2/SDL.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdio>   // std::rename

#include <json.hpp>


// This class wraps the templated class with Read() and Write() methods to save
// container classes as JSON files.
// For example, the following code snippet,
//    JSONify<map<string,Record>> myPrefs("example");
// creates a class that is effectively a map of <string,Record> pairs
// that knows how to write itself out to the preferences file named
// "example.json" by calling Write()
//    myPrefs.Write();
//
// You may need to tell nlohmann::json how to serialize some objects.  In the above,
// example, you might do something like this:
//    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Record, name, score, team);
// to tell it which fields you want to save in the Record object.

template<class T>
class JSONify : public T {
    std::string _path;
    bool _backedUp = false;

    void _Backup() {
        if (!_backedUp) {
            std::string backup = _path + ".bak";
            // move the current file to the backup
            std::rename(_path.c_str(), backup.c_str());
            // save the current instance
            Write();
            _backedUp = true;
        }
    }

public:
    JSONify(std::string base) {
        char *ppath = SDL_GetPrefPath("Avaraline", "Avara");
        _path = std::string(ppath) + base + ".json";
        SDL_free(ppath);
        Read();
    }

    void Read() {
        std::ifstream in(_path);
        if (!in.fail()) {
            nlohmann::json j;
            in >> j;
            T::operator=(j.get<T>());  // use the assignment operator of the wrapped/parent class
            // backup only after successfully reading the current json file
            _Backup();
        } else {
            T::operator=({});
        }
    }

    void Write() {
        try {
            std::ostringstream oss;
            nlohmann::json j = *this;
            oss << std::setw(4) << j << std::endl;
            std::ofstream out(_path);
            out << oss.str();
        } catch (std::exception& e) {
            SDL_Log("ERROR WRITING FILE %s: %s", _path.c_str(), e.what());
        }
    }
};

#endif /* JSONify_h */
