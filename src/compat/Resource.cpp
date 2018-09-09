#include "Resource.h"

#include "Memory.h"

#include <SDL2/SDL.h>
#include <stdint.h>
#include <string>

static std::string defaultResource(std::string(SDL_GetBasePath()) + "rsrc/Avara.r");

static std::string currentResource("");

void UseResFile(std::string filename) {
    currentResource.assign(std::string(SDL_GetBasePath()) + filename);
}

std::string OSTypeString(OSType t) {
    uint32_t x = htonl(t);
    return std::string((char *)&x, 4);
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
                nameMatch = rsrcName == name;
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
        if ((file = SDL_RWFromFile(currentResource.c_str(), "rb"))) {
            data = FindResource(file, theType, theID, theName);
            SDL_RWclose(file);
        }
    }

    // If there is no current resource file, or the resource wasn't found there, look in the default file.
    if (data == NULL) {
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
