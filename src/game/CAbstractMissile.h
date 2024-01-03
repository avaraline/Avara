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
    CAbstractMissile *nextMissile;
    long ownerIdent;
    CDepot *itsDepot;

    short frameCount;
    short maxFrameCount;

    Fixed speed;
    Fixed energy;
    RayHitRecord hitRec;
    Vector deltaMove;
    Matrix itsMatrix;

    short missileKind;
    short soundResId;

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
