/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CRealShooters.h
    Created: Saturday, August 10, 1996, 21:00
    Modified: Tuesday, August 27, 1996, 03:53
*/

#pragma once
#include "CRealMovers.h"

class CRealShooters : public CRealMovers {
public:
    //	Firing control.
    Fixed visionRange;
    Fixed shotPower;

    FrameNumber burstStartFrame;
    FrameNumber nextShotFrame;
    short burstCount;

    long burstLength;
    FrameNumber burstSpeed;
    FrameNumber burstCharge;

    //	Who to fire at.
    MaskType watchMask;
    short watchTeams;

    virtual void IAbstractActor();
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();

    virtual Boolean Shoot();
    virtual short GetShooterId();
};
