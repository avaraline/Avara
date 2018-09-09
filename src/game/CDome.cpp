/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CDome.c
    Created: Sunday, May 26, 1996, 15:13
    Modified: Sunday, May 26, 1996, 16:11
*/

#include "CDome.h"

#include "CSmartPart.h"

void CDome::BeginScript() {
    maskBits |= kSolidBit + kDoorIgnoreBit;

    CGlowActors::BeginScript();

    ProgramLongVar(iRoll, 0);
    ProgramLongVar(iPitch, 0);
    ProgramLongVar(iHasFloor, false);
}

CAbstractActor *CDome::EndScript() {
    short resId;
    short showFloor;
    Fixed startAngle, spanAngle;

    CGlowActors::EndScript();

    partCount = 1;

    showFloor = ReadLongVar(iHasFloor);
    partScale = GetDome(location, &heading, &spanAngle);
    heading -= 0x4000;

    switch (((unsigned short)spanAngle) >> 14) {
        case 0:
            resId = 464 + showFloor;
            break;
        case 1:
            resId = 460 + showFloor;
            break;
        case 2:
            resId = 462 + showFloor;
            break;
        case 3:
            resId = 462 + showFloor;
            LoadPartWithColors(1, 460 + showFloor);
            partList[1]->Reset();
            partList[1]->RotateZ(ReadFixedVar(iRoll));
            partList[1]->RotateX(ReadFixedVar(iPitch));
            partList[1]->RotateOneY(heading + 0x8000L);
            TranslatePart(partList[1], location[0], location[1], location[2]);
            partList[1]->MoveDone();
            partCount = 2;
            break;
    }

    LoadPartWithColors(0, resId);
    partList[0]->Reset();
    partList[0]->RotateZ(ReadFixedVar(iRoll));
    partList[0]->RotateX(ReadFixedVar(iPitch));
    partList[0]->RotateOneY(heading);
    TranslatePart(partList[0], location[0], location[1], location[2]);
    partList[0]->MoveDone();
    LinkPartBoxes();

    return this;
}
