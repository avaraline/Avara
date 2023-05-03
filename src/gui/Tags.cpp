//
//  Tags.cpp
//  Avara
//
//  Created by Tom on 5/1/23.
//

#include "Tags.h"

#include <json.hpp>

#include <SDL2/SDL.h>
#include <fstream>
#include <iostream>
#include <sstream>

Tags::TagMap Tags::_tags = {};

std::string Tags::TagsPath() {
    char *path = SDL_GetPrefPath("Avaraline", "Avara");
    std::string jsonPath = std::string(path) + "tags.json";
    SDL_free(path);
    return jsonPath;
}

bool Tags::ReadTags() {
    std::ifstream in(TagsPath());

    if (!in.fail()) {
        nlohmann::json jtags;
        in >> jtags;
        _tags = jtags.get<TagMap>();
    } else {
        // a couple tags to get started
        AddTagToLevel(LevelURL("avaraline-strict-mode", "I/O"), "fav");
        AddTagToLevel(LevelURL("aa-abnormal", "Arsenic"), "koth");
    }
    return true;
}

void Tags::TrimTags() {
    // remove levels with no tags, and sets with no levels
    std::set<std::string> trimSets;
    for (auto [lset, levels]: _tags) {
        std::set<std::string> trimLevels;
//        std::cout << lset << " has " << levels.size() << " levels" << std::endl;
        for (auto [level, tags]: levels) {
//            std::cout << "  " << level << " has " << tags.size() << " tags" << std::endl;
            if (tags.size() == 0) {
                // remove levels outside of this loop to avoid access errors
                trimLevels.insert(level);
            }
        }
        for (auto trimLevel: trimLevels) {
//            std::cout << "  -removing " << trimLevel << std::endl;
            _tags[lset].erase(trimLevel);
        }
        if (_tags[lset].size() == 0) {
            trimSets.insert(lset);
        }
    }
    for (auto trimSet: trimSets) {
//        std::cout << "-removing " << trimSet << std::endl;
        _tags.erase(trimSet);
    }
}

bool Tags::WriteTags() {
    try {
        std::ostringstream oss;
        TrimTags();
        nlohmann::json jtags = _tags;
        oss << std::setw(4) << jtags << std::endl;
        std::ofstream out(TagsPath());
        out << oss.str();
    } catch (std::exception& e) {
        SDL_Log("ERROR WRITING TAGS FILE=%s", e.what());
        return false;
    }
    return true;
}

const std::set<std::string>& Tags::GetTagsForLevel(LevelURL level) {
    if (Tags::_tags.empty()) {
        ReadTags();
    }
    return _tags[level.first][level.second];
}

std::string Tags::NormalizeTagName(std::string tag) {
    if (tag[0] != '#') {
        tag = '#' + tag;
    }
    return tag;
}

void Tags::AddTagToLevel(LevelURL level, std::string tag) {
    _tags[level.first][level.second].insert(NormalizeTagName(tag));
    WriteTags();
}

void Tags::DeleteTagFromLevel(LevelURL level, std::string tag) {
    _tags[level.first][level.second].erase(NormalizeTagName(tag));
    WriteTags();
}

std::set<Tags::LevelURL> Tags::GetLevelsMatchingTag(std::string matchTag) {
    if (Tags::_tags.empty()) {
        ReadTags();
    }
    std::set<Tags::LevelURL> matchingLevels;
    for (auto [lset, levels]: _tags) {
        for (auto [level, tags]: levels) {
            for (auto tag: tags) {
                if (tag == matchTag) {
                    matchingLevels.insert(LevelURL(lset, level));
                }
            }
        }
    }
    return matchingLevels;
}
