/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CPlacedActors.c
    Created: Tuesday, November 22, 1994, 05:46
    Modified: Sunday, January 21, 1996, 05:11
*/

#include "CPlacedActors.h"

#include "CSmartPart.h"

CPlacedActors::CPlacedActors() {
    location[0] = 0;
    location[1] = 0;
    location[2] = 0;
}

void CPlacedActors::BeginScript() {
    CAbstractActor::BeginScript();

    ProgramFixedVar(iHeight, location[1]);
}

CAbstractActor *CPlacedActors::EndScript() {
    CAbstractActor::EndScript();

    heading = GetLastArcDirection();
    GetLastArcLocation(location);
    location[1] = ReadFixedVar(iHeight) + ReadFixedVar(iBaseHeight);

    return this;
}
