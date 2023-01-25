#include "Resource.h"
#include "AvaraDefines.h"
#include "Memory.h"
#include "FastMat.h"
#include "AudioFile.h"

#include <SDL2/SDL.h>
#include <stdint.h>
#include <string>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>

#define CUTE_FILES_IMPLEMENTATION
#include <cute_files.h>

#include <stb_vorbis.h>

static std::string defaultResource(std::string(SDL_GetBasePath()) + "rsrc/Avara.r");

static std::string currentResource("");

static std::string currentLevelDir("");

void UseLevelFolder(std::string set) {
    currentLevelDir = set;
    LoadLevelOggFiles(set);
}

void UseResFile(std::string filename) {
    currentResource.assign(std::string(SDL_GetBasePath()) + filename);
}

std::string OSTypeString(OSType t) {
    uint32_t x = htonl(t);
    return std::string((char *)&x, 4);
}

OSType StringOSType(std::string s) {
    OSType theLevel = 0;
    uint8_t lo = (uint8_t)s[3];
    uint8_t lohi = (uint8_t)s[2];
    uint8_t hilo = (uint8_t)s[1];
    uint8_t hi = (uint8_t)s[0];
    theLevel += lo;
    theLevel += lohi << 8;
    theLevel += hilo << 16;
    theLevel += hi << 24;
    return theLevel;
}

bool IsEquals(const std::string& str1, const std::string& str2) {
    return str1.length() == str2.length() &&
    std::equal(str1.begin(), str1.end(), str2.begin(),
    [](char a, char b) {
        return tolower(a) == tolower(b);
    });
}

Handle FindResource(SDL_RWops *file, OSType theType, short theID, std::string name) {
    uint32_t dataOffset = SDL_ReadBE32(file);
    uint32_t mapOffset = SDL_ReadBE32(file);
    uint32_t dataLen = SDL_ReadBE32(file);
    uint32_t mapLen = SDL_ReadBE32(file);

    SDL_RWseek(file, mapOffset + 22, 0);

    uint16_t forkAttrs = SDL_ReadBE16(file);
    uint16_t typeListOffset = SDL_ReadBE16(file);
    uint16_t nameListOffset = SDL_ReadBE16(file);
    int16_t numTypes = SDL_ReadBE16(file);

    uint32_t offset = mapOffset + typeListOffset + 2;
    while (numTypes >= 0) {
        SDL_RWseek(file, offset, 0); // already read num_types above

        uint32_t rsrcType = SDL_ReadBE32(file);
        int16_t numResources = SDL_ReadBE16(file);
        uint16_t rsrcListOffset = SDL_ReadBE16(file);

        uint32_t rsrcOffset = mapOffset + typeListOffset + rsrcListOffset;
        while (numResources >= 0) {
            SDL_RWseek(file, rsrcOffset, 0);

            uint16_t rsrcId = SDL_ReadBE16(file);
            int16_t nameOffset = SDL_ReadBE16(file);
            uint32_t rsrcInfo = SDL_ReadBE32(file);
            uint32_t rsrcDataOffset = dataOffset + (rsrcInfo & 0x00FFFFFF);
            std::string rsrcName;

            bool nameMatch = false;
            if (nameOffset >= 0 && name != "") {
                // Only read the resource name if we're looking up by name.
                uint32_t rsrcNameOffset = mapOffset + nameListOffset + nameOffset;
                SDL_RWseek(file, rsrcNameOffset, 0);
                uint8_t nameLen = SDL_ReadU8(file);
                char cName[nameLen];
                SDL_RWread(file, cName, nameLen, 1);
                std::string rsrcName(cName, nameLen);
                nameMatch = IsEquals(rsrcName, name);
            }

            if (rsrcType == theType && ((rsrcId == theID) || nameMatch)) {
                SDL_RWseek(file, rsrcDataOffset, 0);
                uint32_t rsrcDataLen = SDL_ReadBE32(file);
                Handle h = NewHandle(rsrcDataLen);
                SDL_RWread(file, *h, rsrcDataLen, 1);
                return h;
            }

            rsrcOffset += 12;
            numResources--;
        }

        offset += 8;
        numTypes--;
    }

    return NULL;
}

