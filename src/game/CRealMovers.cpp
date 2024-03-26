/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CRealMovers.c
    Created: Friday, March 1, 1996, 17:20
    Modified: Sunday, August 25, 1996, 08:54
*/
// #define ENABLE_FPS_DEBUG  // uncomment if you want to see FPS_DEBUG output for this file

#include "CRealMovers.h"

#include "CSmartPart.h"

CRealMovers::CRealMovers() {
    maskBits |= kCanPushBit;
    baseMass = FIX(300);
    mass = FIX(50); //	Some default mass
    speed[0] = 0;
    speed[1] = 0;
    speed[2] = 0;
}

void CRealMovers::GetSpeedEstimate(Fixed *theSpeed) {
    theSpeed[0] = speed[0];
    theSpeed[1] = speed[1];
    theSpeed[2] = speed[2];
}

void CRealMovers::Push(Fixed *direction) {
    speed[0] = direction[0];
    speed[1] = direction[1];
    speed[2] = direction[2];
}

void CRealMovers::Accelerate(Fixed *direction) {
    Fixed theMass;

    isActive |= kHasImpulse;

    theMass = GetTotalMass();
    if (theMass) {
        Vector oldSpeed = {speed[0], speed[1], speed[2], 0};
        speed[0] += FMulDivNZ(direction[0], baseMass, theMass);
        speed[1] += FMulDivNZ(direction[1], baseMass, theMass);
        speed[2] += FMulDivNZ(direction[2], baseMass, theMass);
        FPS_DEBUG("CRealMovers::Accelerate: speed = " << FormatVectorFloat(speed, 3) << "\n");

        dSpeed[0] = speed[0] - oldSpeed[0];
        dSpeed[1] = speed[1] - oldSpeed[1];
        dSpeed[2] = speed[2] - oldSpeed[2];
    }
}

#include <iostream>
void CRealMovers::WasHit(RayHitRecord *theHit, Fixed hitEnergy) {
    Vector impulseMotion;
    Fixed impulsePower;

    CGlowActors::WasHit(theHit, hitEnergy);

    if (shields) {
        impulsePower = hitEnergy >> 1;
        impulseMotion[0] = FMul(impulsePower, theHit->direction[0]);
        impulseMotion[1] = FMul(impulsePower, theHit->direction[1]);
        impulseMotion[2] = FMul(impulsePower, theHit->direction[2]);

        Accelerate(impulseMotion);
    }
}

void CRealMovers::FindBestMovement(CSmartPart *objHit) {
    Vector newSpeed;
    Vector deltaSpeed, deltaLoc;
    Fixed dotProd;
    Fixed absDot;
    Fixed *norm;
    short i;
    Fixed bestDot;
    Vector bestSpeed;

    bestDot = FIX1;
    bestSpeed[0] = 0;
    bestSpeed[1] = 0;
    bestSpeed[2] = 0;
    deltaSpeed[0] = 0;
    deltaSpeed[1] = 0;
    deltaSpeed[2] = 0;

    for (i = 0; i < 3; i++) {
        norm = objHit->itsTransform[i];

        dotProd = FMul(norm[0], speed[0]) + FMul(norm[1], speed[1]) + FMul(norm[2], speed[2]);
        absDot = dotProd < 0 ? -dotProd : dotProd;

        if (absDot < bestDot) {
            newSpeed[0] = speed[0] - FMul(norm[0], dotProd);
            newSpeed[1] = speed[1] - FMul(norm[1], dotProd);
            newSpeed[2] = speed[2] - FMul(norm[2], dotProd);

            Fixed MINSPEED = FpsCoefficient2(FIX3(10));
            if (newSpeed[0] > MINSPEED || newSpeed[0] < -MINSPEED || newSpeed[1] > MINSPEED ||
                newSpeed[1] < -MINSPEED || newSpeed[2] > MINSPEED || newSpeed[2] < -MINSPEED) {

                deltaSpeed[0] = newSpeed[0] - deltaSpeed[0];
                deltaSpeed[1] = newSpeed[1] - deltaSpeed[1];
                deltaSpeed[2] = newSpeed[2] - deltaSpeed[2];

                deltaLoc[0] = FpsCoefficient2(deltaSpeed[0]);
                deltaLoc[1] = FpsCoefficient2(deltaSpeed[1]);
                deltaLoc[2] = FpsCoefficient2(deltaSpeed[2]);

                OffsetParts(deltaLoc);

                deltaSpeed[0] = newSpeed[0];
                deltaSpeed[1] = newSpeed[1];
                deltaSpeed[2] = newSpeed[2];
                if (!DoCollisionTest(&proximityList.p)) {
                    bestSpeed[0] = newSpeed[0];
                    bestSpeed[1] = newSpeed[1];
                    bestSpeed[2] = newSpeed[2];
                    bestDot = absDot;
                }
            }
        }
    }

    deltaSpeed[0] = bestSpeed[0] - deltaSpeed[0];
    deltaSpeed[1] = bestSpeed[1] - deltaSpeed[1];
    deltaSpeed[2] = bestSpeed[2] - deltaSpeed[2];

    location[0] += FpsCoefficient2(bestSpeed[0]);
    location[1] += FpsCoefficient2(bestSpeed[1]);
    location[2] += FpsCoefficient2(bestSpeed[2]);
    deltaLoc[0] = FpsCoefficient2(deltaSpeed[0]);
    deltaLoc[1] = FpsCoefficient2(deltaSpeed[1]);
    deltaLoc[2] = FpsCoefficient2(deltaSpeed[2]);

    speed[0] = bestSpeed[0];
    speed[1] = bestSpeed[1];
    speed[2] = bestSpeed[2];
    OffsetParts(deltaLoc);
}

void CRealMovers::StandingOn(CAbstractActor *who, //	Who is touching me?
    Fixed *where, //	Location of touch in global coordinates
    Boolean firstLeg) //	True, when this is the only or first leg to touch this object
{
    CGlowActors::StandingOn(who, where, firstLeg);

    if (firstLeg) {
        who->Slide(speed);
    }
}
