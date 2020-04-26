#include "Beeper.h"

#include <SDL.h>
#include <stdint.h>
#include <stdio.h>

SDL_AudioSpec wavSpec;
uint32_t wavLength;
uint8_t *wavBuffer;
SDL_AudioDeviceID beepDevice = 0;
bool beepLoaded = false;

void Beep() {
    if (!beepLoaded) {
        // TODO: backslash on Windows
        char wavPath[256];
        snprintf(wavPath, 256, "%srsrc/%s", SDL_GetBasePath(), "sosumi.wav");
        SDL_memset(&wavSpec, 0, sizeof(wavSpec));
        if (SDL_LoadWAV(wavPath, &wavSpec, &wavBuffer, &wavLength)) {
            beepLoaded = true;
        }
        else {
            SDL_Log("Could not load beep: %s\n", SDL_GetError());
        }
    }
    
    if (!beepDevice) {
        beepDevice = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
        SDL_PauseAudioDevice(beepDevice, 0);
    }

    if (beepLoaded && beepDevice) {
        SDL_QueueAudio(beepDevice, wavBuffer, wavLength);
    }
}
