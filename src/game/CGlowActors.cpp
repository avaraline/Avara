/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CGlowActors.c
    Created: Wednesday, March 15, 1995, 07:06
    Modified: Tuesday, July 30, 1996, 08:40
*/

#include "CGlowActors.h"

#include "CSmartPart.h"

extern Fixed FRandSeed;

void CGlowActors::IAbstractActor() {
    CPlacedActors::IAbstractActor();
    canGlow = true;
    glow = 0;
}
void CGlowActors::BeginScript() {
    CPlacedActors::BeginScript();

    if (shields >= 0) {
        ProgramLongVar(iHitSound, ReadLongVar(iShieldHitSoundDefault));
    }

    ProgramLongVar(iCanGlow, shields >= 0);
}

CAbstractActor *CGlowActors::EndScript() {
    glow = 0;
    canGlow = ReadLongVar(iCanGlow);

    return CPlacedActors::EndScript();
}

void CGlowActors::WasHit(RayHitRecord *theHit, Fixed hitEnergy) {
    CPlacedActors::WasHit(theHit, hitEnergy);

    if (canGlow) {
        glow += hitEnergy + hitEnergy;
        if (glow > FIX3(1800))
            glow = FIX3(1800);
        isActive |= kIsGlowing;
    }
}

void CGlowActors::FrameAction() {
    CSmartPart **thePart;

    CPlacedActors::FrameAction();

    if (glow) {
        Fixed doubleGlow;

        glow -= (glow >> 1) + 1;
        if (glow <= 0) {
            glow = 0;
            isActive &= ~kIsGlowing;
        }

        doubleGlow = glow + glow;
        for (thePart = partList; *thePart; thePart++) {
            (*thePart)->extraAmbient = doubleGlow;
        }
    }

    // both the server and client update the FRandSeed here, if they get out of sync
    // then this number will be different and will be noticed in CNetManager::AutoLatencyControl
    Fixed locsum = location[0] + location[1] + location[2];
    FRandSeed += locsum;
    // SDL_Log("frameNumber = %ld, FRandSeed = %10d, locsum = %8d, Actor = %s", gCurrentGame->frameNumber, (Fixed)FRandSeed, locsum, typeid(*this).name());
}
