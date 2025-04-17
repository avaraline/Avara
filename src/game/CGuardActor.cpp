/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CGuardActor.c
    Created: Saturday, December 17, 1994, 02:51
    Modified: Monday, August 26, 1996, 03:06
*/

#include "CGuardActor.h"

#include "AvaraDefines.h"
#include "CDepot.h"
#include "CMissile.h"
#include "CSmartPart.h"

void CGuardActor::BeginScript() {
    maskBits |= kTargetBit + kSolidBit + kRobotBit;
    shields = FIX(5);
    hitScore = 50;
    destructScore = 100;
    blastPower = FIX(2);
    sliverCounts[kSmallSliver] = 8;
    sliverCounts[kMediumSliver] = 2;
    location[1] = FIX(2);

    CGlowActors::BeginScript();

    ProgramLongVar(iFireMsg, 0);
    ProgramLongVar(iStartMsg, 0);
    ProgramLongVar(iStopMsg, 0);
    ProgramLongVar(iWatchMask, kPlayerBit);
    ProgramLongVar(iMask, -1);
    ProgramLongVar(iSpeed, 1);
    ProgramLongVar(iFrequency, 20);
    ProgramFixedVar(iShotPower, FIX3(800));
    ProgramLongVar(iShape, kGuardBSP);
    ProgramFixedVar(iRange, STANDARDMISSILERANGE);
}

CAbstractActor *CGuardActor::EndScript() {
    CGlowActors::EndScript();

    maskBits |= kSolidBit;
    isActive = kIsActive;

    RegisterReceiver(&fireActivator, ReadLongVar(iFireMsg));
    RegisterReceiver(&trackingActivator, ReadLongVar(iStartMsg));
    RegisterReceiver(&trackingDeactivator, ReadLongVar(iStopMsg));

    fireFrequency = ReadLongVar(iFrequency);
    rotSpeed = FDegToOne(ReadFixedVar(iSpeed));
    watchBits = ReadLongVar(iWatchMask);
    watchTeams = ReadLongVar(iMask);

    shotPower = ReadFixedVar(iShotPower);
    visionRange = ReadFixedVar(iRange);

    pitch = 0;
    fireCount = 0;

    LoadPartWithColors(0, ReadLongVar(iShape));

    PlaceParts();
    LinkPartSpheres();

    partCount = 1;
    isTracking = !(trackingActivator.messageId);

    return this;
}

CGuardActor::~CGuardActor() {
    itsGame->RemoveReceiver(&fireActivator);
    itsGame->RemoveReceiver(&trackingActivator);
    itsGame->RemoveReceiver(&trackingDeactivator);
}

void CGuardActor::PlaceParts() {
    UnlinkLocation();
    partList[0]->Reset();
    InitialRotatePartX(partList[0], pitch);
    partList[0]->RotateOneY(heading);
    TranslatePart(partList[0], location[0], location[1], location[2]);
    partList[0]->MoveDone();

    CGlowActors::PlaceParts();
}

CSmartPart *CGuardActor::FindClosestTarget() {
    CAbstractActor *theActor;
    CSmartPart *thePart;
    CSmartPart *closestPart = NULL;
    Fixed closest = visionRange;
    Fixed current;

    BuildActorProximityList(location, closest, watchBits);
    theActor = proximityList.a;

    while (theActor) {
        if (theActor->teamMask & watchTeams) {
            thePart = theActor->partList[0];
            if (thePart) {
                current = FDistanceEstimate(location[0] - thePart->modelTransform[3][0],
                    location[1] - thePart->modelTransform[3][1],
                    location[2] - thePart->modelTransform[3][2]);
                if (current < closest) {
                    closest = current;
                    closestPart = thePart;
                }
            }
        }
        theActor = theActor->proximityList.a;
    }

    return closestPart;
}

