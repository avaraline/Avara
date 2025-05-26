/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CSoundMixer.c
    Created: Friday, December 23, 1994, 08:25
    Modified: Tuesday, August 20, 1996, 00:20
*/

#include "CSoundMixer.h"

#include "CBasicSound.h"
#include "CSoundHub.h"
#include "Memory.h"
#include "System.h"

#define DONT_USE_MINIMUM_VOLUME

void AudioCallback(void *userData, uint8_t *stream, int size) {
    CSoundMixer *theMaster = (CSoundMixer *)userData;
    theMaster->DoubleBack(stream, size);
}

void CSoundMixer::StartTiming() {
    baseTime = SDL_GetTicks();
}

int CSoundMixer::ReadTime() {
    return SDL_GetTicks() - baseTime;
}

void CSoundMixer::SetSoundEnvironment(Fixed speedOfSound, Fixed distanceToOne, int timeUnit) {
    if (speedOfSound) {
        soundSpeed = speedOfSound;
        distanceToSamples = (unsigned int)samplingRate / (unsigned int)soundSpeed;
    }

    if (timeUnit) {
        timeConversion = 65536L / timeUnit;
    } else {
        timeConversion = 65536; //  Assume time unit is milliseconds.
    }

    if (distanceToOne) {
        distanceToLevelOne = distanceToOne;
        distanceToOne >>= 4;
        distanceAdjust = FMul(distanceToOne, distanceToOne);
    }

    if (stereo)
        maxAdjustedVolume = MAXADJUSTEDVOLUME;
    else
        maxAdjustedVolume = MAXADJUSTEDVOLUME / 2;
}

/*
**  Since what you hear depends very much on what kind of environment
**  is used to listen to the audio, two different stereo environments
**  are provided.
**
**  Strong stereo (strongStereo = true) is provided for users who have
**  speakers. Since both ears can hear the sound from speakers, the
**  stereo separation can be very clear as follows:
**
**  Strong stereo:  Sound position: LEFT        MIDDLE      RIGHT
**  (Speakers)      Left channel    1.0         0.5         0.0
**                  Right channel   0.0         0.5         1.0
**
**  Users with headphones can only hear the left channel with the
**  left ear and the right channel with the right ear. It seems the
**  human hearing system doesn't like hearing sounds from just one
**  ear (it gives me a headache), so even when the signal is coming
**  from the left side, the channel will still play it at one third
**  of the volume on the left and vice versa.
**
**  Weak stereo:    Sound position: LEFT        MIDDLE      RIGHT
**  (Headphones)    Left channel    1.0         0.66        0.33
**                  Right channel   0.33        0.66        1.0
*/

void CSoundMixer::SetStereoSeparation(Boolean strongFlag) {
    strongStereo = strongFlag;
}

void CSoundMixer::UpdateRightVector(Fixed *right) {
    //  Lock with metaphor
    newRightMeta = false;
    newRight[0] = right[0];
    newRight[1] = right[1];
    newRight[2] = right[2];

    //  Announce that it is ok to access these.
    newRightMeta = true;
}

