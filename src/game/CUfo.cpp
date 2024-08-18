/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CUfo.c
    Created: Monday, December 4, 1995, 03:09
    Modified: Thursday, September 12, 1996, 00:23
*/

#include "CUfo.h"

#include "CAbstractPlayer.h"
#include "CDepot.h"
#include "CShuriken.h"
#include "CSmartPart.h"

#define kAngleStep FIX3(2)
#define kAngleTolerance FIX3(8)

#define kUfoFloatFriction 0.99
#define kUfoFriction FIX(kUfoFloatFriction)
#define kUfoCoastDistance ((long)(0.5 / (1 - kUfoFloatFriction)))
#define kUfoTurnFriction FIX3(950)
#define kUfoCourseCorrector 8

void CUfo::BeginScript() {
    sliverCounts[kSmallSliver] = 10;
    sliverLives[kSmallSliver] = 10;

    sliverCounts[kMediumSliver] = 5;
    sliverLives[kMediumSliver] = 10;

    maskBits |= kTargetBit + kSolidBit + kRobotBit + kCollisionDamageBit;
    mass = FIX(300);
    destructScore = 500;
    hitScore = 100;
    shields = FIX(5);

    CRealShooters::BeginScript();

    ProgramLongVar(iShape, 808);

    ProgramLongVar(iMotionRange, 40);
    ProgramLongVar(iPitchRange, 17);
    ProgramLongVar(iVerticalRangeMin, 0);
    ProgramLongVar(iVerticalRangeMax, 5);
    ProgramLongVar(iCheckPeriod, 32);
    ProgramFixedVar(iAccelerate, FIX3(12));

    ProgramFixedVar(iAttack, FIX3(1000));
    ProgramFixedVar(iDefense, FIX3(-600));
    ProgramFixedVar(iVisionScore, FIX3(1000));
    ProgramFixedVar(iHideScore, FIX3(300));

    ProgramMessage(iStartMsg, iStartMsg);
    ProgramLongVar(iStopMsg, 0);
    ProgramLongVar(iStatus, false);

    ProgramFixedVar(iHomeSick, FIX3(100));
    ProgramLongVar(iHomeRange, -1);
    ProgramLongVar(iHomeBase, 0);
}

CAbstractActor *CUfo::EndScript() {
    if (CRealShooters::EndScript()) {
        short shapeRes;

        shapeRes = ReadLongVar(iShape);

        motionRange = ReadFixedVar(iMotionRange);
        pitchRange = FDegToOne(ReadFixedVar(iPitchRange));
        verticalMin = ReadFixedVar(iVerticalRangeMin);
        verticalMax = ReadFixedVar(iVerticalRangeMax);
        checkPeriod = ReadLongVar(iCheckPeriod);
        acceleration = ReadFixedVar(iAccelerate);

        homeSick = ReadFixedVar(iHomeSick);
        homeRange = ReadFixedVar(iHomeRange);

        attack = ReadFixedVar(iAttack);
        defense = ReadFixedVar(iDefense);
        visionScore = ReadFixedVar(iVisionScore);
        hideScore = ReadFixedVar(iHideScore);

        RegisterReceiver(&startMsg, ReadLongVar(iStartMsg));
        RegisterReceiver(&stopMsg, ReadLongVar(iStopMsg));
        RegisterReceiver(&homeMsg, ReadLongVar(iHomeBase));
        status = ReadLongVar(iStatus);

        partCount = 1;
        LoadPartWithColors(0, shapeRes);
        if (location[1] < -partList[0]->minBounds.y)
            location[1] = -partList[0]->minBounds.y;

        VECTORCOPY(homeBase, location);
        VECTORCOPY(destination, location);
        course[0] = course[1] = course[2] = 0;
        speed[0] = speed[1] = speed[2] = 0;

        if (verticalMin < -partList[0]->minBounds.y)
            verticalMin = -partList[0]->minBounds.y;

        if (verticalMax < partList[0]->bigRadius)
            verticalMax = partList[0]->bigRadius;

        if (homeBase[1] < verticalMin || homeBase[1] > verticalMax) {
            homeBase[1] = (verticalMin + verticalMax) >> 1;
        }

        happiness = 0;
        currentBias = attack;

        pitch = 0;
        roll = 0;
        turnRate = 0;
        moveClock = 0;
        isMoving = false;

        sleepTimer = ident & 7;
        isActive = kIsActive;

        PlaceParts();

        return this;
    } else
        return NULL;
}

