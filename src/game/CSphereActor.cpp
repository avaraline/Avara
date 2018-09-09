/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CSphereActor.c
    Created: Wednesday, November 23, 1994, 08:48
    Modified: Monday, May 6, 1996, 16:31
*/

#include "CSphereActor.h"

#include "CSmartPart.h"

void CSphereActor::IAbstractActor() {
    CGlowActors::IAbstractActor();

    maskBits |= kTargetBit + kSolidBit;
}
CAbstractActor *CSphereActor::EndScript() {
    CGlowActors::EndScript();

    partCount = 1;

    LoadPart(0, kSphereBSP);

    TranslatePart(partList[0], location[0], location[1], location[2]);
    partList[0]->MoveDone();
    LinkPartBoxes();

    sliverCounts[kMediumSliver] = 24;

    return this;
}

void CSphereActor::WasHit(RayHitRecord *theHit, Fixed hitEnergy) {
    //	itsGame->Score(theHit->team, theHit->playerId, 100, true, teamColor, -1);

    Blast();
    Dispose();
}
