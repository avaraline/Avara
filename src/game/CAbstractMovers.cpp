/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CAbstractMovers.c
    Created: Tuesday, December 5, 1995, 06:03
    Modified: Tuesday, February 20, 1996, 10:37
*/

#include "CAbstractMovers.h"

CAbstractMovers::CAbstractMovers() {
    mass = FIX(50); //	Some default mass
    speed[0] = 0;
    speed[1] = 0;
    speed[2] = 0;
    internalImpulse[0] = 0;
    internalImpulse[1] = 0;
    internalImpulse[2] = 0;
    impulse[0] = 0;
    impulse[1] = 0;
    impulse[2] = 0;
    impulseSlowDown = FIX3(800);
}

void CAbstractMovers::GetSpeedEstimate(Fixed *theSpeed) {
    *theSpeed++ = speed[0] + internalImpulse[0];
    *theSpeed++ = speed[1] + internalImpulse[1];
    *theSpeed++ = speed[2] + internalImpulse[2];
}

void CAbstractMovers::Push(Fixed *direction) {
    Vector curSpeed;

    isActive |= kHasImpulse;
    GetSpeedEstimate(curSpeed);
    internalImpulse[0] += direction[0] - curSpeed[0];
    internalImpulse[1] += direction[1] - curSpeed[1];
    internalImpulse[2] += direction[2] - curSpeed[2];
}

void CAbstractMovers::Accelerate(Fixed *direction) {
    Fixed theMass;

    isActive |= kHasImpulse;

    theMass = GetTotalMass();
    if (theMass) {
        internalImpulse[0] += FMulDivNZ(direction[0], FIX(300), theMass);
        internalImpulse[1] += FMulDivNZ(direction[1], FIX(300), theMass);
        internalImpulse[2] += FMulDivNZ(direction[2], FIX(300), theMass);
    }
}

void CAbstractMovers::WasHit(RayHitRecord *theHit, Fixed hitEnergy) {
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

void CAbstractMovers::FrameAction() {
    CGlowActors::FrameAction();

    if (isActive & kHasImpulse) {
        impulse[0] = internalImpulse[0];
        impulse[1] = internalImpulse[1];
        impulse[2] = internalImpulse[2];

        internalImpulse[0] = FMul(internalImpulse[0], impulseSlowDown);
        internalImpulse[1] = FMul(internalImpulse[1], impulseSlowDown);
        internalImpulse[2] = FMul(internalImpulse[2], impulseSlowDown);
        if (!(internalImpulse[0] || internalImpulse[1] || internalImpulse[2])) {
            isActive &= ~kHasImpulse;
        }
    }
}
