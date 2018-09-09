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

    long burstStartFrame;
    long nextShotFrame;
    short burstCount;

    long burstLength;
    long burstSpeed;
    long burstCharge;

    //	Who to fire at.
    MaskType watchMask;
    short watchTeams;

    virtual void IAbstractActor();
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();

    virtual Boolean Shoot();
    virtual short GetShooterId();
};
