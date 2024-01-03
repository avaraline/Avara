/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CShuriken.c
    Created: Tuesday, December 5, 1995, 05:28
    Modified: Sunday, December 10, 1995, 13:08
*/

#include "CShuriken.h"

#define kShurikenBSP 809
#define SHURIKENSPIN FpsCoefficient2(2621)
#define SHURIKENLIFE FpsFramesPerClassic(100)

void CShuriken::IAbstractMissile(CDepot *theDepot) {
    CAbstractMissile::IAbstractMissile(theDepot);

    speed = SHURIKENSPEED;
    maxFrameCount = SHURIKENLIFE;

    partCount = 1;
    LoadPart(0, kShurikenBSP);
}

void CShuriken::MoveForward() {
    partList[0]->isTransparent = false;
    partList[0]->extraAmbient = energy;
    partList[0]->Reset();
    InitialRotatePartX(partList[0], FOneSin(rotation << 1) >> 5);
    partList[0]->RotateOneY(rotation);
    TranslatePart(partList[0], itsMatrix[3][0], itsMatrix[3][1], itsMatrix[3][2]);
    partList[0]->MoveDone();

    itsMatrix[3][0] += FpsCoefficient2(deltaMove[0]);
    itsMatrix[3][1] += FpsCoefficient2(deltaMove[1]);
    itsMatrix[3][2] += FpsCoefficient2(deltaMove[2]);

    rotation += SHURIKENSPIN;
}
