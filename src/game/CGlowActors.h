/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CGlowActors.h
    Created: Wednesday, March 15, 1995, 07:06
    Modified: Friday, December 8, 1995, 04:46
*/

#pragma once
#include "CPlacedActors.h"

class CGlowActors : public CPlacedActors {
public:
    Fixed glow;
    Boolean canGlow;

    CGlowActors();
    virtual void BeginScript() override;
    virtual CAbstractActor *EndScript() override;
    virtual void FrameAction() override;
    virtual void WasHit(RayHitRecord *theHit, Fixed hitEnergy) override;
    virtual bool IsGeometryStatic() override;
};
