/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CParasite.c
    Created: Monday, November 20, 1995, 14:40
    Modified: Monday, August 26, 1996, 03:04
*/

#include "CParasite.h"

#include "CAbstractPlayer.h"
#include "CSmartPart.h"

#define kBSPParasite 807
#define kParasiteAttachSound 311
#define kCourseCheckPeriod 6
#define kAngleStep FIX3(20)
#define kSlowAngleStep FIX3(5)
#define kParasiteDefaultShield FIX3(200)
#define kParasiteFriction FIX3(950)
#define kParasiteDeathSound 310

void CParasite::BeginScript() {
    mass = FIX(50);
    maskBits |= kTargetBit + kSolidBit + kRobotBit + kCollisionDamageBit;
    destructScore = 10;
    blastVolume >>= 2;
    hostPart = NULL;
    hostIdent = 0;
    energyDrain = FIX3(10);
    maxPower = FIX3(1000);
    shields = kParasiteDefaultShield;
    acceleration = FIX3(20);

    CRealMovers::BeginScript();

    ProgramFixedVar(iDrain, energyDrain);
    ProgramFixedVar(iMaxPower, maxPower);
    ProgramFixedVar(iAccelerate, acceleration);
    ProgramLongVar(iShape, kBSPParasite);
    ProgramLongVar(iSound, kParasiteAttachSound);
    ProgramFixedVar(iVolume, FIX3(500));
    ProgramLongVar(iRange, 40);
    ProgramLongVar(iBlastSound, kParasiteDeathSound);
}

CAbstractActor *CParasite::EndScript() {
    if (CRealMovers::EndScript()) {
        auto _ = AssetManager::GetOgg(clampSound = ReadLongVar(iSound));
        clampVolume = ReadFixedVar(iVolume);
        maxPower = ReadFixedVar(iMaxPower);
        energyDrain = ReadFixedVar(iDrain);
        range = ReadFixedVar(iRange);

        partCount = 1;
        LoadPartWithColors(0, ReadLongVar(iShape));
        location[1] -= partList[0]->minBounds.x;

        acceleration = ReadFixedVar(iAccelerate);
        pitch = 0x4000;

        sleepTimer = ident & 7;
        isActive = kIsActive;
        moveClock = 0;

        course[0] = course[1] = course[2] = 0;
        speed[0] = speed[1] = speed[2] = 0;

        PlaceParts();

        return this;
    } else
        return NULL;
}

void CParasite::ReleaseAttachment() {
    CAbstractActor *host;

    if (hostPart) {
        host = itsGame->FindIdent(hostIdent);
        if (host) {
            host->Detach(&clamp);
            hostIdent = 0;
            hostPart = NULL;
            location[0] = partList[0]->itsTransform[3][0];
            location[1] = partList[0]->itsTransform[3][1];
            location[2] = partList[0]->itsTransform[3][2];
        }
    }
}
void CParasite::Dispose() {
    ReleaseAttachment();

    CRealMovers::Dispose();
}

void CParasite::PlaceParts() {
    if (hostPart) {
        partList[0]->CopyTransform(&relation);
        partList[0]->ApplyMatrix(&hostPart->itsTransform);
    } else {
        partList[0]->Reset();
        InitialRotatePartX(partList[0], pitch);
        partList[0]->RotateOneY(heading);
        TranslatePart(partList[0], location[0], location[1], location[2]);
    }

    partList[0]->MoveDone();
    LinkSphere(partList[0]->itsTransform[3], partList[0]->bigRadius);

    CRealMovers::PlaceParts();
}

void CParasite::ClampOn(CSmartPart *clampTo) {
    CAbstractPlayer *host;
    Matrix *invHost;

    hostPart = clampTo;
    host = (CAbstractPlayer *)hostPart->theOwner;
    invHost = hostPart->GetInverseTransform();

    CombineTransforms(&partList[0]->itsTransform, &relation, invHost);
    DoSound(clampSound, location, clampVolume, FIX1);

    clamp.me = this;
    hostIdent = host->Attach(&clamp);
}

