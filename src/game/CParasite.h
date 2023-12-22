/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CParasite.h
    Created: Monday, November 20, 1995, 14:37
    Modified: Friday, March 1, 1996, 17:24
*/

#pragma once
#include "CRealMovers.h"

class CAbstractPlayer;

class CParasite : public CRealMovers {
public:
    virtual void PlaceParts();
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void ReleaseAttachment();
    virtual void Dispose();
    virtual void CourseCheck();
    virtual void FrameAction();
    virtual void ClampOn(CSmartPart *clampTo);
private:
    Fixed maxPower;
    Fixed energyDrain;
    Fixed range;
    short pitch;
    long clock;

    Vector course;
    Fixed acceleration;
    Fixed goodHeading;
    Fixed goodPitch;
    short moveClock;

    ActorAttachment clamp;
    long hostIdent;
    CSmartPart *hostPart;
    Matrix relation;

    short clampSound;
    Fixed clampVolume;

    Boolean isTracking;

};
