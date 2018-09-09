/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CSoundActor.h
    Created: Thursday, November 30, 1995, 23:26
    Modified: Sunday, September 1, 1996, 18:54
*/

#pragma once
#include "CPlacedActors.h"

class CSoundActor : public CPlacedActors {
public:
    class CRateSound *theSampler;
    SoundLink *controlLink;
    MessageRecord startMsg;
    MessageRecord stopMsg;
    MessageRecord killMsg;

    short soundId;

    Fixed volumes[2];
    Fixed rate;
    long loopCount;
    long phase;

    SampleIndex savedPosition[2];
    long savedLoopCount[2];
    Fixed savedBalance;
    Boolean needsResume;

    Boolean isMusic;
    Boolean isAmbient;
    Boolean isPlaced;
    Boolean restart;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void GetSound();
    virtual void ResumeLevel();
    virtual void PauseLevel();
    virtual void FrameAction();
    virtual void Dispose();

    virtual Boolean ShouldPlay();
};