void CParasite::CourseCheck() {
    CAbstractPlayer *aPlayer;
    Fixed bestDistance;
    RayHitRecord ray;

    isTracking = false;

    aPlayer = itsGame->playerList;
    bestDistance = range;
    ray.origin[0] = location[0];
    ray.origin[1] = location[1];
    ray.origin[2] = location[2];

    while (aPlayer) {
        Fixed dist;
        Fixed *ploc;
        Vector delta;

        ploc = aPlayer->partList[0]->itsTransform[3];

        ray.direction[0] = delta[0] = ploc[0] - location[0];
        ray.direction[1] = delta[1] = ploc[1] - location[1];
        ray.direction[2] = delta[2] = ploc[2] - location[2];
        dist = FDistanceOverEstimate(ray.direction[0], ray.direction[1], ray.direction[2]);

        if (dist < bestDistance) {
            NormalizeVector(3, ray.direction);
            //	Could also be: FindSpaceAngleAndNormalize(ray.direction, &goodHeading, &goodPitch);
            ray.distance = dist;
            ray.closestHit = NULL;
            RayTest(&ray, kSolidBit);

            if (ray.closestHit && (ray.closestHit->theOwner->maskBits & kPlayerBit)) {
                delta[0] -= speed[0] * (kCourseCheckPeriod / 2);
                //	delta[1] -= speed[1] * (kCourseCheckPeriod/2);
                delta[2] -= speed[2] * (kCourseCheckPeriod / 2);
                if (delta[0] || delta[1] || delta[2])
                    NormalizeVector(3, delta);
                else
                    VECTORCOPY(delta, ray.direction);

                isTracking = true;
                course[0] = FMul(delta[0], acceleration);
                course[1] = FMul(delta[1], acceleration);
                course[2] = FMul(delta[2], acceleration);
                bestDistance = ray.distance;
                FindSpaceAngle(ray.direction, &goodHeading, &goodPitch);
            }
        }
        aPlayer = aPlayer->nextPlayer;
    }
}

void CParasite::FrameAction() {
    Fixed oldHeading;
    Fixed oldPitch;
    CSmartPart *hitPart;

    CRealMovers::FrameAction();

    if (hostPart) {
        CAbstractPlayer *host;

        host = (CAbstractPlayer *)hostPart->theOwner;
        if (host->energy > energyDrain) {
            host->energy -= energyDrain;
            blastPower += energyDrain >> 1;
        } else {
            blastPower += host->energy >> 1;
            host->energy = 0;
        }

        if (host->shields > (energyDrain >> 1)) {
            host->shields -= energyDrain >> 1;
            shields += energyDrain >> 3;
        } else {
            maxPower = blastPower;
        }

        if (blastPower >= maxPower) {
            WasDestroyed();
            SecondaryDamage(teamColor, -1, ksiParasiteBlast);
            return;
        }
    } else {
        short delta;

        if (--moveClock <= 0) {
            CourseCheck();
            moveClock = kCourseCheckPeriod;
        }

        oldHeading = heading;
        oldPitch = pitch;

        if (isTracking) {
            delta = goodHeading - heading;
            if (delta < -kAngleStep)
                heading -= kAngleStep;
            else if (delta > kAngleStep)
                heading += kAngleStep;
            else
                heading = goodHeading;
            heading = (short)heading;

            delta = goodPitch - pitch;
            if (delta < -kAngleStep)
                pitch -= kAngleStep;
            else if (delta > kAngleStep)
                pitch += kAngleStep;
            else
                pitch = goodPitch;
        } else {
            course[0] >>= 1;
            if (course[0] == -1)
                course[0] = 0;
            course[2] >>= 1;
            course[2]++;
            if (course[2] == -1)
                course[2] = 0;
            if (location[1] > -partList[0]->minBounds.x)
                course[1] = FIX3(-10);
            else
                course[1] = 0;

            delta = 0x4000 - pitch;
            if (delta < -kSlowAngleStep)
                pitch -= kSlowAngleStep;
            else if (delta > kSlowAngleStep)
                pitch += kSlowAngleStep;
            else
                pitch = 0x4000;
        }

        if (pitch != oldPitch || oldHeading != heading || speed[0] || speed[1] || speed[2]) {
            location[0] += speed[0];
            location[1] += speed[1];
            if (location[1] < -partList[0]->minBounds.x) {
                location[1] = -partList[0]->minBounds.x;
                speed[1] = 0;
            }
            location[2] += speed[2];

            PlaceParts();
            BuildPartProximityList(location, partList[0]->enclosureRadius, kSolidBit);

            if ((hitPart = DoCollisionTest(&proximityList.p))) {
                if (hitPart->theOwner->maskBits & kPlayerBit) {
                    Vector origLoc;

                    origLoc[0] = location[0] - speed[0];
                    origLoc[1] = location[1] - speed[1];
                    origLoc[2] = location[2] - speed[2];
                    heading = oldHeading;
                    pitch = oldPitch;

                    do {
                        speed[0] /= 2;
                        speed[1] /= 2;
                        speed[2] /= 2;
                        location[0] = origLoc[0] + speed[0];
                        location[1] = origLoc[1] + speed[1];
                        location[2] = origLoc[2] + speed[2];
                        PlaceParts();
                    } while (DoCollisionTest(&proximityList.p) && (speed[0] || speed[1] || speed[2]));

                    ClampOn(hitPart);
                } else {
                    location[0] -= speed[0];
                    location[1] -= speed[1];
                    location[2] -= speed[2];
                    heading = oldHeading;
                    pitch = oldPitch;
                    PlaceParts();

                    speed[0] >>= 1;
                    speed[1] >>= 1;
                    speed[2] >>= 1;
                }
            }

            speed[0] = FMul(speed[0] + course[0], kParasiteFriction);
            speed[1] = FMul(speed[1] + course[1], kParasiteFriction);
            speed[2] = FMul(speed[2] + course[2], kParasiteFriction);
        }
    }
}
