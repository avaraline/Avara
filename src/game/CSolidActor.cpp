/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CSolidActor.c
    Created: Saturday, December 3, 1994, 11:50
    Modified: Monday, February 26, 1996, 16:00
*/

#include "AssetManager.h"
#include "CSolidActor.h"

#include "CSmartPart.h"

void CSolidActor::BeginScript() {
    maskBits |= kSolidBit + kDoorIgnoreBit;

    CGlowActors::BeginScript();

    ProgramLongVar(iShape, 411);
    ProgramLongVar(iRoll, 0);
    ProgramLongVar(iPitch, 0);
}

CAbstractActor *CSolidActor::EndScript() {
    short resId;

    CGlowActors::EndScript();

    resId = ReadLongVar(iShape);

    auto bsp = AssetManager::GetBsp(resId);
    if (bsp) {
        LoadPartWithMaterials(0, resId);
        partList[0]->Reset();
        partList[0]->RotateZ(ReadFixedVar(iRoll));
        partList[0]->RotateX(ReadFixedVar(iPitch));
        partList[0]->RotateOneY(heading);
        TranslatePart(partList[0], location[0], location[1], location[2]);
        partList[0]->MoveDone();
        LinkPartBoxes();

        partCount = 1;
    }

    return this;
}
