#pragma once
#include "OggFile.h"
#include "PlayerConfig.h"

#include <json.hpp>

#include <map>
#include <string>
#include <vector>

struct PackageRequirement {
    std::string packageName;
};

struct LevelDirectoryEntry {
    std::string levelName;
    std::string levelInfo;
    std::string alfPath;
    bool useAftershock = false;
};

class PackageManifest {
public:
    PackageManifest(nlohmann::json json);

    std::vector<PackageRequirement> requiredPackages = {};
    std::vector<LevelDirectoryEntry> levelDirectory = {};
    std::map<int16_t, HSNDRecord> hsndResources = {};
    std::map<int16_t, HullConfigRecord> hullResources = {};
};
