/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CBasicSound.h
    Created: Friday, December 23, 1994, 09:53
    Modified: Saturday, February 24, 1996, 09:24
*/

#pragma once
#include "CDirectObject.h"
#include "OggFile.h"
#include "SoundSystemDefines.h"

#include <memory>

class CSoundMixer;
class CSoundHub;

class CBasicSound : public CDirectObject {
public:
    Fixed masterVolume;
    int16_t volumes[2];
    int16_t volumeMax;

    int16_t hubId;
    CSoundHub *itsHub;
    std::shared_ptr<OggFile> itsSamples;
    CBasicSound *nextSound;

    CSoundMixer *itsMixer;
    int32_t sampleLen;
    SampleIndex currentCount[2];
    int32_t loopStart;
    int32_t loopEnd;
    int32_t loopCount[2];

    int32_t squareAcc[2]; //	Distance squared
    Fixed dSquare; //	Distance squared as a Fixed.
    Fixed distance; //	Distance (square root of squareAcc).
    Fixed relPos[3]; //	Distance as a vector
    Fixed balance;

    SoundLink *motionLink; //	Location information.
    SoundLink *controlLink; //	For control, when motionLink is not good enough.
    Boolean stopNow; //	Flag to make the sound stop.
    Boolean distanceDelay; //	Delay sound depending on position.

    //	The following four functions will usually be overridden:
    virtual int16_t CalcVolume(int16_t theChannel); //	Return volume
    virtual void WriteFrame(int16_t theChannel, int16_t volumeAllowed);
    virtual Boolean EndFrame(); //	Return true after last frame.
    virtual void SanityCheck(); //	Check parameters for sanity.
    virtual void FirstFrame(); //	Perform interrupt time inits.

    void CalculatePosition(int t);
    void CalculateMotionVolume();

    //	Start playing the sound
    virtual void Start();

    virtual void Reset();
    virtual void UseSamples(std::shared_ptr<OggFile> theSample);

    virtual void Release();
    virtual void SetVolume(Fixed vol);
    virtual void SetDirectVolumes(Fixed vol0, Fixed vol1);
    virtual void StopSound();

    virtual void SetLoopCount(short count);

    virtual void SetRate(Fixed theRate);
    virtual UnsignedFixed GetSampleRate(); //	Returns samples/second as a fixed!
    virtual void SetSoundLength(Fixed seconds);

    void SetSoundLink(SoundLink *linkPtr);
    void SetControlLink(SoundLink *linkPtr);
    void CheckSoundLink();
};