Handle _GetResource(OSType theType, short theID, std::string theName) {
    /*
    // TODO: handle TEXT, BSPT (bsp templates)
    // everything else is just checking if it's there and not using the data
    std::string typeString = OSTypeString(theType);
    std::string idString = std::to_string(theID);

    nlohmann::json result = GetKeyFromSetJSON(typeString, idString, 0);
    if (result == -1) return NULL;
    else return NewHandle(0);
    */
    SDL_RWops *file;
    Handle data = NULL;

    // If there is a "current" resource file, look there first.
    if (currentResource.size() > 0) {
        //SDL_Log("Loading %i : %s from %s", theID, theName.c_str(), currentResource.c_str());
        if ((file = SDL_RWFromFile(currentResource.c_str(), "rb"))) {
            data = FindResource(file, theType, theID, theName);
            SDL_RWclose(file);
        }
    }

    // If there is no current resource file, or the resource wasn't found there, look in the default file.
    if (data == NULL) {
        //SDL_Log("Loading %i : %s from Avara resource", theID, theName.c_str());
        if ((file = SDL_RWFromFile(defaultResource.c_str(), "rb"))) {
            data = FindResource(file, theType, theID, theName);
            SDL_RWclose(file);
        }
    }

    return data;

}

Handle GetResource(OSType theType, short theID) {
    return _GetResource(theType, theID, "");
}

Handle GetNamedResource(OSType theType, std::string name) {
    return _GetResource(theType, -1, name);
}

void WriteResource(Handle theResource) {}

void ChangedResource(Handle theResource) {}

void ReleaseResource(Handle theResource) {
    DisposeHandle(theResource);
}

void DetachResource(Handle theResource) {}

void GetIndString(Str255 theString, short strListID, short index) {
    theString[0] = 0;
}

char *BundlePath(const char *rel) {
    char *path = new char[256];
    snprintf(path, 256, "%s%s", SDL_GetBasePath(), rel);
#ifdef _WIN32
    char *c = path;
    while (*c != 0) {
        if (*c == '/') {
            *c = '\\';
        }
        c++;
    }
#endif
    return path;
}

char *BundlePath(std::stringstream &buffa) {
    return BundlePath(buffa.str().c_str());
}

struct AvaraDirListEntry {
    int8_t is_dir;
    std::string file_name;
    char *full_path;
};

std::map<std::string, AvaraDirListEntry> level_sets;
std::vector<std::string> set_name_list;

bool listingDone = false;

std::vector<std::string> LevelDirNameListing() {
    if (!listingDone)
        LevelDirListing();
    return set_name_list;
}

char* PathForLevelSet(std::string set) {
    if (!listingDone)
        LevelDirListing();
    return level_sets.at(set).full_path;
}

void LevelDirListing() {
    cf_dir_t dir;
    const char* ldir = BundlePath(LEVELDIR);
    cf_dir_open(&dir, ldir);
    delete [] ldir;

    std::vector<AvaraDirListEntry> raw_dir_listing;

    while (dir.has_next) {
        cf_file_t file;
        cf_read_file(&dir, &file);
        AvaraDirListEntry entry;
        entry.is_dir = file.is_dir;
        entry.file_name = std::string(file.name);
        raw_dir_listing.push_back(entry);
        cf_dir_next(&dir);
    }
    cf_dir_close(&dir);
    // sort directory listing alphabetically
    std::sort(raw_dir_listing.begin(), raw_dir_listing.end(),
        [](AvaraDirListEntry &a, AvaraDirListEntry &b) -> bool {
            return a.file_name < b.file_name; });

    for (std::vector<AvaraDirListEntry>::iterator it = raw_dir_listing.begin(); it != raw_dir_listing.end(); ++it) {
        auto file_str = it->file_name;
        auto is_dir = it->is_dir;
        if (file_str.size() >= 2) {

            if (file_str.compare(0, 1, ".") != 0 && file_str.compare(0, 2, "..") != 0  &&
                is_dir > 0) {
                // this is a directory, try to see if there's a manifest inside

                std::stringstream ss;
                ss << LEVELDIR << PATHSEP << file_str << PATHSEP << SETFILE;
                char * manf = BundlePath(ss);
                if (cf_file_exists(manf)) {
                    // we found a set json file so add it (as version 2)
                    char * manft = BundlePath(file_str.c_str());
                    it->full_path = manft;
                    level_sets.insert(std::make_pair(file_str, (*it)));
                    set_name_list.push_back(file_str);
                }
                delete [] manf;
            }
        }
    }
    listingDone = true;
};

nlohmann::json LoadLevelListFromJSON(std::string set) {
    return GetManifestJSON(set)["LEDI"];
}

nlohmann::json GetDefaultManifestJSON() {
    std::stringstream setManifestName;
    setManifestName << "rsrc" << PATHSEP << SETFILE;
    std::ifstream setManifestFile(std::string(BundlePath(setManifestName)));
    return nlohmann::json::parse(setManifestFile);
}

