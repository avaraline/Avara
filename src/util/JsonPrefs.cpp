//
//  JsonPrefs.cpp
//  Avara
//
//  Created by Tom on 5/18/23.
//

#include "JsonPrefs.h"


template<class T>
void JsonPrefs<T>::Read() {
    std::ifstream in(Path());
    if (!in.fail()) {
        nlohmann::json j;
        in >> j;
        prefs = j.get<T>();
    } else {
        prefs = {};
    }
}

template<class T>
void JsonPrefs<T>::Write() {
    try {
        std::ostringstream oss;
        nlohmann::json j = prefs;
        oss << std::setw(4) << j << std::endl;
        std::ofstream out(Path());
        out << oss.str();
    } catch (std::exception& e) {
        SDL_Log("ERROR WRITING FILE %s: %s", Path().c_str(), e.what());
    }
}

