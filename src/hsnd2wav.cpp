#define USE_LEGACY_HSND
#include "AudioFile.h"
#include "CHuffProcessor.h"
#include "FastMat.h"
#include "SDL2/SDL.h"
#include "SoundSystemDefines.h"
#include "BasePath.h"

#include <cstdio>

typedef struct {
    uint32_t versNum;
    uint32_t loopStart;
    uint32_t loopEnd;
    uint32_t loopCount;
    uint32_t dataOffset;
    UnsignedFixed baseRate;
} LegacyHSNDRecord;

struct SampleHeader {
    int16_t resId;
    int16_t refCount;
    uint32_t len;
    uint32_t loopStart;
    uint32_t loopEnd;
    uint32_t loopCount;
    UnsignedFixed baseRate;
    struct SampleHeader **nextSample;
    int16_t flags;
};

enum { kOldSampleFlag = 1 };

typedef struct SampleHeader SampleHeader;
typedef SampleHeader *SampleHeaderPtr;
typedef SampleHeaderPtr *SampleHeaderHandle;

static SampleHeaderHandle sampleList = NULL;
static CHuffProcessor *itsCompressor = new CHuffProcessor;

static std::string defaultResource(GetBasePath() + "rsrc/Avara.r");

static std::string currentResource("");

bool IsEquals(const std::string& str1, const std::string& str2) {
    return str1.length() == str2.length() &&
    std::equal(str1.begin(), str1.end(), str2.begin(),
    [](char a, char b) {
        return tolower(a) == tolower(b);
    });
}

void UseResFile(std::string filename) {
    currentResource.assign(GetBasePath() + filename);
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
                const uint8_t nameLen = SDL_ReadU8(file);
                char* cName = new char[nameLen];
                SDL_RWread(file, cName, nameLen, 1);
                std::string rsrcName(cName, nameLen);
                nameMatch = IsEquals(rsrcName, name);
                delete[] cName;
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

void ReleaseResource(Handle theResource) {
    DisposeHandle(theResource);
}

SampleHeaderHandle LoadSampleLegacy(short resId) {
    SampleHeaderHandle aSample;
    SampleHeaderPtr sampP;

    aSample = sampleList;

    while (aSample) {
        sampP = *aSample;
        if (sampP->resId == resId) {
            sampP->flags = 0;
            break;
        }
        aSample = sampP->nextSample;
    }

    if (!aSample) {
        Handle compressedData;

        compressedData = GetResource(HSOUNDRESTYPE, resId);
        if (compressedData) {
            int len;
            //short tryCount;
            Ptr soundData;
            //float base;
            LegacyHSNDRecord *ir;

            // MoveHHi(compressedData);
            HLock(compressedData);

            ir = (LegacyHSNDRecord *)*compressedData;

            ir->versNum = ntohl(ir->versNum);
            ir->loopStart = ntohl(ir->loopStart);
            ir->loopEnd = ntohl(ir->loopEnd);
            ir->loopCount = ntohl(ir->loopCount);
            ir->dataOffset = ntohl(ir->dataOffset);
            if (ir->versNum >= 2) {
                ir->baseRate = ntohl(ir->baseRate);
                //base = ir->baseRate / 65536.0;
            }
            //else {
            //    base = 1.0;
            //}

            soundData = ir->dataOffset + *compressedData;
            len = itsCompressor->GetUncompressedLen(soundData);

            //SDL_Log("LegacyHSNDRecord versNum=%d, loopStart=%d, loopEnd=%d, loopCount=%d, dataOffset=%d, baseRate=%f, len=%i\n",
            //ir->versNum, ir->loopStart, ir->loopEnd, ir->loopCount, ir->dataOffset, base, len);

            aSample = (SampleHeaderHandle)NewHandle(sizeof(SampleHeader) + len);

            if (aSample) {
                uint8_t value;
                uint8_t *p;
                size_t i;

                sampP = *aSample;
                sampP->resId = resId;
                sampP->refCount = 0;
                sampP->flags = 0;
                sampP->len = len;
                sampP->loopStart = ir->loopStart;
                sampP->loopEnd = ir->loopEnd;
                sampP->loopCount = ir->loopCount;
                sampP->nextSample = sampleList;

                if (ir->versNum < 2) {
                    sampP->baseRate = FIX1;
                } else {
                    sampP->baseRate = ir->baseRate;
                }

                sampleList = aSample;

                HLock((Handle)aSample);
                p = sizeof(SampleHeader) + (unsigned char *)sampP;
                itsCompressor->Uncompress(soundData, (Ptr)p);
                HUnlock((Handle)aSample);

                value = 128 >> (8 - BITSPERSAMPLE);
                for (i = 0; i < len; i++) {
                    value += *p;
                    *p++ = value & (0xFF >> (8 - BITSPERSAMPLE));
                }
            }

            ReleaseResource(compressedData);
        }
    }

    return aSample;
}

void DisposeSamples() {
    SampleHeaderHandle aSample, nextSample, *prevP;

    prevP = &sampleList;

    aSample = sampleList;
    while (aSample) {
        nextSample = (*aSample)->nextSample;
        GetHandleSize((Handle)aSample);
        DisposeHandle((Handle)aSample);
        *prevP = nextSample;
        aSample = nextSample;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: hsnd2wav [hsnd_num] [outfile] [rsrc]\n");
        return 1;
    }

    if (argc > 3) {
        SDL_Log("UseResFile: %s", argv[3]);
        UseResFile(argv[3]);
    }

    char *file_name = argv[2];

    InitMatrix();
    itsCompressor->Open();

    int resId = std::stoi(argv[1]);
    SampleHeaderHandle header = LoadSampleLegacy(resId);

    if (header) {
        SampleHeaderPtr sp = *header;
        int len = sp->len;
        unsigned char *p = sizeof(SampleHeader) + (unsigned char *)sp;

        AudioFile<uint8_t> audioFile;
        audioFile.setSampleRate(ToFloat(sp->baseRate) * 22254.54545);
        audioFile.setAudioBufferSize(1, len); // 1 channel, num samples
        for (int i = 0; i < len; i++) {
            // Avara samples were 7-bit (0-127), introduce some range for The Tools.
            audioFile.samples[0][i] = p[i] * 2;
        }
        SDL_Log("Saving %s", file_name);
        audioFile.printSummary();
        audioFile.save(file_name);
    } else {
        SDL_Log("HSND resource not found");
    }

    itsCompressor->Dispose();
    DisposeSamples();

    return 0;
}
