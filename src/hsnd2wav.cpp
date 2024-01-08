#define USE_LEGACY_HSND
#include "AudioFile.h"
#include "CHuffProcessor.h"
#include "FastMat.h"
#include "Resource.h"
#include "SDL2/SDL.h"
#include "SoundSystemDefines.h"

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
