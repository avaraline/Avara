/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: CMissile.c
    Created: Sunday, December 18, 1994, 03:29
    Modified: Wednesday, March 8, 1995, 06:46
*/

#include "CMissile.h"

CMissile::CMissile(CDepot *theDepot) : CAbstractMissile(theDepot) {
    maxFrameCount = FpsFramesPerClassic(50);
    partCount = 1;
    LoadPart(0, kMissileBSP);
}
