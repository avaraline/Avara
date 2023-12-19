/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CPlayerMissile.c
    Created: Saturday, November 26, 1994, 11:18
    Modified: Monday, January 29, 1996, 14:43
*/
#define ENABLE_FPS_DEBUG  // uncomment if you want to see FPS_DEBUG output for this file

#include "CPlayerMissile.h"

#define kPlayerMissileBSP 203
#define MISSILESPIN FpsCoefficient2(FIX(17))

CPlayerMissile::CPlayerMissile(CDepot *theDepot) : CAbstractMissile(theDepot) {
    speed = PLAYERMISSILESPEED;
    maxFrameCount = FpsFramesPerClassic(PLAYERMISSILELIFE);

    partCount = 1;
    LoadPart(0, kPlayerMissileBSP);
}

void CPlayerMissile::MoveForward() {
    partList[0]->isTransparent = false;
    partList[0]->extraAmbient = energy;
    partList[0]->Reset();
    InitialRotatePartZ(partList[0], FDegToOne(rotation));
    partList[0]->ApplyMatrix(&itsMatrix);
    itsMatrix[3][0] += FpsCoefficient2(deltaMove[0]);
    itsMatrix[3][1] += FpsCoefficient2(deltaMove[1]);
    itsMatrix[3][2] += FpsCoefficient2(deltaMove[2]);
    partList[0]->MoveDone();

    rotation += spin;
}

void CPlayerMissile::SetSpin(Boolean spinDirection) {
    rotation = 0;
    if (spinDirection) {
        spin = MISSILESPIN;
    } else {
        spin = -MISSILESPIN;
    }
}