void CUfo::PlaceParts() {
    partList[0]->Reset();
    InitialRotatePartZ(partList[0], roll);
    partList[0]->RotateOneX(pitch);
    partList[0]->RotateOneY(heading);
    TranslatePart(partList[0], location[0], location[1], location[2]);

    partList[0]->MoveDone();
    LinkSphere(partList[0]->itsTransform[3], partList[0]->bigRadius);

    CRealShooters::PlaceParts();
}

void CUfo::Navigate() {
    Vector delta;
    short alpha;
    Fixed dist;

    delta[0] = destination[0] - location[0];
    delta[1] = destination[1] - location[1];
    delta[2] = destination[2] - location[2];

    dist = FDistanceEstimate(delta[0], delta[1], delta[2]);

    if (dist < FIX3(100)) {
        course[0] = -speed[0] >> 1;
        course[1] = -speed[1] >> 1;
        course[2] = -speed[2] >> 1;

        goodHeading = heading;
        goodPitch = pitch >> 1;
        moveClock -= moveClock >> 2;
        happiness -= (happiness >> 4) + 2;
    } else {
        FindSpaceAngle(delta, &goodHeading, &goodPitch);

        delta[0] -= speed[0] * kUfoCourseCorrector;
        delta[1] -= speed[1] * kUfoCourseCorrector;
        delta[2] -= speed[2] * kUfoCourseCorrector;

        if (delta[0] || delta[1] || delta[2])
            NormalizeVector(3, delta);

        course[0] = FMul(delta[0], acceleration);
        course[1] = FMul(delta[1], acceleration);
        course[2] = FMul(delta[2], acceleration);
    }

    alpha = goodHeading - heading - turnRate;
    if (alpha < -kAngleTolerance)
        turnRate -= kAngleStep;
    else if (alpha > kAngleTolerance)
        turnRate += kAngleStep;

    heading += turnRate;
    turnRate = FMul(turnRate, kUfoTurnFriction);

    alpha = goodPitch - pitch;
    if (alpha < -kAngleStep)
        pitch -= kAngleStep;
    else if (alpha > kAngleStep)
        pitch += kAngleStep;
    else
        pitch = goodPitch;

    location[0] += speed[0];
    location[1] += speed[1];
    if (location[1] < -partList[0]->minBounds.y) {
        location[1] = -partList[0]->minBounds.y;
        speed[1] = 0;
    }
    location[2] += speed[2];

    roll = -turnRate << 1;
}

