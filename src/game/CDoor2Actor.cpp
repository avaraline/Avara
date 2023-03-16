/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CDoor2Actor.c
    Created: Tuesday, November 21, 1995, 18:42
    Modified: Thursday, November 23, 1995, 15:10
*/

#include "CDoor2Actor.h"

#include "CSmartPart.h"

void CDoor2Actor::PlaceParts() {
    Fixed r;
    Fixed t[3];
    Fixed stat = doorStatus;
    Fixed *endTwist;
    Vector deltaTwist;
    Fixed *endTrans;
    Vector deltaTrans;

    CSmartPart *theDoor;

    if (stat >= midState) {
        stat = FDivNZ(stat - FIX1, FIX1 - midState);
        endTrans = deltas;
        endTwist = twists;
        deltaTrans[0] = deltas[0] - midDeltas[0];
        deltaTrans[1] = deltas[1] - midDeltas[1];
        deltaTrans[2] = deltas[2] - midDeltas[2];

        deltaTwist[0] = twists[0] - midTwists[0];
        deltaTwist[1] = twists[1] - midTwists[1];
        deltaTwist[2] = twists[2] - midTwists[2];
    } else {
        stat = FDivNZ(stat - midState, midState);
        endTrans = midDeltas;
        endTwist = midTwists;
        deltaTrans[0] = midDeltas[0];
        deltaTrans[1] = midDeltas[1];
        deltaTrans[2] = midDeltas[2];

        deltaTwist[0] = midTwists[0];
        deltaTwist[1] = midTwists[1];
        deltaTwist[2] = midTwists[2];
    }

    theDoor = partList[0];
    theDoor->Reset();

    r = endTwist[0] + FMul(deltaTwist[0], stat);
    if (r)
        theDoor->RotateX(r);
    r = endTwist[1] + FMul(deltaTwist[1], stat);
    if (r)
        theDoor->RotateY(r);
    r = endTwist[2] + FMul(deltaTwist[2], stat);
    if (r)
        theDoor->RotateZ(r);

    t[0] = endTrans[0] + FMul(deltaTrans[0], stat);
    t[1] = endTrans[1] + FMul(deltaTrans[1], stat);
    t[2] = endTrans[2] + FMul(deltaTrans[2], stat);
    TranslatePart(theDoor, t[0], t[1], t[2]);

    theDoor->RotateOneY(heading);
    TranslatePart(theDoor, location[0], location[1], location[2]);
    theDoor->MoveDone();

    CAbstractActor::PlaceParts();
}

void CDoor2Actor::ProgramDoorVariables() {
    CDoorActor::ProgramDoorVariables();
    ProgramLongVar(iMidState, 0);

    ProgramLongVar(iMidX, 0);
    ProgramLongVar(iMidY, 0);
    ProgramLongVar(iMidZ, 0);

    ProgramLongVar(iMidPitch, 0);
    ProgramLongVar(iMidYaw, 0);
    ProgramLongVar(iMidRoll, 0);
}

void CDoor2Actor::ReadDoorVariables() {
    CDoorActor::ReadDoorVariables();

    midState = ReadFixedVar(iMidState);

    midDeltas[0] = ReadFixedVar(iMidX);
    midDeltas[1] = ReadFixedVar(iMidY);
    midDeltas[2] = ReadFixedVar(iMidZ);

    midTwists[0] = ReadFixedVar(iMidPitch);
    midTwists[1] = ReadFixedVar(iMidYaw);
    midTwists[2] = ReadFixedVar(iMidRoll);
}
