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
                hull.hullBSP = rawHull.value<short>("Hull Res ID", 0);
                hull.maxMissiles = rawHull.value<short>("Max Missiles", 0);
                hull.maxGrenades = rawHull.value<short>("Max Grenades", 0);
                hull.maxBoosters = rawHull.value<short>("Max boosters", 0);
                hull.mass = ToFixed(rawHull.value<float>("Mass", 0));
                hull.energyRatio = ToFixed(rawHull.value<float>("Max Energy", 0));
                hull.energyChargeRatio = ToFixed(rawHull.value<float>("Energy Charge", 0));
                hull.shieldsRatio = ToFixed(rawHull.value<float>("Max Shields", 0));
                hull.shieldsChargeRatio = ToFixed(rawHull.value<float>("Shield Charge", 0));
                hull.minShotRatio = ToFixed(rawHull.value<float>("Min Shot", 0));
                hull.maxShotRatio = ToFixed(rawHull.value<float>("Max Shot", 0));
                hull.shotChargeRatio = ToFixed(rawHull.value<float>("Shot Charge", 0));
                hull.rideHeight = ToFixed(rawHull.value<float>("Riding Height", 0));
                hull.accelerationRatio = ToFixed(rawHull.value<float>("Acceleration", 0));
                hull.jumpPowerRatio = ToFixed(rawHull.value<float>("Jump Power", 0));
                
                hullResources.insert_or_assign(id, hull);
            }
        }
    }
    
    if (json.find("BSPS") != json.end()) {
        for (auto const &[idString, rawBsps] : json["BSPS"].items()) {
            int tmp(std::stoi(idString));
            if (tmp >= static_cast<int>(INT16_MIN) && tmp <= static_cast<int>(INT16_MAX)) {
                int16_t id = static_cast<int16_t>(tmp);
                BSPSRecord bsps;
                bsps.baseSize = ToFixed(rawBsps.value<float>("1:1 size", 1));
                bsps.scaleStyle = rawBsps.value("Stretch/Scale (0/1)", 0);
                bspsResources.insert_or_assign(id, bsps);
            }
        }
    }
}
