/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CAbstractMovers.h
    Created: Tuesday, December 5, 1995, 06:02
    Modified: Friday, January 19, 1996, 03:08
*/

#pragma once
#include "CGlowActors.h"

class CAbstractMovers : public CGlowActors {
public:
    Vector speed;
    Fixed impulse[3];
    Fixed internalImpulse[3];
    Fixed impulseSlowDown;

    virtual void IAbstractActor();
    virtual void GetSpeedEstimate(Fixed *theSpeed);
    virtual void FrameAction();
    virtual void Push(Fixed *direction);
    virtual void Accelerate(Fixed *direction);
    virtual void WasHit(RayHitRecord *theHit, Fixed hitEnergy);
};