nlohmann::json GetManifestJSON(std::string set) {
    if (set.length() < 1) return GetDefaultManifestJSON();
    std::stringstream setManifestName;
    setManifestName << LEVELDIR << PATHSEP << set << PATHSEP << SETFILE;
    std::ifstream setManifestFile(BundlePath(setManifestName));
    if (setManifestFile.fail()) {
        SDL_Log("Couldn't read %s", setManifestName.str().c_str());
        return -1;
    }

    return nlohmann::json::parse(setManifestFile);
}

void LoadHullFromSetJSON(HullConfigRecord *hull, short resId) {
    std::string key = std::to_string(resId);
    nlohmann::json hullJson = GetKeyFromSetJSON("HULL", key, "129");

    hull->hullBSP = (short)hullJson["Hull Res ID"];
    hull->maxMissiles = (short)hullJson["Max Missiles"];
    hull->maxGrenades = (short)hullJson["Max Grenades"];
    hull->maxBoosters = (short)hullJson["Max boosters"];
    hull->mass = ToFixed(hullJson["Mass"]);
    hull->energyRatio = ToFixed(hullJson["Max Energy"]);
    hull->energyChargeRatio = ToFixed(hullJson["Energy Charge"]);
    hull->shieldsRatio = ToFixed(hullJson["Max Shields"]);
    hull->shieldsChargeRatio = ToFixed(hullJson["Shield Charge"]);
    hull->minShotRatio = ToFixed(hullJson["Min Shot"]);
    hull->maxShotRatio = ToFixed(hullJson["Max Shot"]);
    hull->shotChargeRatio = ToFixed(hullJson["Shot Charge"]);
    hull->rideHeight = ToFixed(hullJson["Riding Height"]);
    hull->accelerationRatio = ToFixed(hullJson["Acceleration"]);
    hull->jumpPowerRatio = ToFixed(hullJson["Jump Power"]);
}

char* GetBSPPath(int resId) {
    char* bspName = NULL;
    std::stringstream relPath;

    // first check for the resource in the levelset directory
    relPath << LEVELDIR << PATHSEP << currentLevelDir << PATHSEP;
    relPath << BSPSDIR << PATHSEP << resId << BSPSEXT;
    bspName = BundlePath(relPath);
    std::ifstream testFile(bspName);
    if (testFile.fail()) {
        bspName = NULL;
    } else {
        SDL_Log("Using BSP file in %s\n", bspName);
    }
    // haven't found the BSP file yet, try the top-level bsps directory
    if (bspName == NULL) {
        relPath.str("");
        relPath << RSRCDIR << PATHSEP << BSPSDIR << PATHSEP << resId << BSPSEXT;
        bspName = BundlePath(relPath);
    }
    return bspName;
}

std::string GetALFPath(std::string alfname) {
    std::stringstream buffa;
    buffa << LEVELDIR << PATHSEP << currentLevelDir << PATHSEP;
    buffa << ALFDIR << PATHSEP << alfname;
    return std::string(BundlePath(buffa.str().c_str()));
}

std::string GetDefaultScript() {
    std::stringstream buffa;
    buffa << LEVELDIR << PATHSEP << currentLevelDir  << PATHSEP;
    buffa << DEFAULTSCRIPT;
    auto path = std::string(BundlePath(buffa));
    std::ifstream t(path);
    if (t.good()) {
        std::string defaultscript;
        t.seekg(0, std::ios::end);
        defaultscript.reserve(t.tellg());
        t.seekg(0, std::ios::beg);

        defaultscript.assign((std::istreambuf_iterator<char>(t)),
                              std::istreambuf_iterator<char>());
        return defaultscript;
    }

    return "";
}

std::string GetBaseScript() {
    std::stringstream buffa;
    buffa << "rsrc" << PATHSEP << DEFAULTSCRIPT;
    auto path = std::string(BundlePath(buffa));
    std::ifstream t(path);
    if (t.good()) {
        std::string defaultscript;
        t.seekg(0, std::ios::end);
        defaultscript.reserve(t.tellg());
        t.seekg(0, std::ios::beg);

        defaultscript.assign((std::istreambuf_iterator<char>(t)),
                              std::istreambuf_iterator<char>());
        return defaultscript;
    }
    else {

        SDL_Log("There was an error opening %s", path.c_str());
    }

    return "";
}

nlohmann::json GetKeyFromSetJSON(std::string rsrc, std::string key, std::string default_id) {
    nlohmann::json manifest = GetManifestJSON(currentLevelDir);
    nlohmann::json target = NULL;
    if (manifest != -1 && manifest.find(rsrc) != manifest.end() &&
        manifest[rsrc].find(key)  != manifest[rsrc].end())
        return manifest[rsrc][key];
    else {
        manifest = GetDefaultManifestJSON();
        if (manifest.find(rsrc) != manifest.end() &&
            manifest[rsrc].find(key) != manifest[rsrc].end())
            return manifest[rsrc][key];
        else if (manifest.find(rsrc) != manifest.end() &&
            manifest[rsrc].find(default_id) != manifest[rsrc].end())
            return manifest[rsrc][default_id];
        else
            return -1;
    }
}
#include <math.h>

