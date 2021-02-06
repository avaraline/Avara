#include "JSONLevelDescriptor.h"

#include <SDL2/SDL.h>
#include <fstream>
#include <json.hpp>
#include <string>

nlohmann::json LoadLevelListFromJSON(std::string set) {
    return GetManifestJSON(set)["LEDI"];
}

nlohmann::json GetManifestJSON(std::string set) {
    std::stringstream setManifestName;
    setManifestName << "levels/" << set << "/set.json";
    std::ifstream setManifestFile(setManifestName.str());
    if (setManifestFile.fail()) {
        SDL_Log("Couldn't read %s", setManifestName.str().c_str());
        return -1;
    }

    return nlohmann::json::parse(setManifestFile);
}