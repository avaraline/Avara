/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CDepot.h
    Created: Wednesday, March 8, 1995, 06:00
    Modified: Wednesday, August 14, 1996, 16:23
*/

#pragma once
#include "AvaraDefines.h"
#include "CDirectObject.h"
#include "FastMat.h"
#include "RayHit.h"

#define WEAPONKINDS 2
#define MISSILEKINDS 3

#define SLIVERSIZES kSliverSizes
#define SLIVERCOUNT 64
#define MISSILECOUNT 24
#define WEAPONCOUNT 16

class CAbstractActor;
class CBSPPart;
class CSliverPart;
class CAbstractMissile;
class CAvaraGame;
class CWeapon;

enum { kmiFlat, kmiTurning, kmiShuriken };
enum { kweGrenade, kweSmart };

class CDepot : public CDirectObject {
public:
    CAvaraGame *itsGame;

    CAbstractMissile *missileList[MISSILEKINDS];
    CWeapon *weaponList[WEAPONKINDS];

    CSliverPart *freeSlivers[SLIVERSIZES];
    CSliverPart *activeSlivers[SLIVERSIZES];

    CBSPPart *smartSight;
    CBSPPart *smartHairs;
    CBSPPart *grenadeSight;
    CBSPPart *grenadeTop;
    Boolean bspInGame;

    Fixed grenadePower;
    Fixed missilePower;
    Fixed missileTurnRate;
    Fixed missileAcceleration;

    virtual void IDepot(CAvaraGame *theGame);
    virtual void Dispose();
    virtual void EndScript();

    virtual void CreateSlivers();
    virtual void RunSliverActions();

    virtual void FireSlivers(short n,
        Fixed *origin,
        Fixed *direction,
        Fixed scale,
        Fixed speedFactor,
        short spread,
        short age,
        short sizeGroup,
        CBSPPart *fromObject);

    virtual CAbstractMissile *MakeMissile(short kind);
    virtual void CreateMissiles();
    virtual void RetireMissile(CAbstractMissile *theMissile);
    virtual void DisposeMissiles();

    virtual CWeapon *MakeWeapon(short kind);
    virtual void CreateWeapons();
    virtual void RetireWeapon(CWeapon *theWeapon);
    virtual void DisposeWeapons();
    virtual CWeapon *AquireWeapon(short weaponKind);

    virtual CAbstractMissile *LaunchMissile(short kind,
        Matrix *startMatrix,
        RayHitRecord *target,
        CAbstractActor *theOwner,
        Fixed energy,
        Fixed *speedVector);

    virtual void LevelReset();

    virtual void FrameAction();
};