void CSoundMixer::ISoundMixer(Fixed sampRate,
    short maxChannelCount,
    short maxMixCount,
    Boolean stereoEnable,
    Boolean sample16Enable,
    Boolean interpolateEnable) {
    //OSErr iErr;
    //int globTemp;
    short i, j;
    Vector rightVect = {FIX1, 0, 0, 0};

    sample16flag = true;

    // TODO: bump this to 48khz?
    samplingRate = sampRate ? sampRate : rate22khz;

    if (samplingRate == rate22khz) {
        standardRate = FIX1;
    } else {
        standardRate = FDivNZ(rate22050hz, samplingRate >> 1) >> 1;
    }

    soundBufferBits = BASESOUNDBUFFERBITS;

    if (samplingRate > rate22khz) {
        soundBufferBits++;
    }

    soundBufferSize = 1 << soundBufferBits;

    mixBuffers[0] = (WordSample *)NewPtr(2 * soundBufferSize * sizeof(WordSample));
    mixBuffers[1] = mixBuffers[0] + soundBufferSize;

    frameTime = FMulDivNZ(soundBufferSize, FIX(500), samplingRate >> 1);

    SetSoundEnvironment(FIX(343), FIX1, 1);

    UpdateRightVector(rightVect);
    motion.loc[0] = motion.loc[1] = motion.loc[2] = 0;
    motion.speed[0] = motion.speed[1] = motion.speed[2] = 0;

    motionLink = NULL;
    altLink = NULL;
    useAltLink = false;

    maxChannels = maxChannelCount;
    maxMix = maxMixCount;

    if (maxMix > maxChannels)
        maxMix = maxChannels;

    stereo = stereoEnable;
    strongStereo = false;
    hushFlag = false;

    infoTable = (MixerInfo *)NewPtr(sizeof(MixerInfo) * maxChannels);
    sortSpace[0] = (MixerInfo **)NewPtr(sizeof(MixerInfo *) * (maxChannels + 1) * 2);
    sortSpace[1] = sortSpace[0] + maxChannels + 1;

    for (i = 0; i < maxChannels; i++) {
        infoTable[i].intro = NULL;
        infoTable[i].introBackup = NULL;
        infoTable[i].active = NULL;
        infoTable[i].release = NULL;
        infoTable[i].volume = 0;
        infoTable[i].isFree = true;

        for (j = 0; j < 2; j++)
            sortSpace[j][i + 1] = &infoTable[i];
    }

    if (sample16flag) {
        scaleLookup = NULL;

#ifndef DONT_USE_MINIMUM_VOLUME
        if (soundCapabilities & gestalt16BitSoundIO)
            minimumVolume = 0;
        else
            minimumVolume = MINIMUM8BITVOLUME;
#endif
    } else {
        PrepareScaleLookup();
#ifndef DONT_USE_MINIMUM_VOLUME
        minimumVolume = MINIMUM8BITVOLUME;
#endif
    }

    volumeLookup = (SampleConvert *)NewPtr(sizeof(SampleConvert) * VOLUMERANGE);
    PrepareVolumeLookup();

    sortSpace[0][0] = &dummyChannel;
    sortSpace[1][0] = &dummyChannel;

    dummyChannel.volume = 32767;
    volumeMax = VOLUMERANGE;

    StartTiming();

    size_t bufferRAM = sizeof(WordSample) * 2 * soundBufferSize;
    doubleBuffers[0] = (WordSample *)NewPtr(bufferRAM);
    doubleBuffers[1] = (WordSample *)NewPtr(bufferRAM);

    SilenceBuffers();

    frameCounter = -2;

    SDL_AudioSpec want, have;

    SDL_memset(&want, 0, sizeof(want));
    want.freq = samplingRate / 65536;
    want.format = AUDIO_S16;
    want.channels = 2;
    want.samples = soundBufferSize;
    want.callback = AudioCallback;
    want.userdata = this;
    outputDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    SDL_Log("Sound device (id=%d): format=%d channels=%d samples=%d size=%d\n",
        outputDevice,
        want.format,
        want.channels,
        want.samples,
        want.size);
    SDL_PauseAudioDevice(outputDevice, 0);
}

void CSoundMixer::PrepareScaleLookup() {
    int i;
    int value;
    Sample *output;
    int scaleLookupSize;

    scaleLookupSize = sizeof(Sample) << (VOLUMEBITS + BITSPERSAMPLE - 1);
    scaleLookup = NewPtr(scaleLookupSize);
    output = (Sample *)scaleLookup;
    scaleLookupSize >>= 1;
    scaleLookupZero = scaleLookup + scaleLookupSize;

    for (i = -scaleLookupSize; i < scaleLookupSize; i++) {
        value = (i + (i >> 1)) >> (BITSPERSAMPLE + VOLUMEBITS - 10);
        value = (value + 257) >> 1;
        if (value > 255)
            value = 255;
        else if (value < 0)
            value = 0;

        *output++ = value;
    }
}

