#include "Resource.h"
#include "AvaraDefines.h"
#include "Memory.h"

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

// path separator
#if defined(_WIN32)
#define PATHSEP "\\"
#else
#define PATHSEP "/"
#endif

// files stuff
#define LEVELDIR "levels"
#define SETFILE "set.json"
#define ALFDIR "alf"
#define BSPSDIR "bsps"
#define BSPSEXT ".json"
#define DEFAULTSCRIPT "default.avarascript"

static std::string defaultResource(std::string(SDL_GetBasePath()) + "rsrc/Avara.r");

static std::string currentResource("");

static std::string currentLevelDir("");

void UseLevelFolder(std::string set) {
    currentLevelDir = set;
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
    return std::equal(str1.begin(), str1.end(), str2.begin(),
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
    cf_dir_open(&dir, LEVELDIR);

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

                std::ostringstream ss;
                ss << LEVELDIR << PATHSEP << file_str << PATHSEP << SETFILE;
                if (cf_file_exists(ss.str().c_str())) {
                    // we found a set json file so add it (as version 2)
                    it->full_path = BundlePath(file_str.c_str());
                    level_sets.insert(std::make_pair(file_str, (*it)));
                    set_name_list.push_back(file_str);
                    //SDL_Log("Found SVG level set: %s", file_str.c_str());
                }
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
    std::ifstream setManifestFile(setManifestName.str());
    return nlohmann::json::parse(setManifestFile);
}

nlohmann::json GetManifestJSON(std::string set) {
    std::stringstream setManifestName;
    setManifestName << LEVELDIR << PATHSEP << set << PATHSEP << SETFILE;
    std::ifstream setManifestFile(setManifestName.str());
    if (setManifestFile.fail()) {
        SDL_Log("Couldn't read %s", setManifestName.str().c_str());
        return -1;
    }

    return nlohmann::json::parse(setManifestFile);
}

nlohmann::json LoadHullFromSetJSON(short resId) {
    std::string key = std::to_string(resId);
    nlohmann::json manifest = GetManifestJSON(currentLevelDir);
    if (manifest != -1 && manifest.find("HULL") != manifest.end() &&
        manifest["HULL"].find(key) != manifest["HULL"].end())
        return manifest["HULL"][key];
    else
        manifest = GetDefaultManifestJSON();
        if (manifest.find("HULL") != manifest.end() &&
            manifest["HULL"].find(key) != manifest["HULL"].end())
            return manifest["HULL"][key];
        else
            return manifest["HULL"]["129"];
}

char* GetBSPPath(int resId) {
    char* bspName = NULL;
    std::stringstream relPath;

    // first check for the resource in the levelset directory
    relPath << LEVELDIR << PATHSEP << currentLevelDir << PATHSEP;
    relPath << BSPSDIR << PATHSEP << resId << BSPSEXT;
    bspName = BundlePath(relPath.str().c_str());
    std::ifstream testFile(bspName);
    if (testFile.fail()) {
        bspName = NULL;
    } else {
        SDL_Log("Using BSP file in %s\n", bspName);
    }
    // haven't found the BSP file yet, try the top-level bsps directory
    if (bspName == NULL) {
        relPath.str("");
        relPath << BSPSDIR << PATHSEP << resId << BSPSEXT;
        bspName = BundlePath(relPath.str().c_str());
    }
    return bspName;
}

std::string GetALFPath(std::string alfname) {
    std::stringstream buffa;
    buffa << LEVELDIR << PATHSEP << currentLevelDir << PATHSEP;
    buffa << ALFDIR << PATHSEP << alfname;
    return std::string(BundlePath(buffa.str().c_str()));
}

std::string GetDefaultScriptPath() {
    std::stringstream buffa;
    buffa << LEVELDIR << PATHSEP << currentLevelDir  << PATHSEP;
    buffa << DEFAULTSCRIPT;
    return std::string(BundlePath(buffa.str().c_str()));
}