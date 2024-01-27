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
    class CRateSound *theSampler = 0;
    SoundLink *controlLink = 0;
    MessageRecord startMsg = {0};
    MessageRecord stopMsg = {0};
    MessageRecord killMsg = {0};

    short soundId = 0;

    Fixed volumes[2] = {0};
    Fixed rate = 0;
    long loopCount = 0;
    long phase = 0;

    SampleIndex savedPosition[2] = {{0}};
    int32_t savedLoopCount[2] = {0};
    Fixed savedBalance = 0;
    Boolean needsResume = false;

    Boolean isMusic = false;
    Boolean isAmbient = false;
    Boolean isPlaced = false;
    Boolean restart = false;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void GetSound();
    virtual void ResumeLevel();
    virtual void PauseLevel();
    virtual void FrameAction();
    virtual ~CSoundActor();

    virtual Boolean ShouldPlay();
};
