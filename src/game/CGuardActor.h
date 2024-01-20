/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CGuardActor.h
    Created: Saturday, December 17, 1994, 02:51
    Modified: Thursday, May 30, 1996, 14:25
*/

#pragma once
#include "CGlowActors.h"

#define kGuardBSP 800

class CGuardActor : public CGlowActors {
public:
    short fireFrequency;
    short fireCount;

    MessageRecord fireActivator;
    MessageRecord trackingActivator;
    MessageRecord trackingDeactivator;

    Boolean isTracking;
    Fixed pitch;
    Fixed rotSpeed;
    Fixed shotPower;
    Fixed visionRange;
    MaskType watchBits;
    short watchTeams;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual ~CGuardActor();

    virtual void PlaceParts();
    virtual CSmartPart *FindClosestTarget();
    virtual void FrameAction();
};
