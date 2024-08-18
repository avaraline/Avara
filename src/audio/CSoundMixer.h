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
    int32_t globRegister = 0;
    int32_t volumeMax = 0;
    uint32_t frameCounter = 0;

    int16_t *currentBuffer = 0;
    int16_t *doubleBuffers[2] = {0};

    SDL_AudioDeviceID outputDevice = 0;

    WordSample *mixTo[2] = {0};
    WordSample *mixBuffers[2] = {0};
    SampleConvert *volumeLookup = 0;
    Ptr scaleLookup = 0;
    Ptr scaleLookupZero = 0;

    MixerInfo dummyChannel = {0, 0, 0, 0, 0, 0};
    MixerInfo *infoTable = 0;
    MixerInfo **sortSpace[2] = {0};

    uint32_t baseTime = 0;

    int32_t frameTime = 0;
    int32_t frameStartTime = 0;
    int32_t frameEndTime = 0;

    UnsignedFixed samplingRate = 0;
    UnsignedFixed standardRate = 0;
    Fixed timeConversion = 0;
    Fixed soundSpeed = 0;
    Fixed distanceToSamples = 0;
    Fixed distanceAdjust = 0;
    Fixed distanceToLevelOne = 0;
    Fixed maxAdjustedVolume = 0;

    CSoundHub *motionHub = 0;
    SoundMotionInfo motion = {{0}, {0}};
    SoundLink *motionLink = 0;
    SoundLink *altLink = 0;
    Boolean useAltLink = false;

    char newRightMeta = 0;
    Vector newRight = {0};

    Vector rightVector = {0};

    int16_t minimumVolume = 0;
    int16_t maxChannels = 0;
    int16_t maxMix = 0;

    int32_t soundBufferSize = 0;
    int16_t soundBufferBits = 0;

    Boolean hushFlag = false;
    Boolean sample16flag = false;
    Boolean stereo = false;
    Boolean strongStereo = false;

    void ISoundMixer(Fixed sampRate,
        int16_t maxChannelCount,
        int16_t maxMixCount,
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
