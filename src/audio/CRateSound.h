/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CRateSound.h
    Created: Monday, December 26, 1994, 09:03
    Modified: Saturday, February 24, 1996, 09:24
*/

#pragma once
#include "CBasicSound.h"
#include "OggFile.h"

class CRateSound : public CBasicSound {
public:
    Fixed masterRate;
    Fixed midRate;
    Fixed chanRate[2];
    Fixed phaseBalance;
    Boolean chanDoneFlags[2];

    virtual void Reset();
    virtual void UseSamples(std::shared_ptr<OggFile> theSample);
    virtual void FirstFrame();
    virtual void WriteFrame(int16_t theChannel, int16_t volumeAllowed);
    virtual int16_t CalcVolume(int16_t theChannel); //	Return volume
    virtual Boolean EndFrame();
    virtual void SetRate(Fixed aRate);
    virtual UnsignedFixed GetSampleRate(); //	Returns samples/second as a fixed!
};