Fixed CUfo::EvaluatePosition(Fixed *position, Boolean doAttack) {
    RayHitRecord rayHit;
    Fixed placeScore = 0;
    CAbstractActor *theTarget;
    Fixed nearVision = visionRange >> 3;

    if (motionRange) {
        if (homeRange >= 0) {
            placeScore =
                homeRange -
                FDistanceEstimate(homeBase[0] - position[0], homeBase[1] - position[1], homeBase[2] - position[2]);
            if (placeScore > 0) {
                placeScore = 0;
            } else {
                placeScore = FMul(placeScore, homeSick) + FMul(FRandom(), homeRange >> 2);
            }
        } else {
            placeScore =
                FDistanceEstimate(homeBase[0] - position[0], homeBase[1] - position[1], homeBase[2] - position[2]);

            placeScore += motionRange + FMul(FRandom(), motionRange);

            if (placeScore > FIX3(1)) {
                placeScore = FMulDivNZ(homeSick, motionRange, placeScore);
            }
        }

        BuildActorProximityList(position, visionRange, watchMask);

        VECTORCOPY(rayHit.origin, position);

        theTarget = proximityList.a;
        while (theTarget) {
            Fixed dist;
            Fixed *ploc;
            Vector delta;

            if (theTarget->teamMask & watchTeams) {
                ploc = theTarget->partList[0]->itsTransform[3];

                rayHit.direction[0] = delta[0] = ploc[0] - rayHit.origin[0];
                rayHit.direction[1] = delta[1] = ploc[1] - rayHit.origin[1];
                rayHit.direction[2] = delta[2] = ploc[2] - rayHit.origin[2];

                dist = FDistanceOverEstimate(rayHit.direction[0], rayHit.direction[1], rayHit.direction[2]);
                if (doAttack) {
                    if (dist >= nearVision)
                        placeScore += FMulDivNZ(hideScore, nearVision, dist);
                    else
                        placeScore += FMulDivNZ(hideScore, dist, visionRange >> 2) - hideScore;
                }

                NormalizeVector(3, rayHit.direction);
                rayHit.distance = dist;
                rayHit.closestHit = NULL;
                RayTest(&rayHit, kSolidBit);

                if (rayHit.closestHit && (rayHit.closestHit->theOwner == proximityList.a)) {
                    if (doAttack) {
                        if (rayHit.distance >= nearVision)
                            placeScore += FMulDivNZ(visionScore, nearVision, rayHit.distance);
                        else
                            placeScore += FMulDivNZ(visionScore, rayHit.distance, visionRange >> 2) - visionScore;
                    } else {
                        placeScore -= visionScore;
                    }
                }
            }

            theTarget = theTarget->proximityList.a;
        }
    }

    return placeScore;
}

extern Boolean showDebug;

void CUfo::ReTarget() {
    RayHitRecord rayHit;
    Vector newSpot;
    Fixed newScore, currentScore;
    short dir;
    Fixed randRange;
    Boolean doAttack;

    if (motionRange) {
        randRange = FMul(motionRange - (motionRange >> 2), FRandom()) + (motionRange >> 2);
        dir = 2 * (FRandom() - FIX3(500));
        if (dir < 0)
            dir = heading - FMul(dir, dir);
        else
            dir = heading + FMul(dir, dir);

        newSpot[0] = location[0] + FMul(FOneCos(dir), randRange);
        newSpot[2] = location[2] + FMul(FOneSin(dir), randRange);

        dir = FMul(FRandom() - FIX3(500), pitchRange);
        newSpot[1] = location[1] + FMul(FOneSin(dir), randRange);

        if (newSpot[1] < verticalMin)
            newSpot[1] = verticalMin;
        else if (newSpot[1] > verticalMax)
            newSpot[1] = verticalMax;

        rayHit.direction[0] = newSpot[0] - location[0];
        rayHit.direction[1] = newSpot[1] - location[1];
        rayHit.direction[2] = newSpot[2] - location[2];

        rayHit.distance = NormalizeVector(3, rayHit.direction) + partList[0]->bigRadius;

        VECTORCOPY(rayHit.origin, location);
        rayHit.closestHit = NULL;

        RayTest(&rayHit, kSolidBit);

        if (rayHit.closestHit) {
            Fixed distance;

            distance = rayHit.distance - 2 * partList[0]->bigRadius;

            if (distance < 0)
                return;

            newSpot[0] = rayHit.origin[0] + FMul(distance, rayHit.direction[0]);
            newSpot[1] = rayHit.origin[1] + FMul(distance, rayHit.direction[1]);
            newSpot[2] = rayHit.origin[2] + FMul(distance, rayHit.direction[2]);
        }

        doAttack = FRandom() < currentBias;
        newScore = EvaluatePosition(newSpot, doAttack);
        currentScore = happiness + EvaluatePosition(destination, doAttack);

        moveClock = checkPeriod;
        if (newScore > currentScore) {
            happiness = FIX3(50);
            VECTORCOPY(destination, newSpot);
            //		showDebug = !showDebug;
        } else {
            if (!doAttack) {
                moveClock >>= 2;
            }
        }
    }
}

