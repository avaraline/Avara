/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: CSphereActor.h
    Created: Wednesday, November 23, 1994, 08:48
    Modified: Friday, September 8, 1995, 08:27
*/

#pragma once
#include "CGlowActors.h"
#include "FastMat.h"

#define kSphereBSP 300

class CSphereActor : public CGlowActors {
public:
    virtual void IAbstractActor();
    virtual CAbstractActor *EndScript();
    virtual void WasHit(RayHitRecord *theHit, Fixed hitEnergy);
};
