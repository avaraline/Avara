#import "AudioFile.h"
#import "CSoundHub.h"
#import "CSoundMixer.h"
#import "FastMat.h"
#import "Resource.h"
#include "SDL2/SDL.h"
#import "SoundSystemDefines.h"

#include <cstdio>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: hsnd2wav [hsnd_num] [outfile] [rsrc]\n");
        return 1;
    }

    if (argc > 3) {
        UseResFile(argv[3]);
    }

    char *file_name = argv[2];

    InitMatrix();

    CSoundHubImpl *soundHub = new CSoundHubImpl;
    soundHub->ISoundHub(32, 32);

    int resId = std::stoi(argv[1]);
    SampleHeaderHandle header = soundHub->LoadSample(resId);

    if (header) {
        SampleHeaderPtr sp = *header;
        // use loop length if it exists
        bool loops = true;
        int len = sp->loopEnd - sp->loopStart;
        // else use the total sound length
        if (len < 1) {
            loops = false;
            len = sp->len;
        }
        unsigned char *p = sizeof(SampleHeader) + (unsigned char *)sp;

        AudioFile<float> audioFile;
        audioFile.setBitDepth(16);
        audioFile.setSampleRate(ToFloat(sp->baseRate) * 22254.54545);
        audioFile.setAudioBufferSize(1, len); // 1 channel, num samples
        for (int i = 0; i < len; i++) {
            // SDL_Log("%f", (((double)p[i]) / 127.0f) - 0.5);
            // audioFile.samples[0][i] = (p[i+sp->loopStart] * 32767) / (0xFF >> (8-BITSPERSAMPLE));
            // samples are 8 bit unsigned char
            int loc = i;
            // if the sample loops, start from the loop location
            if (loops)
                loc = i + sp->loopStart;
            // convert to signed double

            audioFile.samples[0][i] = (((uint8_t)p[loc] / 127.0f) - 0.5f) * 2.0f;
        }
        SDL_Log("Saving %s", file_name);
        audioFile.printSummary();
        audioFile.save(file_name);
    } else {
        SDL_Log("HSND resource not found");
    }
    return 0;
}
