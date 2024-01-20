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
    MessageRecord onActivator;
    MessageRecord offActivator;

    MsgType onMessage;
    MsgType offMessage;

    Fixed togglePower;
    Fixed blastToggle;
    long toggleCount;

    Boolean theState;
    Boolean shownState;
    Boolean msgToggle;
    Boolean restart;
    Boolean enabled;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void PlaceParts();
    virtual CSmartPart *CollisionTest();
    virtual void FrameAction();
    virtual void WasHit(RayHitRecord *theHit, Fixed hitEnergy);
    virtual void BlastHit(BlastHitRecord *theHit);
    virtual ~CSwitchActor();
};
