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

class CSmart final : public CWeapon {
public:
    Fixed goodYaw = 0;
    Fixed goodPitch = 0;

    CSmartPart *targetPart = 0;
    long targetIdent = 0;
    Fixed sightDistance = 0;
    Fixed thrust = 0;
    Fixed angleStep = 0;
    short inSight = 0;
    FrameNumber fireFrame = 0;

    CSmart(CDepot *theDepot);
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
    virtual bool IsClassicInterval();
};
