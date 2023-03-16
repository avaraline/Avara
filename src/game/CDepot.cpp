/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CDepot.c
    Created: Wednesday, March 8, 1995, 06:01
    Modified: Wednesday, August 14, 1996, 16:34
*/

#include "CDepot.h"

#include "CAbstractMissile.h"
#include "CAvaraGame.h"
#include "CBSPWorld.h"
#include "ColorManager.h"
#include "CGrenade.h"
#include "CMissile.h"
#include "CPlayerMissile.h"
#include "CShuriken.h"
#include "CSliverPart.h"
#include "CSmart.h"
#include "CSmartPart.h"
#include "CViewParameters.h"

void CDepot::IDepot(CAvaraGame *theGame) {
    itsGame = theGame;

    smartSight = new CBSPPart;
    smartSight->IBSPPart(208);
    smartSight->ReplaceColor(0xfffffb00, ColorManager::getMissileSightPrimaryColor());
    smartSight->ReplaceColor(0xffff2600, ColorManager::getMissileSightSecondaryColor());
    smartSight->ignoreDirectionalLights = true;
    smartSight->privateAmbient = FIX1;

    smartHairs = new CBSPPart;
    smartHairs->IBSPPart(207);
    smartHairs->ReplaceColor(0xffff2600, ColorManager::getMissileLockColor());
    smartHairs->ignoreDirectionalLights = true;
    smartHairs->privateAmbient = FIX1;

    grenadeSight = new CBSPPart;
    grenadeSight->IBSPPart(200);
    grenadeSight->ReplaceColor(0xfffffb00, ColorManager::getGrenadeSightPrimaryColor());
    grenadeSight->ignoreDirectionalLights = true;
    grenadeSight->privateAmbient = FIX1;

    grenadeTop = new CBSPPart;
    grenadeTop->IBSPPart(201);
    grenadeTop->ReplaceColor(0xffff2600, ColorManager::getGrenadeSightSecondaryColor());
    grenadeTop->ignoreDirectionalLights = true;
    grenadeTop->privateAmbient = FIX1;

    bspInGame = false;

    CreateMissiles();
    CreateWeapons();
    CreateSlivers();
}

void CDepot::Dispose() {
    short i;
    CSliverPart *nextSliver, *slivers;

    DisposeMissiles();
    DisposeWeapons();

    for (i = 0; i < SLIVERSIZES; i++) {
        slivers = freeSlivers[i];
        while (slivers) {
            nextSliver = slivers->nextSliver;
            slivers->Dispose();
            slivers = nextSliver;
        }
    }

    smartHairs->Dispose();
    smartSight->Dispose();
    grenadeSight->Dispose();
    grenadeTop->Dispose();

    CDirectObject::Dispose();
}

void CDepot::EndScript() {
    grenadePower = ReadFixedVar(iGrenadePower);
    missilePower = ReadFixedVar(iMissilePower);
    missileTurnRate = ReadFixedVar(iMissileTurnRate);
    missileAcceleration = ReadFixedVar(iMissileAcceleration);
}

void CDepot::CreateSlivers() {
    short i, j;

    for (j = 0; j < SLIVERSIZES; j++) {
        freeSlivers[j] = NULL;
        activeSlivers[j] = NULL;

        for (i = 0; i < SLIVERCOUNT; i++) {
            CSliverPart *theSliver;

            theSliver = new CSliverPart;
            theSliver->ISliverPart(500 + j);
            theSliver->nextSliver = freeSlivers[j];
            freeSlivers[j] = theSliver;
        }
    }
}

void CDepot::RunSliverActions() {
    short i;
    CSliverPart *aSliver, **prevNext;
    CBSPWorld *theWorld = itsGame->itsWorld;

    for (i = 0; i < SLIVERSIZES; i++) {
        prevNext = &(activeSlivers[i]);
        aSliver = activeSlivers[i];

        while (aSliver) {
            if (aSliver->SliverAction()) {
                theWorld->RemovePart(aSliver);

                *prevNext = aSliver->nextSliver;
                aSliver->nextSliver = freeSlivers[i];
                freeSlivers[i] = aSliver;
                aSliver = *prevNext;
            } else {
                prevNext = &aSliver->nextSliver;
                aSliver = aSliver->nextSliver;
            }
        }
    }
}

void CDepot::FireSlivers(short n,
    Fixed *origin,
    Fixed *direction,
    Fixed scale,
    Fixed speedFactor,
    short spread,
    short age,
    short sizeGroup,
    CBSPPart *fromObject) {
    CBSPWorld *theWorld = itsGame->itsWorld;
    Vector loc;

    if (itsGame->simpleExplosions) {
        n = (n + 2) >> 1;
    }

    if (n > 2) {
        CViewParameters *vp;

        vp = itsGame->itsView;
        VectorMatrixProduct(1, (Vector *)origin, (Vector *)loc, &vp->viewMatrix);

        if (loc[2] < FIX(-10) || loc[2] > vp->yonBound) {
            n = (n + 3) >> 2;
            if (sizeGroup < kLargeSliver) {
                sizeGroup++;
            }
        } else {
            loc[2] += FIX(5);
            if (loc[2] > vp->yonBound || loc[2] < 0 || loc[0] > loc[2] || loc[1] > loc[2] || loc[0] < -loc[2] ||
                loc[1] < -loc[2]) {
                n = (n + 1) >> 1;
            }
        }
    }

    while (n-- && freeSlivers[sizeGroup]) {
        CSliverPart *theSliver;

        theSliver = freeSlivers[sizeGroup];
        freeSlivers[sizeGroup] = theSliver->nextSliver;
        theSliver->nextSliver = activeSlivers[sizeGroup];
        activeSlivers[sizeGroup] = theSliver;

        theSliver->Activate(origin, direction, scale, speedFactor, spread, age, fromObject);
        theWorld->AddPart(theSliver);
    }
}