void CSoundMixer::PrepareVolumeLookup(uint8_t mixerVolume /* 0-100 */) {
    WordSample *dest;
    short vol, samp;

    dest = &volumeLookup[0][0];

    if (sample16flag) {
        for (vol = 1; vol <= VOLUMERANGE; vol++) {
            for (samp = -SAMPLERANGE / 2; samp < SAMPLERANGE / 2; samp++) {
                *dest++ = (samp * vol * mixerVolume / 100) << (16 - BITSPERSAMPLE - VOLUMEBITS);
            }
        }
    } else {
        for (vol = 1; vol <= VOLUMERANGE; vol++) {
            for (samp = -SAMPLERANGE / 2; samp < SAMPLERANGE / 2; samp++) {
                *dest++ = (samp * vol * mixerVolume / 100);
            }
        }
    }
}

void CSoundMixer::SetVolume(uint8_t volume) {
    PrepareVolumeLookup(std::min(volume, uint8_t(100)));
}

void CSoundMixer::SilenceBuffers() {
    for (int j = 0; j < 2; j++) {
        size_t numChannels = 2;
        memset(doubleBuffers[j], 0, numChannels * soundBufferSize);
    }
}

void CSoundMixer::Dispose() {
    //OSErr iErr;
    short i;
    MixerInfo *mix;

    SDL_PauseAudioDevice(outputDevice, 1);
    SDL_CloseAudioDevice(outputDevice);

    if (motionLink) {
        altLink = NULL;
        useAltLink = true;
        motionHub->ReleaseLink(motionLink);
        motionLink = NULL;
    }

    mix = infoTable;
    for (i = 0; i < maxChannels; i++) {
        if (mix->active)
            mix->active->Release();
        else if (mix->release)
            mix->release->Release();
        else if (mix->intro)
            mix->intro->Release();

        mix++;
    }

    if (mixBuffers[0]) {
        DisposePtr((Ptr)mixBuffers[0]);
        mixBuffers[0] = NULL;
    }

#define OBLITERATE(pointer) \
    if (pointer) { \
        DisposePtr((Ptr)pointer); \
        pointer = NULL; \
    }

    OBLITERATE(doubleBuffers[0])
    OBLITERATE(doubleBuffers[1])
    OBLITERATE(infoTable)
    OBLITERATE(volumeLookup)
    OBLITERATE(scaleLookup)
    OBLITERATE(sortSpace[0])
}

void CSoundMixer::HouseKeep() {
    short i;
    MixerInfo *mix;

    mix = infoTable;
    for (i = 0; i < maxChannels; i++) {
        if (mix->release) {
            mix->release->Release();
            mix->release = NULL;
            mix->isFree = true;
        }
        mix++;
    }
}

void CSoundMixer::AddSound(CBasicSound *theSound) {
    short i;
    MixerInfo *mix;

    mix = infoTable;
    for (i = 0; i < maxChannels; i++) {
        if (mix->isFree) {
            theSound->itsMixer = this;
            mix->isFree = false;
            mix->intro = theSound;
            mix->introBackup = theSound;
            return;
        }
        mix++;
    }

    //  Couldn't find a free slot, so release the sound immediately.
    theSound->Release();
}

void CSoundMixer::UpdateMotion() {
    SoundLink *aLink;

    frameStartTime = ReadTime();
    frameEndTime = frameStartTime + frameTime;

    if (newRightMeta) {
        rightVector[0] = newRight[0];
        rightVector[1] = newRight[1];
        rightVector[2] = newRight[2];
        newRightMeta = false;
    }

    if (useAltLink)
        aLink = altLink;
    else
        aLink = motionLink;

    if (aLink && (aLink->meta == metaNewData)) {
        Fixed *s, *d;
        Fixed t;

        s = aLink->nSpeed;
        d = aLink->speed;
        *d++ = FMul(*s++, timeConversion);
        *d++ = FMul(*s++, timeConversion);
        *d++ = FMul(*s++, timeConversion);

        s = aLink->nLoc;
        d = aLink->loc;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;

        s = aLink->speed;
        d = aLink->loc;
        t = aLink->t;

        *d++ -= *s++ * t;
        *d++ -= *s++ * t;
        *d++ -= *s++ * t;

        aLink->meta = metaNoData;

        s = aLink->loc;
        d = motion.loc;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;

        s = aLink->speed;
        d = motion.speed;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
    }
}

