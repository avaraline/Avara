#include "PackageManifest.h"

PackageManifest::PackageManifest(nlohmann::json json)
{
    if (json.find("REQD") != json.end()) {
        for (auto &item : json["REQD"].items()) {
            nlohmann::json rawReq = item.value();
            std::string pkgPath = rawReq.value("Package", "");
            // TODO: Support version constraints?
            if (!pkgPath.empty() &&
                pkgPath.rfind(".", 0) != 0 &&
                pkgPath.find("..") == std::string::npos &&
                pkgPath.find("/") == std::string::npos &&
                pkgPath.find("\\") == std::string::npos) {
                PackageRequirement req;
                req.packageName = pkgPath;
                requiredPackages.push_back(req);
            }
        }
    }

    if (json.find("LEDI") != json.end()) {
        for (auto &item : json["LEDI"].items()) {
            nlohmann::json rawLedi = item.value();
            LevelDirectoryEntry level;
            level.levelName = rawLedi.value("Name", "");
            level.levelInfo = rawLedi.value("Message", "");
            level.alfPath = rawLedi.value("Alf", "");
            level.useAftershock = rawLedi.value("Aftershock", false);
            levelDirectory.push_back(level);
        }
    }

    if (json.find("HSND") != json.end()) {
        for (auto const &[idString, rawHsnd] : json["HSND"].items()) {
            int tmp(std::stoi(idString));
            if (tmp >= static_cast<int>(INT16_MIN) && tmp <= static_cast<int>(INT16_MAX)) {
                int16_t id = static_cast<int16_t>(tmp);
                HSNDRecord hsnd;
                hsnd.versNum = rawHsnd.value("Version", 1);
                hsnd.loopStart = rawHsnd.value("Loop Start", 0);
                hsnd.loopEnd = rawHsnd.value("Loop End", 0);
                hsnd.loopCount = rawHsnd.value("Loop Count", 0);
                hsnd.baseRate = ToFixed(rawHsnd.value<float>("Base Rate", 1));
                hsndResources.insert_or_assign(id, hsnd);
            }
        }
    }

    if (json.find("HULL") != json.end()) {
        for (auto const &[idString, rawHull] : json["HULL"].items()) {
            int tmp(std::stoi(idString));
            if (tmp >= static_cast<int>(INT16_MIN) && tmp <= static_cast<int>(INT16_MAX)) {
                int16_t id = static_cast<int16_t>(tmp);
                HullConfigRecord hull;
                hull.hullBSP = static_cast<short>(rawHull["Hull Res ID"]);
                hull.maxMissiles = static_cast<short>(rawHull["Max Missiles"]);
                hull.maxGrenades = static_cast<short>(rawHull["Max Grenades"]);
                hull.maxBoosters = static_cast<short>(rawHull["Max boosters"]);
                hull.mass = ToFixed(rawHull["Mass"]);
                hull.energyRatio = ToFixed(rawHull["Max Energy"]);
                hull.energyChargeRatio = ToFixed(rawHull["Energy Charge"]);
                hull.shieldsRatio = ToFixed(rawHull["Max Shields"]);
                hull.shieldsChargeRatio = ToFixed(rawHull["Shield Charge"]);
                hull.minShotRatio = ToFixed(rawHull["Min Shot"]);
                hull.maxShotRatio = ToFixed(rawHull["Max Shot"]);
                hull.shotChargeRatio = ToFixed(rawHull["Shot Charge"]);
                hull.rideHeight = ToFixed(rawHull["Riding Height"]);
                hull.accelerationRatio = ToFixed(rawHull["Acceleration"]);
                hull.jumpPowerRatio = ToFixed(rawHull["Jump Power"]);
                hullResources.insert_or_assign(id, hull);
            }
        }
    }
}
