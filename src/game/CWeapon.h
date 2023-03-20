/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CWeapon.h
    Created: Saturday, December 16, 1995, 21:13
    Modified: Monday, September 16, 1996, 16:27
*/

#pragma once

#include "CAbstractActor.h"

class CWeapon : public CAbstractActor {
public:
    class CWeapon *nextWeapon;
    class CDepot *itsDepot;

    Vector speed;
    Vector location;
    ActorAttachment clamp;

    long hostIdent;
    CSmartPart *hostPart;
    Matrix relation;

    Fixed pitch;
    Fixed yaw;
    Fixed roll;

    short ownerSlot;
    short weaponKind;

    FrameNumber flyCount;
    Boolean doExplode;
    Boolean isTargetLocked;

    virtual void IWeapon(CDepot *theDepot);
    virtual void ResetWeapon();

    virtual void LevelReset();
    virtual void Deactivate();

    virtual void Locate();
    virtual void ReleaseAttachment();
    virtual void Disarm();

    virtual void WasDestroyed();

    virtual long Arm(CSmartPart *aPart);

    virtual void PostMortemBlast(short scoreTeam, short scoreId, Boolean doDispose);
    virtual void Explode();
    virtual void Fire();

    virtual void DoTargeting(); //	Do targeting, if necessary.
    virtual void ShowTarget(); //	Show HUD graphics for targeting

    virtual void PreLoadSounds();

    virtual void Accelerate(Fixed *direction);
};
