/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CDepot.c
    Created: Wednesday, March 8, 1995, 06:01
    Modified: Wednesday, August 14, 1996, 16:34
*/

#include "CDepot.h"

#include "AbstractRenderer.h"
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

CDepot::CDepot(CAvaraGame *theGame) {
    itsGame = theGame;

    ReloadParts();

    bspInGame = false;

    CreateMissiles();
    CreateWeapons();
    CreateSlivers();
}

CDepot::~CDepot() {
    short i;
    CSliverPart *nextSliver, *slivers;

    DisposeMissiles();
    DisposeWeapons();

    for (i = 0; i < SLIVERSIZES; i++) {
        slivers = freeSlivers[i];
        while (slivers) {
            nextSliver = slivers->nextSliver;
            delete slivers;
            slivers = nextSliver;
        }
    }

    delete smartHairs;
    delete smartSight;
    delete grenadeSight;
    delete grenadeTop;
}

void CDepot::EndScript() {
    grenadePower = ReadFixedVar(iGrenadePower);
    missilePower = ReadFixedVar(iMissilePower);
    missileTurnRate = ReadFixedVar(iMissileTurnRate);
    missileAcceleration = ReadFixedVar(iMissileAcceleration);

    ReloadParts();
    DisposeMissiles();
    DisposeWeapons();
    CreateMissiles();
    CreateWeapons();
}

void CDepot::CreateSlivers() {
    short i, j;

    for (j = 0; j < SLIVERSIZES; j++) {
        freeSlivers[j] = NULL;
        activeSlivers[j] = NULL;

        for (i = 0; i < SLIVERCOUNT; i++) {
            CSliverPart *theSliver;

            theSliver = new CSliverPart(500 + j);
            theSliver->nextSliver = freeSlivers[j];
            freeSlivers[j] = theSliver;
        }
    }
}

void CDepot::RunSliverActions() {
    short i;
    CSliverPart *aSliver, **prevNext;

    for (i = 0; i < SLIVERSIZES; i++) {
        prevNext = &(activeSlivers[i]);
        aSliver = activeSlivers[i];

        while (aSliver) {
            if (aSliver->SliverAction()) {
                gRenderer->RemovePart(aSliver);

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
    Vector loc;

    if (itsGame->simpleExplosions) {
        n = (n + 2) >> 1;
    }

    if (n > 2) {
        auto vp = gRenderer->viewParams;
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
        gRenderer->AddPart(theSliver);
    }
}

CAbstractMissile *CDepot::MakeMissile(short kind) {
    CAbstractMissile *newMissile = NULL;

    switch (kind) {
        case kmiFlat:
            newMissile = new CMissile(this);
            break;
        case kmiTurning:
            newMissile = new CPlayerMissile(this);
            break;
        case kmiShuriken:
            newMissile = new CShuriken(this);
            break;
        default:
            return newMissile;
    }

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
            delete mList;
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

void CDepot::ReloadParts() {
    if (smartHairs) delete smartHairs;
    if (smartSight) delete smartSight;
    if (grenadeSight) delete grenadeSight;
    if (grenadeTop) delete grenadeTop;

    smartSight = CBSPPart::Create(208);
    smartSight->ReplaceColor(0xfffffb00, ColorManager::getMissileSightPrimaryColor());
    smartSight->ReplaceColor(0xffff2600, ColorManager::getMissileSightSecondaryColor());
    smartSight->ignoreDepthTesting = true;
    smartSight->ignoreDirectionalLights = true;
    smartSight->privateAmbient = FIX1;

    smartHairs = CBSPPart::Create(207);
    smartHairs->ReplaceColor(0xffff2600, ColorManager::getMissileLockColor());
    smartHairs->ignoreDepthTesting = true;
    smartHairs->ignoreDirectionalLights = true;
    smartHairs->privateAmbient = FIX1;

    grenadeSight = CBSPPart::Create(200);
    grenadeSight->ReplaceColor(0xfffffb00, ColorManager::getGrenadeSightPrimaryColor());
    grenadeSight->ignoreDepthTesting = true;
    grenadeSight->ignoreDirectionalLights = true;
    grenadeSight->privateAmbient = FIX1;

    grenadeTop = CBSPPart::Create(201);
    grenadeTop->ReplaceColor(0xffff2600, ColorManager::getGrenadeSightSecondaryColor());
    grenadeTop->ignoreDirectionalLights = true;
    grenadeTop->privateAmbient = FIX1;
}

void CDepot::LevelReset() {
    short i;
    CSliverPart *nextSliver, *aSliver;

    for (i = 0; i < SLIVERSIZES; i++) {
        aSliver = activeSlivers[i];
        while (aSliver) {
            gRenderer->RemovePart(aSliver);
            nextSliver = aSliver->nextSliver;
            aSliver->nextSliver = freeSlivers[i];
            freeSlivers[i] = aSliver;

            aSliver = nextSliver;
        }

        activeSlivers[i] = 0;
    }

    if (bspInGame) {
        bspInGame = false;

        gRenderer->RemovePart(grenadeTop);
        gRenderer->RemoveHUDPart(smartHairs);
        gRenderer->RemoveHUDPart(smartSight);
        gRenderer->RemoveHUDPart(grenadeSight);
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
            newWeapon = new CGrenade(this);
            break;
        case kweSmart:
            newWeapon = new CSmart(this);
            break;
        default:
            return newWeapon;
    }

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
            delete wList;
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
        gRenderer->AddHUDPart(smartHairs);
        gRenderer->AddHUDPart(smartSight);
        gRenderer->AddHUDPart(grenadeSight);
        gRenderer->AddPart(grenadeTop);
        bspInGame = true;
    }
    smartHairs->isTransparent = true;
    smartSight->isTransparent = true;
    grenadeSight->isTransparent = true;
    grenadeTop->isTransparent = true;
}
