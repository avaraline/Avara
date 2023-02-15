/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CZombieActor.c
    Created: Tuesday, October 31, 1995, 14:58
    Modified: Monday, February 26, 1996, 16:01
*/

#include "CZombieActor.h"

#include "CSmartPart.h"

#define zombieFireHeight FIX3(1200)
#define zombieScanDistance FIX(64)

#define zombieFarDistance FIX(20)
#define zombieCloseDistance FIX(9)
#define zombieRealCloseDistance FIX(4)

#define zombieAccel FIX3(80)
#define zombieFriction FIX3(900)
#define zombieTurn FIX3(5)
#define zombieHappinessDecay FIX3(1)

#define zombieCheckPeriod 10

void CZombieActor::BeginScript() {
    maskBits |= kTargetBit;

    CGlowActors::BeginScript();
}

CAbstractActor *CZombieActor::EndScript() {
    short i;

    if (CGlowActors::EndScript()) {
        ARGBColor repColor(0);

        isActive = kIsActive;
        maskBits |= kTargetBit + kSolidBit;

        speed[0] = 0;
        speed[1] = 0;
        speed[2] = 0;

        accel[0] = 0;
        accel[1] = 0;
        accel[2] = 0;

        happiness = 0;
        checkCounter = 0;
        goodHeading = heading;
        watchHeading = heading;
        watchFlip = 1;

        partCount = 2;
        LoadPartWithColors(0, 810);
        LoadPartWithColors(1, 811);

        repColor = GetPixelColor();

        bigRadius = partList[0]->bigRadius;
        if (partList[1]->bigRadius > bigRadius)
            bigRadius = partList[1]->bigRadius;

        cornerAngle = FOneToDeg(FOneArcTan2(-partList[0]->minBounds.z, -partList[0]->minBounds.x));

        PlaceParts();

        nextSmart = 0;
        for (i = 0; i < SMARTMEMSIZE; i++) {
            smartMem[i][3] = 0; //	Neutral experience base.
        }
        return this;
    } else
        return NULL;
}

void CZombieActor::PlaceParts() {
    UnlinkLocation();
    partList[0]->Reset();
    InitialRotatePartY(partList[0], heading);
    TranslatePart(partList[0], location[0], location[1], location[2]);
    partList[0]->MoveDone();

    partList[1]->CopyTransform(&partList[0]->itsTransform);
    partList[1]->MoveDone();
    LinkPartSpheres();

    CGlowActors::PlaceParts();
}

Fixed CZombieActor::ScanDirection(Fixed direction, RayHitRecord *target) {
    target->origin[0] = location[0];
    target->origin[1] = location[1] + zombieFireHeight;
    target->origin[2] = location[2];
    target->direction[0] = FOneSin(direction);
    target->direction[1] = 0;
    target->direction[2] = FOneCos(direction);
    target->distance = zombieScanDistance;
    target->closestHit = NULL;
    RayTest(target, kSolidBit);

    return target->distance;
}

Fixed CZombieActor::JudgePlace(Fixed *place) {
    Fixed placeScore;
    Fixed distance;
    Fixed feeling;
    Vector *smarts;
    short i;

    placeScore = 0;
    smarts = smartMem;
    for (i = 0; i < SMARTMEMSIZE; i++) {
        distance = FDistanceEstimate(place[0] - (*smarts)[0], place[1] - (*smarts)[1], place[2] - (*smarts)[2]);

        if (distance > FIX(1)) {
            feeling = (*smarts)[3];
            placeScore += FDiv(feeling, distance);
            feeling -= feeling >> 3;
            (*smarts)[3] = feeling;
        } else {
            placeScore += (*smarts)[3];
        }
    }

    return placeScore;
}

void CZombieActor::RememberExperience(Fixed *place, Fixed weight) {
    Fixed *smarts;
    static long expoCount = 0;

    smarts = smartMem[nextSmart++];

    if (nextSmart >= SMARTMEMSIZE)
        nextSmart = 0;

    smarts[0] = place[0];
    smarts[1] = place[1];
    smarts[2] = place[2];
    smarts[3] = weight;

    expoCount++;
}

