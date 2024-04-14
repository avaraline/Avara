/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CGrenade.h
    Created: Tuesday, February 13, 1996, 00:03
    Modified: Monday, June 3, 1996, 21:44
*/

#pragma once

#include "CWeapon.h"

class CDepot;

class CGrenade final : public CWeapon {
public:
    Fixed gravity = 0, classicGravity = 0;
    Fixed friction = 0, classicFriction = 0;
    Fixed speedOffset = 0;

    CGrenade(CDepot *theDepot);
    virtual void PlaceParts();
    virtual void AdaptableSettings();
    virtual long Arm(CSmartPart *aPart);
    virtual void Locate();
    virtual void Fire();
    virtual void FrameAction();
    virtual void ShowTarget(); //	Show HUD graphics for targeting
    virtual void PreLoadSounds();
    virtual bool HandlesFastFPS() { return true; }
};
