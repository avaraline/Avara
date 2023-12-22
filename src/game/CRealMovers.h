/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CRealMovers.h
    Created: Friday, March 1, 1996, 17:18
    Modified: Friday, August 9, 1996, 21:07
*/

#pragma once
#include "CGlowActors.h"

class CRealMovers : public CGlowActors {
public:
    Fixed baseMass;
    Vector speed = {0, 0, 0, 0}; // Track speed when Hector gets hit
    Vector dSpeed = {0, 0, 0, 0}; // Track delta velocity when Hector gets hit

    CRealMovers();
    virtual void GetSpeedEstimate(Fixed *theSpeed);
    virtual void Push(Fixed *direction);
    virtual void Accelerate(Fixed *direction);
    virtual void WasHit(RayHitRecord *theHit, Fixed hitEnergy);
    virtual void FindBestMovement(CSmartPart *objHit);

    virtual void StandingOn(CAbstractActor *who, Fixed *where, Boolean firstLeg);
};
