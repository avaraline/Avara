/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CHologramActor.c
    Created: Tuesday, November 29, 1994, 04:47
    Modified: Friday, March 29, 1996, 10:44
*/

#include "CHologramActor.h"

#include "CBSPPart.h"
#include "CBSPWorld.h"

void CHologramActor::BeginScript() {
    CPlacedActors::BeginScript();

    ProgramLongVar(iShape, 600);
    ProgramVariable(iRoll, 0);
    ProgramLongVar(iIsAmbient, false);
}

CAbstractActor *CHologramActor::EndScript() {
    short resId;

    CPlacedActors::EndScript();

    resId = ReadLongVar(iShape);

    if (GetResource('BSPT', resId)) {
        CBSPWorld *theWorld;
        CBSPPart *thePart;

        theWorld = gCurrentGame->itsWorld;

        LoadPartWithColors(0, resId);
        thePart = (CBSPPart *)partList[0];
        thePart->userFlags |= ReadLongVar(iIsAmbient);
        thePart->Reset();
        thePart->RotateZ(ReadFixedVar(iRoll));
        thePart->RotateOneY(heading);
        TranslatePart(thePart, location[0], location[1], location[2]);
        thePart->MoveDone();
        partList[0] = NULL;

        theWorld->AddPart(thePart);
    }

    Dispose();
    return NULL;
}