CAbstractMissile *CDepot::MakeMissile(short kind) {
    CAbstractMissile *newMissile = NULL;

    switch (kind) {
        case kmiFlat:
            newMissile = new CMissile;
            break;
        case kmiTurning:
            newMissile = new CPlayerMissile;
            break;
        case kmiShuriken:
            newMissile = new CShuriken;
            break;
        default:
            return newMissile;
    }

    newMissile->IAbstractMissile(this);
    newMissile->missileKind = kind;

    return newMissile;
}

void CDepot::CreateMissiles() {
    CAbstractMissile *newMissile;
    short i, j;

    for (j = 0; j < MISSILEKINDS; j++) {
        missileList[j] = NULL;

        for (i = 0; i < MISSILECOUNT; i++) {
            newMissile = MakeMissile(j);
            newMissile->nextMissile = missileList[j];
            missileList[j] = newMissile;
        }
    }
}

void CDepot::DisposeMissiles() {
    CAbstractMissile *nextMissile;
    CAbstractMissile *mList;
    short j;

    for (j = 0; j < MISSILEKINDS; j++) {
        mList = missileList[j];
        while (mList) {
            nextMissile = mList->nextMissile;
            mList->Dispose();
            mList = nextMissile;
        }
    }
}

void CDepot::RetireMissile(CAbstractMissile *theMissile) {
    itsGame->RemoveActor(theMissile);
    theMissile->nextMissile = missileList[theMissile->missileKind];
    missileList[theMissile->missileKind] = theMissile;
}

CAbstractMissile *CDepot::LaunchMissile(short kind,
    Matrix *startMatrix,
    RayHitRecord *target,
    CAbstractActor *theOwner,
    Fixed energy,
    Fixed *speedVector) {
    CAbstractMissile *freshMissile;

    freshMissile = missileList[kind];
    if (!freshMissile) {
        freshMissile = MakeMissile(kind);
        freshMissile->nextMissile = NULL;
    }

    if (freshMissile) {
        missileList[kind] = freshMissile->nextMissile;
        freshMissile->Launch(startMatrix, target, theOwner, energy, speedVector);

        return freshMissile;
    } else
        return NULL;
}

void CDepot::LevelReset() {
    short i;
    CSliverPart *nextSliver, *aSliver;
    CBSPWorld *theWorld = itsGame->itsWorld;

    for (i = 0; i < SLIVERSIZES; i++) {
        aSliver = activeSlivers[i];
        while (aSliver) {
            theWorld->RemovePart(aSliver);
            nextSliver = aSliver->nextSliver;
            aSliver->nextSliver = freeSlivers[i];
            freeSlivers[i] = aSliver;

            aSliver = nextSliver;
        }

        activeSlivers[i] = 0;
    }

    if (bspInGame) {
        bspInGame = false;

        theWorld->RemovePart(grenadeTop);

        theWorld = itsGame->hudWorld;
        theWorld->RemovePart(smartHairs);
        theWorld->RemovePart(smartSight);
        theWorld->RemovePart(grenadeSight);
    }

    for (i = 0; i < MISSILEKINDS; i++) {
        missileList[i]->PreLoadSounds();
    }

    for (i = 0; i < WEAPONKINDS; i++) {
        weaponList[i]->PreLoadSounds();
    }
}

CWeapon *CDepot::MakeWeapon(short kind) {
    CWeapon *newWeapon = NULL;

    switch (kind) {
        case kweGrenade:
            newWeapon = new CGrenade;
            break;
        case kweSmart:
            newWeapon = new CSmart;
            break;
        default:
            return newWeapon;
    }

    newWeapon->IWeapon(this);
    newWeapon->weaponKind = kind;

    return newWeapon;
}

void CDepot::CreateWeapons() {
    CWeapon *newWeapon;
    short i, j;

    for (j = 0; j < WEAPONKINDS; j++) {
        weaponList[j] = NULL;

        for (i = 0; i < WEAPONCOUNT; i++) {
            newWeapon = MakeWeapon(j);
            newWeapon->nextWeapon = weaponList[j];
            weaponList[j] = newWeapon;
        }
    }
}

void CDepot::DisposeWeapons() {
    CWeapon *nextWeapon;
    CWeapon *wList;
    short j;

    for (j = 0; j < WEAPONKINDS; j++) {
        wList = weaponList[j];
        while (wList) {
            nextWeapon = wList->nextWeapon;
            wList->Dispose();
            wList = nextWeapon;
        }
    }
}

void CDepot::RetireWeapon(CWeapon *theWeapon) {
    itsGame->RemoveActor(theWeapon);
    theWeapon->nextWeapon = weaponList[theWeapon->weaponKind];
    weaponList[theWeapon->weaponKind] = theWeapon;
}

CWeapon *CDepot::AquireWeapon(short weaponKind) {
    CWeapon *theWeapon;

    theWeapon = weaponList[weaponKind];
    if (theWeapon) {
        weaponList[weaponKind] = theWeapon->nextWeapon;
    } else {
        theWeapon = MakeWeapon(weaponKind);
    }

    return theWeapon;
}

void CDepot::FrameAction() {
    if (!bspInGame) {
        CBSPWorld *theWorld;

        theWorld = itsGame->hudWorld;
        theWorld->AddPart(smartHairs);
        theWorld->AddPart(smartSight);
        theWorld->AddPart(grenadeSight);

        theWorld = itsGame->itsWorld;
        theWorld->AddPart(grenadeTop);
        bspInGame = true;
    }
    smartHairs->isTransparent = true;
    smartSight->isTransparent = true;
    grenadeSight->isTransparent = true;
    grenadeTop->isTransparent = true;
}
