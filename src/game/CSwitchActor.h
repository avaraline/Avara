/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CSwitchActor.h
    Created: Tuesday, January 16, 1996, 00:03
    Modified: Friday, April 12, 1996, 16:42
*/

#pragma once
#include "CGlowActors.h"

class CSwitchActor : public CGlowActors {
public:
    MessageRecord onActivator = {0};
    MessageRecord offActivator = {0};

    MsgType onMessage = 0;
    MsgType offMessage = 0;

    Fixed togglePower = 0;
    Fixed blastToggle = 0;
    long toggleCount = 0;

    Boolean theState = 0;
    Boolean shownState = 0;
    Boolean msgToggle = 0;
    Boolean restart = 0;
    Boolean enabled = 0;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void PlaceParts();
    virtual CSmartPart *CollisionTest();
    virtual void FrameAction();
    virtual void WasHit(RayHitRecord *theHit, Fixed hitEnergy);
    virtual void BlastHit(BlastHitRecord *theHit);
    virtual ~CSwitchActor();
};
