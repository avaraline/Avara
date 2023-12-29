#include "Resource.h"
#include "AvaraDefines.h"
#include "Memory.h"
#include "FastMat.h"
#include "AudioFile.h"

#include <SDL2/SDL.h>
#include <stdint.h>
#include <algorithm>
#include <cctype>

#ifndef PATH_MAX
#define PATH_MAX 260
#endif

std::string sdlBasePath = "";

static std::string defaultResource(std::string(SDL_GetBasePath()) + "rsrc/Avara.r");

static std::string currentResource("");

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
    // uint32_t dataLen =
    SDL_ReadBE32(file);
    // uint32_t mapLen =
    SDL_ReadBE32(file);

    SDL_RWseek(file, mapOffset + 22, 0);

    //uint16_t forkAttrs =
    SDL_ReadBE16(file);

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

void BundlePath(const char *rel, char *dest) {

    if (sdlBasePath.length() < 1) {
        sdlBasePath = SDL_GetBasePath();
    }
    //char *path = new char[256];
    snprintf(dest, PATH_MAX, "%s%s", sdlBasePath.c_str(), rel);
#ifdef _WIN32
    char *c = dest;
    while (*c != 0) {
        if (*c == '/') {
            *c = '\\';
        }
        c++;
    }
#endif
}

void BundlePath(std::stringstream &buffa, char *dest) {
    return BundlePath(buffa.str().c_str(), dest);
}
