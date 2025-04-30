/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CTriPyramidActor.c
    Created: Saturday, December 3, 1994, 14:46
    Modified: Monday, February 26, 1996, 16:00
*/

#include "CTriPyramidActor.h"

#include "CSmartPart.h"

CAbstractActor *CTriPyramidActor::EndScript() {
    CPlacedActors::EndScript();

    maskBits |= kSolidBit + kDoorIgnoreBit;
    partCount = 1;

    LoadPartWithMaterials(0, kTriPyramidBSP);
    partList[0]->RotateZ(FIX(45));
    partList[0]->RotateX(FIX3(35264));
    partList[0]->RotateOneY(heading);
    TranslatePart(partList[0], location[0], location[1] - FIX3(1414), location[2]);
    partList[0]->MoveDone();
    LinkPartBoxes();

    return this;
}
