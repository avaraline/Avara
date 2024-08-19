/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CRealShooters.c
    Created: Saturday, August 10, 1996, 20:57
    Modified: Tuesday, August 27, 1996, 05:18
*/

#include "CRealShooters.h"

#include "CDepot.h"
#include "CShuriken.h"
#include "CSmartPart.h"

CRealShooters::CRealShooters() {
    visionRange = FIX(100);
    shotPower = FIX3(500);
    burstLength = 2;
    burstSpeed = 5;
    burstCharge = 40;

    watchTeams = -2;
    watchMask = kPlayerBit;
}

void CRealShooters::BeginScript() {
    CRealMovers::BeginScript();

    ProgramFixedVar(iRange, visionRange);
    ProgramFixedVar(iShotPower, shotPower);
    ProgramLongVar(iBurstLength, burstLength);
    ProgramLongVar(iBurstSpeed, burstSpeed);
    ProgramLongVar(iBurstCharge, burstCharge);

    ProgramLongVar(iMask, watchTeams);
    ProgramLongVar(iWatchMask, watchMask);
}

short CRealShooters::GetShooterId() {
    return -1;
}

CAbstractActor *CRealShooters::EndScript() {
    if (CRealMovers::EndScript()) {
        visionRange = ReadFixedVar(iRange);
        shotPower = ReadFixedVar(iShotPower);
        burstLength = ReadLongVar(iBurstLength);
        burstSpeed = FrameNumber(ReadLongVar(iBurstSpeed));
        burstCharge = FrameNumber(ReadLongVar(iBurstCharge));

        watchTeams = ReadLongVar(iMask);
        watchMask = ReadLongVar(iWatchMask);

        nextShotFrame = burstCharge;
        burstCount = burstLength;
        burstStartFrame = burstCharge;

        return this;
    } else
        return NULL;
}

Fixed EstimateTravelTime(Fixed *delta, Fixed *targSpeed, Fixed missileSpeed, Fixed initialT);

Fixed EstimateTravelTime(Fixed *delta, Fixed *targSpeed, Fixed missileSpeed, Fixed initialT) {
    Vector s;
    Vector v;
    Vector vs;
    Vector v2;
    Fixed divMS;
    Fixed ft, fdt;
    short i;//, j;

    divMS = FDivNZ(FIX1, missileSpeed);
    for (i = 0; i < 3; i++) {
        s[i] = FMul(delta[i], divMS);
        v[i] = FMul(targSpeed[i], divMS);
        vs[i] = FMul(v[i], s[i]);
        v2[i] = FMul(v[i], v[i]);
    }

    {
        ft = -FMul(initialT, initialT);
        fdt = -initialT;
        for (i = 0; i < 3; i++) {
            Fixed temp;

            temp = s[i] + FMul(v[i], initialT);
            ft += FMul(temp, temp);
            fdt += vs[i] + FMul(v2[i], initialT);
        }

        fdt += fdt;
        initialT -= FDiv(ft, fdt);
    }

    return initialT;
}

Boolean CRealShooters::Shoot() {
    RayHitRecord rayHit;
    RayHitRecord closestVisible;
    CAbstractActor *theTarget;
    Fixed *ploc;

    BuildActorProximityList(location, visionRange, watchMask);

    VECTORCOPY(rayHit.origin, location);

    closestVisible.distance = visionRange;
    closestVisible.closestHit = NULL;

    theTarget = proximityList.a;
    while (theTarget) {
        if (theTarget->teamMask & watchTeams) {
            ploc = theTarget->partList[0]->itsTransform[3];
            rayHit.direction[0] = ploc[0] - rayHit.origin[0];
            rayHit.direction[1] = ploc[1] - rayHit.origin[1];
            rayHit.direction[2] = ploc[2] - rayHit.origin[2];

            rayHit.distance = NormalizeVector(3, rayHit.direction);
            if (rayHit.distance < closestVisible.distance) {
                rayHit.closestHit = NULL;
                RayTest(&rayHit, kSolidBit);

                if (rayHit.closestHit && (rayHit.closestHit->theOwner == proximityList.a)) {
                    closestVisible = rayHit;
                }
            }
        }

        theTarget = theTarget->proximityList.a;
    }

    if (closestVisible.closestHit) {
        Vector delta;
        Vector estimate;
        Fixed frameEstimate;
        CAbstractMissile *theMissile;

        theTarget = closestVisible.closestHit->theOwner;
        ploc = theTarget->partList[0]->itsTransform[3];
        delta[0] = ploc[0] - location[0];
        delta[1] = ploc[1] - location[1];
        delta[2] = ploc[2] - location[2];

        theTarget->GetSpeedEstimate(estimate);
        frameEstimate = FDivNZ(closestVisible.distance, SHURIKENSPEED);
        frameEstimate = EstimateTravelTime(delta, estimate, SHURIKENSPEED, frameEstimate);
        frameEstimate = (frameEstimate + 0x8000) & 0xFFFF0000;

        ploc = theTarget->partList[0]->itsTransform[3];
        closestVisible.direction[0] = delta[0] + FMul(frameEstimate, estimate[0]);
        closestVisible.direction[1] = delta[1] + FMul(frameEstimate, estimate[1]);
        closestVisible.direction[2] = delta[2] + FMul(frameEstimate, estimate[2]);

        if (closestVisible.direction[1] + location[1] < 0) {
            closestVisible.direction[1] = -location[1];
        }

        NormalizeVector(3, closestVisible.direction);

        closestVisible.team = teamColor;
        closestVisible.playerId = GetShooterId();
        theMissile = itsGame->itsDepot->LaunchMissile(
            kmiShuriken, &partList[0]->itsTransform, &closestVisible, this, shotPower, NULL);
        if (theMissile)
            theMissile->TransparentStep();

        return true;
    } else
        return false;
}