void CGuardActor::FrameAction() {
    CGlowActors::FrameAction();

    if (isActive & kHasMessage) {
        if (trackingActivator.triggerCount) {
            trackingActivator.triggerCount = 0;
            isTracking = true;
        }

        if (trackingDeactivator.triggerCount) {
            trackingDeactivator.triggerCount = 0;
            isTracking = false;
        }
    }

    fireCount--;

    if (isTracking) {
        CSmartPart *target;

        isActive = kIsActive;
        target = FindClosestTarget();
        if (target) {
            Vector delta;
            short angleChange;
            Fixed oldHeading;
            Fixed oldPitch;
            Fixed goodHeading;
            Fixed goodPitch;

            oldHeading = heading;
            oldPitch = pitch;
            delta[0] = target->sphereGlobCenter[0] - location[0];
            delta[1] = target->sphereGlobCenter[1] - location[1];
            delta[2] = target->sphereGlobCenter[2] - location[2];

            FindSpaceAngle(delta, &goodHeading, &goodPitch);

            angleChange = goodHeading - heading;
            if (angleChange < -rotSpeed)
                heading -= rotSpeed;
            else if (angleChange > rotSpeed)
                heading += rotSpeed;
            else
                heading = goodHeading;
            heading = (short)heading;

            angleChange = goodPitch - pitch;
            if (angleChange < -rotSpeed)
                pitch -= rotSpeed;
            else if (angleChange > rotSpeed)
                pitch += rotSpeed;
            else
                pitch = goodPitch;
            pitch = (short)pitch;

            PlaceParts();
            BuildPartSpheresProximityList(kSolidBit);
            if (DoCollisionTest(&proximityList.p)) {
                heading = oldHeading;
                pitch = oldPitch;
                PlaceParts();
            }
            LinkPartSpheres();
        }
    } else {
        if (fireCount == 0)
            isActive = false;
    }

    if (fireCount < 0) {
        fireCount = 0;

        if (isTracking || fireActivator.triggerCount) {
            RayHitRecord theHit;
            Vector *gunMatrix;
            CAbstractActor *theActor;
            Fixed groundDist;

            gunMatrix = partList[0]->modelTransform;
            fireCount = fireFrequency;

            theHit.distance = STANDARDMISSILERANGE;
            theHit.closestHit = NULL;
            theHit.origin[0] = gunMatrix[3][0];
            theHit.origin[1] = gunMatrix[3][1];
            theHit.origin[2] = gunMatrix[3][2];
            theHit.origin[3] = FIX1;

            theHit.direction[0] = gunMatrix[2][0];
            theHit.direction[1] = gunMatrix[2][1];
            theHit.direction[2] = gunMatrix[2][2];
            theHit.direction[3] = 0;
            NormalizeVector(3, theHit.direction);

            groundDist = theHit.direction[1];

            if (groundDist < -FIX3(1)) {
                groundDist = FDivNZ(theHit.origin[1], -groundDist);
                if (groundDist < theHit.distance) {
                    theHit.distance = groundDist;
                }
            }

            if (!fireActivator.triggerCount)
                RayTest(&theHit, kSolidBit);

            if (theHit.closestHit) {
                theActor = theHit.closestHit->theOwner;
                if (theActor->maskBits & watchBits) {
                    CAbstractMissile *theMissile;

                    theHit.team = teamColor;
                    theHit.playerId = -1;
                    theMissile = itsGame->itsDepot->LaunchMissile(
                        kmiFlat, &partList[0]->modelTransform, &theHit, this, shotPower, NULL);
                    if (theMissile)
                        theMissile->TransparentStep();
                }
            } else if (fireActivator.triggerCount) {
                CAbstractMissile *theMissile;

                theHit.team = teamColor;
                theHit.playerId = -1;
                theMissile = itsGame->itsDepot->LaunchMissile(
                    kmiFlat, &partList[0]->modelTransform, &theHit, this, shotPower, NULL);
                if (theMissile)
                    theMissile->TransparentStep();
            }
        }

        fireActivator.triggerCount = 0;
    }
}

bool CGuardActor::IsGeometryStatic()
{
    if (!CGlowActors::IsGeometryStatic()) {
        return false;
    }
    
    if (trackingActivator.messageId > 0 && rotSpeed != 0) {
        return false;
    }
    
    return true;
}
