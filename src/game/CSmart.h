/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CSmart.h
    Created: Wednesday, February 14, 1996, 01:39
    Modified: Monday, September 16, 1996, 16:31
*/

#pragma once

#include "CWeapon.h"

class CDepot;

class CSmart : public CWeapon {
public:
    Fixed goodYaw;
    Fixed goodPitch;

    CSmartPart *targetPart;
    long targetIdent;
    Fixed sightDistance;
    Fixed thrust;
    Fixed angleStep;
    short inSight;
    long fireFrame;

    virtual void IWeapon(CDepot *theDepot);
    virtual void ResetWeapon();

    virtual void PlaceParts();
    virtual long Arm(CSmartPart *aPart);

    virtual void DoTargeting();
    virtual void ShowTarget();

    virtual void Locate();
    virtual void Fire();

    virtual void TurnTowardsTarget();
    virtual void FrameAction();
    virtual void PreLoadSounds();
    virtual bool HandlesFastFPS() { return true; }
};