void CUfo::WasHit(RayHitRecord *theHit, Fixed hitEnergy) {
    CRealShooters::WasHit(theHit, hitEnergy);

    happiness -= hitEnergy >> 2;
    moveClock -= 2;
    currentBias = (currentBias + defense) >> 1;
}

void CUfo::FrameAction() {
    Vector oldLocation;
    Fixed oldHeading = heading;
    Fixed oldPitch = pitch;
    Fixed oldTurnRate = turnRate;
    Fixed oldRoll = roll;
    CSmartPart *hitPart;

    if (isActive & kHasMessage) {
        if (stopMsg.triggerCount > startMsg.triggerCount)
            status = false;
        else if (stopMsg.triggerCount < startMsg.triggerCount)
            status = true;

        stopMsg.triggerCount = 0;
        startMsg.triggerCount = 0;
        homeMsg.triggerCount = 0;

        isActive &= ~kHasMessage;
    }

    VECTORCOPY(oldLocation, location);

    CRealShooters::FrameAction();

    if (status) {
        if (--moveClock <= 0) {
            ReTarget();
        }

        if (itsGame->frameNumber >= nextShotFrame) {
            Boolean didShoot;

            didShoot = Shoot();
            if (didShoot) {
                if (burstCount == burstLength) {
                    burstStartFrame = itsGame->FramesFromNow(burstCharge);
                }

                if (--burstCount == 0) {
                    burstCount = burstLength;
                    nextShotFrame = burstStartFrame;
                } else {
                    nextShotFrame = itsGame->FramesFromNow(burstSpeed);
                }
            }
        }
    }

    currentBias += (attack - currentBias) >> 7;

    Navigate();
    isMoving = true;

    if (isMoving) {
        PlaceParts();

        BuildPartProximityList(location, partList[0]->bigRadius * 2, kSolidBit);

        if ((hitPart = DoCollisionTest(&proximityList.p))) {
            roll = oldRoll;
            heading = oldHeading;
            turnRate = oldTurnRate;
            pitch = oldPitch;
            PlaceParts();
            moveClock -= 2;
            happiness -= FIX3(10);

            if (DoCollisionTest(&proximityList.p)) {
                Vector delta;
                short i;

                speed[0] >>= 1;
                speed[1] >>= 1;
                speed[2] >>= 1;
                delta[0] = location[0] - oldLocation[0];
                delta[1] = location[1] - oldLocation[1];
                delta[2] = location[2] - oldLocation[2];
                VECTORCOPY(location, oldLocation);

                for (i = 0; i < 3; i++) {
                    location[i] += delta[i];
                    PlaceParts();
                    hitPart = DoCollisionTest(&proximityList.p);
                    if (hitPart) {
                        if ((delta[i] > 0) ^ (course[i] < 0)) {
                            course[i] = -2 * course[i];
                            speed[i] /= 2;
                        }
                        location[i] -= delta[i];
                    }
                }

                if (hitPart) {
                    PlaceParts();
                }
            }
        }
        speed[0] = FMul(speed[0] + course[0], kUfoFriction);
        speed[1] = FMul(speed[1] + course[1], kUfoFriction);
        speed[2] = FMul(speed[2] + course[2], kUfoFriction);
    }
}

void CUfo::ImmediateMessage() {
    if (homeMsg.triggerCount) {
        homeMsg.triggerCount = 0;
        if (itsGame->baseLocation) {
            Fixed *v;

            v = itsGame->baseLocation;
            homeBase[0] = v[0];
            homeBase[1] = v[1];
            homeBase[2] = v[2];

            if (v[3] >= 0)
                verticalMin = v[3];

            if (v[4] >= 0)
                verticalMax = v[4];
        }
    }

    CRealShooters::ImmediateMessage();
}
