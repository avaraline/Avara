#import "SoundSystemDefines.h"
#import "FastMat.h"
#import "CSoundHub.h"
#import "AudioFile.h"
#import "CSoundMixer.h"
#import "Resource.h"

//#include <nanogui/nanogui.h>
#include <cstdio>


int main(int argc, const char * argv[]) {
    if (argc < 3) {
        printf("Usage: hsnd2wav [hsnd_num] [outfile] [rsrc]\n");
        return 1;
    }

    if (argc > 3) {
        UseResFile(argv[3]);
    }

    //nanogui::init();
    InitMatrix();

    CSoundHubImpl *soundHub = new CSoundHubImpl;
    soundHub->ISoundHub(32, 32);

    int resId = std::stoi(argv[1]);
    SampleHeaderHandle sample = soundHub->LoadSample(resId);

    if (sample) {
        SampleHeaderPtr sp = *sample;

        int len = sp->loopEnd - sp->loopStart;
        unsigned char *p = sizeof(SampleHeader) + (unsigned char *)sp;

        AudioFile<float> audioFile; // defaults to 16-bit, 44100hz
        audioFile.setBitDepth(16);
        audioFile.setSampleRate(ToFloat(sp->baseRate) * 22254.54545);
        audioFile.setAudioBufferSize(1, len); // 1 channel, num samples
        for (int i = 0; i < len; i++) {
            audioFile.samples[0][i] = p[i+sp->loopStart] / 255.0;
            //audioFile.samples[0][i] = (p[i+sp->loopStart] * 32767) / (0xFF >> (8-BITSPERSAMPLE));
        }
        audioFile.save(argv[2]);
    }
    else {
        SDL_Log("HSND resource not found");
    }

    return 0;
}