std::map<short, std::vector<uint8_t>> app_sound_cash;
std::map<short, std::vector<uint8_t>> level_sound_cash;
std::string current_sound_cash_dir;

void LoadOggFile(short resId, std::string filename, std::map<short, std::vector<uint8_t>> &cash) {
    if (cash.count(resId) > 0) return;

    std::stringstream buffa;
    buffa << LEVELDIR << PATHSEP << currentLevelDir << PATHSEP;
    buffa << OGGDIR << PATHSEP << filename;
    char* fullpath = BundlePath(buffa.str().c_str());
    std::ifstream t(fullpath);
    if(!t.good()) {
        std::stringstream temp;
        buffa.swap(temp);
        buffa << RSRCDIR << PATHSEP;
        buffa << OGGDIR << PATHSEP << filename;
        delete [] fullpath;
        fullpath = BundlePath(buffa.str().c_str());
    }

    SDL_Log("Loading %s", fullpath);

    int error;
    stb_vorbis *v = stb_vorbis_open_filename(fullpath, &error, NULL);
    
    stb_vorbis_info info = stb_vorbis_get_info(v);
    SDL_Log("%d channels, %d samples/sec\n", info.channels, info.sample_rate);

    auto sound = std::vector<uint8_t>();

    for(;;) {
        const int buffa_length = 512;
        short buffa[buffa_length];
        int n = stb_vorbis_get_samples_short_interleaved(v, 1, buffa, buffa_length);
        if (n == 0) break;
        for (int i = 0; i < buffa_length; ++i) {
            sound.push_back((buffa[i] + 32768) >> 9);
        }
    }
    cash[resId] = sound;
    stb_vorbis_close(v);
    delete [] fullpath;
}

void LoadDefaultOggFiles() {
    nlohmann::json manifest = GetDefaultManifestJSON();
    if (manifest != -1 && manifest.find("HSND") != manifest.end()) {
        for (auto &hsnd: manifest["HSND"].items()) {
            LoadOggFile(stoi(hsnd.key()), hsnd.value()["Ogg"], app_sound_cash);
        }
    }
}

void LoadLevelOggFiles(std::string set) {
    if (current_sound_cash_dir.compare(set) == 0) return;
    level_sound_cash.clear();
    current_sound_cash_dir = set;
    nlohmann::json manifest = GetManifestJSON(set);
    if (manifest != -1 && manifest.find("HSND") != manifest.end()) {
        for (auto &hsnd: manifest["HSND"].items()) {
            LoadOggFile(stoi(hsnd.key()), hsnd.value()["Ogg"], level_sound_cash);
        }
    }
}

SampleHeaderHandle LoadSampleHeaderFromSetJSON(short resId, SampleHeaderHandle sampleList) {

    std::string key = std::to_string(resId);
    nlohmann::json hsndJson = GetKeyFromSetJSON("HSND", key, "129");
    int version = hsndJson["Version"];
    std::map<short, std::vector<uint8_t>> cash;

    int found = 0;

    if (level_sound_cash.count(resId) > 0) {
        cash = level_sound_cash;
        found++;
    }
    else if (app_sound_cash.count(resId) > 0) {
        cash = app_sound_cash;
        found++;
    }

    if (!found) return NULL;

    Fixed arate = FIX(1);
    if (version > 1)
        arate = ToFixed((float)hsndJson["Base Rate"]);

    SampleHeaderHandle aSample;
    SampleHeaderPtr sampP;
    int len = cash[resId].size();

    aSample = (SampleHeaderHandle)NewHandle(sizeof(SampleHeader) + len);

    sampP = *aSample;
    sampP->baseRate = arate;
    sampP->resId = resId;
    sampP->refCount = 0;
    sampP->flags = 0;
    sampP->loopStart = hsndJson["Loop Start"];
    sampP->loopEnd = hsndJson["Loop End"];
    sampP->loopCount = hsndJson["Loop Count"];
    sampP->nextSample = sampleList;
    sampP->len = len;

    unsigned char *p;
    HLock((Handle)aSample);
    p = sizeof(SampleHeader) + (unsigned char *)sampP;

    for (int i = 0; i < len; ++i) {
        *p++ = cash[resId][i];
    }

    HUnlock((Handle)aSample);
    return aSample;
}