#define DEBUGSOUNDno
#ifdef DEBUGSOUND
short gDebugMixCount;
#endif

void CSoundMixer::DoubleBack(uint8_t *stream, int size) {
    short i;
    MixerInfo *mix;
    MixerInfo **sort, **inSort;
    int volumeTotal;
    short whichStereo;

    // SDL_Log("CSoundMixer::DoubleBack wants %d bytes, soundBufferSize=%d\n", size, soundBufferSize);
    SDL_memset(stream, 0, size);

    UpdateMotion();
    frameCounter++;

    mixTo[0] = mixBuffers[0];
    mixTo[1] = mixBuffers[1]; //  Will never actually be touched by monophonic sounds.

    memset(mixTo[0], 0, soundBufferSize * sizeof(WordSample));
    memset(mixTo[1], 0, soundBufferSize * sizeof(WordSample));

    /*
    **  Activate new channels.
    */
    mix = infoTable;
    i = maxChannels;
    do {
        if (mix->intro && mix->intro == mix->introBackup) {
            mix->active = mix->intro;
            mix->intro = NULL;
            mix->introBackup = NULL;
            mix->active->FirstFrame();
        }

        mix++;
    } while (--i);

#ifdef DEBUGSOUND
    gDebugMixCount = 0;
#endif

    for (whichStereo = 0; whichStereo <= (short)stereo; whichStereo++) {
        short volumeLimit = 0;
        /*
        **  Go through channels, asking the volume for this side.
        */
        mix = infoTable;
        i = maxChannels;
        do {
            if (mix->active) {
                volumeLimit += (mix->volume = mix->active->CalcVolume(whichStereo));
            }

            mix++;
        } while (--i);

        // ignore mixes with volume below this threshold
        volumeLimit /= maxChannels;
        if (volumeLimit) {
            mix = infoTable;
            i = maxChannels;
            do {
                if (mix->active && volumeLimit > mix->volume) {
                    mix->volume = 0;
                }

                mix++;
            } while (--i);
        }

        /*
        **  Insertion sort according to volume.
        */
        sort = sortSpace[whichStereo] + 2;
        i = maxChannels;
        while (--i) {
            inSort = sort++;

            while (inSort[-1]->volume < inSort[0]->volume) {
                mix = inSort[0];
                inSort[0] = inSort[-1];
                inSort[-1] = mix;
                inSort--;
            }
        }

        /*
        **  Select as many of the loudest sounds that fit.
        */
        volumeTotal = 0;
        sort = sortSpace[whichStereo] + 1;

        i = maxMix;
        do {
            mix = *sort;

            if (mix->volume) {
                volumeTotal += mix->volume;

                if (volumeTotal >= volumeMax) {
                    mix->volume += volumeMax - volumeTotal;
                    if (mix->volume)
                        sort++;

                    volumeTotal = volumeMax;
                    break;
                }
            } else
                break;

            sort++;
        } while (--i);

        sort--;
        while (volumeTotal > 0) {
            mix = *sort--;

            volumeTotal -= mix->volume;
            mix->active->WriteFrame(whichStereo, mix->volume);
#ifdef DEBUGSOUND
            gDebugMixCount++;
#endif
        }
    }

    mix = infoTable;
    i = maxChannels;

    if (hushFlag) {
        do {
            if (mix->active) {
                mix->release = mix->active;
                mix->active = NULL;
                mix->volume = 0;
            }
            mix++;
        } while (--i);

        hushFlag = false;
    } else {
        do {
            if (mix->active && mix->active->EndFrame()) {
                mix->release = mix->active;
                mix->active = NULL;
                mix->volume = 0;
            }
            mix++;
        } while (--i);
    }

    InterleaveStereoChannels(mixTo[0], mixTo[1], (WordSample *)stream, soundBufferSize);
}
