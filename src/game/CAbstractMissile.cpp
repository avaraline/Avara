/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CAbstractMissile.c
    Created: Wednesday, March 8, 1995, 06:14
    Modified: Monday, August 26, 1996, 03:04
*/

#include "CAbstractMissile.h"

#include "CDepot.h"

#define GROUNDHITSOUNDID 211
#define DEFAULTMISSILESOUND 200

CAbstractMissile::CAbstractMissile(CDepot *theDepot) {
    isActive = kIsActive;
    itsDepot = theDepot;
    itsGame = itsDepot->itsGame;

    speed = FIX3(2000);

    energy = 0;

    nextMissile = NULL;

    soundResId = DEFAULTMISSILESOUND;
}

void CAbstractMissile::PreLoadSounds() {
    auto _ = AssetManager::GetOgg(GROUNDHITSOUNDID);
    _ = AssetManager::GetOgg(soundResId);
}

void CAbstractMissile::Deactivate() {
    if (itsSoundLink) {
        gHub->ReleaseLinkAndKillSounds(itsSoundLink);
        itsSoundLink = NULL;
    }
    itsDepot->RetireMissile(this);
}

void CAbstractMissile::LevelReset() {
    Deactivate();
}

void CAbstractMissile::FrameAction() {
    CAbstractActor *anActor;
    CAbstractActor *theOwner;
    Fixed negOrigin1;

    hitRec.origin[0] = itsMatrix[3][0];
    hitRec.origin[1] = itsMatrix[3][1];
    hitRec.origin[2] = itsMatrix[3][2];

    negOrigin1 = -hitRec.origin[1];
    if (deltaMove[1] < negOrigin1 && negOrigin1 < 0) {
        hitRec.distance = FDivNZ(negOrigin1, hitRec.direction[1]);
        if (hitRec.distance > speed)
            hitRec.distance = speed;
    } else {
        hitRec.distance = speed;
    }

    hitRec.closestHit = NULL;

    theOwner = itsGame->FindIdent(ownerIdent);
    if (theOwner)
        theOwner->searchCount = itsGame->searchCount + 1;

    RayTest(&hitRec, kSolidBit);

    if (hitRec.distance < speed || hitRec.closestHit) {
        FireSlivers();

        if (hitRec.closestHit) {
            if (missileKind == kmiTurning) {
                itsGame->scoreReason = ksiPlasmaHit;
            }
            anActor = hitRec.closestHit->theOwner;
            anActor->WasHit(&hitRec, energy);
            SecondaryDamage(hitRec.team, hitRec.playerId, ksiMissileHit);
        }

        Deactivate();
    } else {
        MoveForward();
        if (frameCount-- == 0) {
            Deactivate();
        }

        if (frameCount < FpsCoefficient2(2))
            energy -= FpsCoefficient2(energy >> 3);
    }
}

void CAbstractMissile::MoveForward() {
    partList[0]->isTransparent = false;
    partList[0]->extraAmbient = energy;
    partList[0]->Reset();
    partList[0]->ApplyMatrix(&itsMatrix);
    itsMatrix[3][0] += FpsCoefficient2(deltaMove[0]);
    itsMatrix[3][1] += FpsCoefficient2(deltaMove[1]);
    itsMatrix[3][2] += FpsCoefficient2(deltaMove[2]);
    partList[0]->MoveDone();
}
void CAbstractMissile::FireSlivers() {
    Vector sliverDir;
    short spread;
    short numSlivers;

    hitRec.origin[0] = hitRec.origin[0] + FMul(hitRec.direction[0], hitRec.distance);
    hitRec.origin[1] = hitRec.origin[1] + FMul(hitRec.direction[1], hitRec.distance);
    hitRec.origin[2] = hitRec.origin[2] + FMul(hitRec.direction[2], hitRec.distance);

    if (hitRec.closestHit) {
        numSlivers = 8;
        spread = 100;
        sliverDir[0] = -hitRec.direction[0];
        sliverDir[1] = -hitRec.direction[1];
        sliverDir[2] = -hitRec.direction[2];
    } else { //	Hits the ground.

        numSlivers = 3;
        spread = 15;
        sliverDir[0] = hitRec.direction[0];
        sliverDir[1] = -hitRec.direction[1];
        sliverDir[2] = hitRec.direction[2];

        DoSound(GROUNDHITSOUNDID, hitRec.origin, 8 * energy, FIX1);
    }

    itsDepot->FireSlivers(numSlivers, hitRec.origin, sliverDir, FIX3(200), FIX3(3000), spread, 15, 0, partList[0]);
}

void CAbstractMissile::Launch(Matrix *startMatrix,
    RayHitRecord *target,
    CAbstractActor *theOwner,
    Fixed theEnergy,
    Fixed *speedVector) {
    CBasicSound *theSound;

    ownerIdent = theOwner->ident;
    itsGame->AddActor(this);

    frameCount = maxFrameCount;

    *(MatrixStruct *)&itsMatrix = *(MatrixStruct *)startMatrix;
    hitRec = *target;

    if (speedVector) {
        deltaMove[0] = speedVector[0];
        deltaMove[1] = speedVector[1];
        deltaMove[2] = speedVector[2];
        speed = VectorLength(3, deltaMove);
    } else {
        deltaMove[0] = FMul(speed, target->direction[0]);
        deltaMove[1] = FMul(speed, target->direction[1]);
        deltaMove[2] = FMul(speed, target->direction[2]);
    }

    partList[0]->isTransparent = true;
    energy = theEnergy;

    itsSoundLink = gHub->GetSoundLink();
    UpdateSoundLink(itsSoundLink, itsMatrix[3], deltaMove, itsGame->soundTime);

    theSound = gHub->GetSoundSampler(hubRate, soundResId);
    //	theSound->SetRate(FIX1));
    theSound->SetVolume(FMul(speed, FIX3(500)));
    theSound->SetSoundLink(itsSoundLink);
    if (itsGame->soundSwitches & kMissileLoopToggle)
        theSound->SetLoopCount(-1);
    theSound->Start();
}

void CAbstractMissile::TransparentStep() {
    FrameAction();
    partList[0]->isTransparent = true;
}
