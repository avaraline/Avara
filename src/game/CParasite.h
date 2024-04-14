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

class CParasite final : public CRealMovers {
public:
    Fixed maxPower = 0;
    Fixed energyDrain = 0;
    Fixed range = 0;
    short pitch = 0;
    long clock = 0;

    Vector course = {0};
    Fixed acceleration = 0;
    Fixed goodHeading = 0;
    Fixed goodPitch = 0;
    short moveClock = 0;

    ActorAttachment clamp = {0};
    long hostIdent = 0;
    CSmartPart *hostPart = 0;
    Matrix relation = {0};

    short clampSound = 0;
    Fixed clampVolume = 0;

    Boolean isTracking = 0;

    virtual void PlaceParts();
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void ReleaseAttachment();
    virtual ~CParasite();
    virtual void CourseCheck();
    virtual void FrameAction();
    virtual void ClampOn(CSmartPart *clampTo);
};
