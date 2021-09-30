/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CSoundMixer.h
    Created: Friday, December 23, 1994, 08:24
    Modified: Monday, August 19, 1996, 07:50
*/

#pragma once
#include "CDirectObject.h"
#include "FastMat.h"
#include "SoundSystemDefines.h"

#include <SDL2/SDL.h>

enum {
    rate48khz = (int)0xBB800000, /*48000.00000 in fixed-point*/
    rate44khz = (int)0xAC440000, /*44100.00000 in fixed-point*/
    rate32khz = 0x7D000000, /*32000.00000 in fixed-point*/
    rate22050hz = 0x56220000, /*22050.00000 in fixed-point*/
    rate22khz = 0x56EE8BA3, /*22254.54545 in fixed-point*/
    rate16khz = 0x3E800000, /*16000.00000 in fixed-point*/
    rate11khz = 0x2B7745D1, /*11127.27273 in fixed-point*/
    rate11025hz = 0x2B110000, /*11025.00000 in fixed-point*/
    rate8khz = 0x1F400000 /* 8000.00000 in fixed-point*/
};

#define CYCLEMILLISECS 1000L
//(5/*hours*/ * 60/*minutes*/ * 60L/*seconds*/ * 1000L /*milli*/)

class CBasicSound;
class CSoundMixer;
class CSoundHub;

typedef struct {
    CBasicSound *active;
    CBasicSound *intro;
    CBasicSound *introBackup;
    CBasicSound *release;
    short volume;
    short isFree;
} MixerInfo;

typedef struct {
    Vector loc; //  Position at t = 0
    Vector speed; //  Linear motion
} SoundMotionInfo;

class CSoundMixer : public CDirectObject {
public:
    int globRegister;
    int volumeMax;
    unsigned int frameCounter;

    int16_t *currentBuffer;
    int16_t *doubleBuffers[2];

    SDL_AudioDeviceID outputDevice;

    WordSample *mixTo[2];
    WordSample *mixBuffers[2];
    SampleConvert *volumeLookup;
    Ptr scaleLookup;
    Ptr scaleLookupZero;

    MixerInfo dummyChannel;
    MixerInfo *infoTable;
    MixerInfo **sortSpace[2];

    uint64_t baseTime;

    int frameTime;
    int frameStartTime;
    int frameEndTime;

    UnsignedFixed samplingRate;
    UnsignedFixed standardRate;
    Fixed timeConversion;
    Fixed soundSpeed;
    Fixed distanceToSamples;
    Fixed distanceAdjust;
    Fixed distanceToLevelOne;
    Fixed maxAdjustedVolume;

    CSoundHub *motionHub;
    SoundMotionInfo motion;
    SoundLink *motionLink;
    SoundLink *altLink;
    Boolean useAltLink;

    char newRightMeta;
    Vector newRight;

    Vector rightVector;

    short minimumVolume;
    short maxChannels;
    short maxMix;

    short soundBufferSize;
    short soundBufferBits;

    Boolean hushFlag;
    Boolean sample16flag;
    Boolean stereo;
    Boolean strongStereo;

    void ISoundMixer(Fixed sampRate,
        short maxChannelCount,
        short maxMixCount,
        Boolean stereoEnable,
        Boolean sample16Enable,
        Boolean interpolateEnable);

    void SetSoundEnvironment(Fixed speedOfSound, Fixed distanceToLevelOne, int timeUnit);
    void SetStereoSeparation(Boolean strongFlag);

    void StartTiming();
    int ReadTime();

    void UpdateRightVector(Fixed *right);

    void PrepareScaleLookup();
    void PrepareVolumeLookup(uint8_t mixerVolume = 100);
    void SetVolume(uint8_t volume);  // 0-100
    void SilenceBuffers();
    void UpdateMotion();

    void DoubleBack(uint8_t *stream, int size);

    virtual void Dispose() override;
    void HouseKeep();
    void AddSound(CBasicSound *theSound);
};
