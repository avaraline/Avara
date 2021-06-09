#define USE_LEGACY_HSND
#import "AudioFile.h"
#import "CSoundHub.h"
#import "CSoundMixer.h"
#import "FastMat.h"
#import "Resource.h"
#include "SDL.h"
#import "SoundSystemDefines.h"

#include <cstdio>

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

    CSoundHubImpl *soundHub = new CSoundHubImpl;
    soundHub->ISoundHub(32, 32);

    int resId = std::stoi(argv[1]);
    SampleHeaderHandle header = soundHub->LoadSampleLegacy(resId);

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
    return 0;
}
