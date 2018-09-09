/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CBasicSound.h
    Created: Friday, December 23, 1994, 09:53
    Modified: Saturday, February 24, 1996, 09:24
*/

#pragma once
#include "CDirectObject.h"
#include "SoundSystemDefines.h"

class CSoundMixer;
class CSoundHub;

class CBasicSound : public CDirectObject {
public:
    Fixed masterVolume;
    unsigned short volumes[2];
    short volumeMax;

    short hubId;
    CSoundHub *itsHub;
    SampleHeaderHandle itsSamples;
    CBasicSound *nextSound;

    CSoundMixer *itsMixer;
    int sampleLen;
    SampleIndex currentCount[2];
    int loopStart;
    int loopEnd;
    int loopCount[2];
    Sample *sampleData;

    int squareAcc[2]; //	Distance squared
    Fixed dSquare; //	Distance squared as a Fixed.
    Fixed distance; //	Distance (square root of squareAcc).
    Fixed relPos[3]; //	Distance as a vector
    Fixed balance;

    SoundLink *motionLink; //	Location information.
    SoundLink *controlLink; //	For control, when motionLink is not good enough.
    Boolean stopNow; //	Flag to make the sound stop.
    Boolean distanceDelay; //	Delay sound depending on position.

    //	The following four functions will usually be overridden:
    virtual short CalcVolume(short theChannel); //	Return volume
    virtual void WriteFrame(short theChannel, short volumeAllowed);
    virtual Boolean EndFrame(); //	Return true after last frame.
    virtual void SanityCheck(); //	Check parameters for sanity.
    virtual void FirstFrame(); //	Perform interrupt time inits.

    void CalculatePosition(int t);
    void CalculateMotionVolume();

    //	Start playing the sound
    virtual void Start();

    virtual void Reset();
    virtual void UseSamplePtr(Sample *samples, int numSamples);
    virtual void UseSamples(SampleHeaderHandle theSample);

    virtual void Release();
    virtual void SetVolume(Fixed vol);
    virtual void SetDirectVolumes(Fixed vol0, Fixed vol1);
    virtual void StopSound();

    virtual void SetLoopCount(short count);

    virtual void SetRate(Fixed theRate);
    virtual Fixed GetSampleRate(); //	Returns samples/second as a fixed!
    virtual void SetSoundLength(Fixed seconds);

    void SetSoundLink(SoundLink *linkPtr);
    void SetControlLink(SoundLink *linkPtr);
    void CheckSoundLink();
};