void CZombieActor::WasHit(RayHitRecord *theHit, Fixed hitEnergy) {
    RememberExperience(location, -hitEnergy);
    happiness -= hitEnergy;
    CGlowActors::WasHit(theHit, hitEnergy);
}

void CZombieActor::MoveForward() {
    short delta;
    Fixed friction;

    delta = goodHeading - heading;
    if (delta < -zombieTurn)
        heading -= zombieTurn;
    else if (delta > zombieTurn)
        heading += zombieTurn;
    else
        heading = goodHeading;
    heading = (short)heading;

    if (delta < 0)
        delta += 0x8000;
    if (delta > 0x4000)
        delta = 0x8000 - delta;

    friction = FMul(zombieFriction, FIX(1) - delta);

    location[0] += speed[0];
    location[1] += speed[1];
    location[2] += speed[2];

    speed[0] = FMul(speed[0] + accel[0], friction);
    speed[1] = FMul(speed[1] + accel[1], friction);
    speed[2] = FMul(speed[2] + accel[2], friction);

    PlaceParts();
}

void CZombieActor::SavePosition() {
    oldLoc[0] = location[0];
    oldLoc[1] = location[1];
    oldLoc[2] = location[2];
    oldHeading = heading;
}

void CZombieActor::UndoMove() {
    location[0] = oldLoc[0];
    location[1] = oldLoc[1];
    location[2] = oldLoc[2];
    heading = oldHeading;
}

void CZombieActor::FrameAction() {
    CGlowActors::FrameAction();

    SavePosition();
    MoveForward();

    BuildPartProximityList(location, bigRadius, kSolidBit);

    if (--checkCounter <= 0) {
        Fixed newHeading;
        RayHitRecord theHit;
        Vector newPlace;
        Fixed newScore = FIX(0);
        short delta;

        if (watchFlip > 0)
            watchFlip = -watchFlip - 100;
        else
            watchFlip = -watchFlip + 100;
        watchHeading += watchFlip;
        newHeading = (FRandom() - 0x8000) << 1;
        newHeading = watchHeading + FMul(newHeading, FMul(newHeading, newHeading));
        ScanDirection(newHeading, &theHit);
        if (!theHit.closestHit) {
            theHit.distance = FIX(2);
            newScore = 0;
        } else {
            if (theHit.distance > FIX(16))
                newScore = FIX(1);
            else
                newScore = theHit.distance >> 4;
        }

#define DISLIKE_TURNING
#ifdef DISLIKE_TURNING
        delta = (newHeading - heading) >> 1;
        if (delta > 0)
            newScore += delta;
        else
            newScore -= delta;
#endif

        newPlace[0] = location[0] + FMul(theHit.distance, theHit.direction[0]);
        newPlace[1] = location[1];
        newPlace[2] = location[0] + FMul(theHit.distance, theHit.direction[2]);
        newScore += JudgePlace(newPlace);
        if (newScore >= happiness) {
            goodHeading = (short)newHeading;
            watchHeading = (short)newHeading;
            watchHeading += watchFlip >> 1;
            accel[0] = FMul(theHit.direction[0], zombieAccel);
            accel[2] = FMul(theHit.direction[2], zombieAccel);
            happiness = newScore;
        }
    }

    if (DoCollisionTest(&proximityList.p)) {
        collisionPain += FIX(1);
        RememberExperience(location, -collisionPain);

        speed[0] = 0;
        speed[1] = 0;
        speed[2] = 0;
        accel[0] >>= 1;
        accel[1] >>= 1;
        accel[2] >>= 1;
        if (collisionPain > FIX(3)) {
            if (collisionPain > FIX(4)) {
                collisionPain = 0;
                goodHeading = heading;
            } else {
                accel[0] = -accel[0];
                accel[2] = -accel[2];
            }
        } else {
            if (happiness > 0)
                happiness >>= 1;
            else
                happiness -= FIX3(500);
        }

        checkCounter >>= 1;
        UndoMove();
        PlaceParts();
    } else {
        happiness -= zombieHappinessDecay;
        collisionPain -= collisionPain >> 3;
    }
}
