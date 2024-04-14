/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CAbstractMissile.h
    Created: Wednesday, March 8, 1995, 06:13
    Modified: Sunday, February 4, 1996, 12:31
*/

#pragma once
#include "CAbstractActor.h"
#include "CSmartPart.h"

enum { kcFlatMissile, kcRotaryMissile };

class CAvaraGame;
class CAbstractMissile;
class CDepot;

class CAbstractMissile : public CAbstractActor {
public:
    CAbstractMissile *nextMissile = 0;
    long ownerIdent = 0;
    CDepot *itsDepot = 0;
    short frameCount = 0;
    short maxFrameCount = 0;

    Fixed speed = 0;
    Fixed energy = 0;
    RayHitRecord hitRec = {0};
    Vector deltaMove = {0};
    Matrix itsMatrix = {0};

    short missileKind = 0;
    short soundResId = 0;

    CAbstractMissile(CDepot *theDepot);
    virtual void Deactivate();
    virtual void Launch(Matrix *startMatrix,
        RayHitRecord *target,
        CAbstractActor *theOwner,
        Fixed theEnergy,
        Fixed *speedVector);
    virtual void FireSlivers();
    virtual void MoveForward();
    virtual void FrameAction();
    virtual void LevelReset();
    virtual void TransparentStep();
    virtual void PreLoadSounds();
};
