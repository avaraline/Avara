/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CAbstractYon.c
    Created: Thursday, August 22, 1996, 02:51
    Modified: Thursday, August 22, 1996, 09:29
*/

#include "CAbstractYon.h"

void CAbstractYon::BeginScript() {
    partYon = FIX(-8);

    CAbstractActor::BeginScript();

    ProgramFixedVar(iHeight, 0);
    ProgramMessage(iStartMsg, iStartMsg);
    ProgramLongVar(iStopMsg, 0);
    ProgramLongVar(iStatus, false);
}

CAbstractActor *CAbstractYon::EndScript() {
    if (CAbstractActor::EndScript()) {
        RegisterReceiver(&startMsg, ReadLongVar(iStartMsg));
        RegisterReceiver(&stopMsg, ReadLongVar(iStopMsg));
        status = ReadLongVar(iStatus);

        return this;

    } else {
        return NULL;
    }
}

void CAbstractYon::SetYon() {
    partYon = ReadFixedVar(iYon);
    if (partYon <= 0) {
        partYon = sphere[3] + sphere[3] - partYon;
    }

    nextYon = gCurrentGame->yonList;
    gCurrentGame->yonList = this;
}

void CAbstractYon::FrameAction() {
    CAbstractActor::FrameAction();

    if (stopMsg.triggerCount > startMsg.triggerCount) {
        status = false;
    } else if (stopMsg.triggerCount < startMsg.triggerCount) {
        status = true;
    }

    stopMsg.triggerCount = 0;
    startMsg.triggerCount = 0;

    isActive &= ~kHasMessage;
}

Fixed CAbstractYon::AdjustYon(Fixed *location, Fixed curYon) {
    if (status && curYon > partYon && location[0] >= minBounds[0] && location[0] < maxBounds[0] &&
        location[1] >= minBounds[1] && location[1] < maxBounds[1] && location[2] >= minBounds[2] &&
        location[2] < maxBounds[2] &&
        sphere[3] >= FDistanceEstimate(location[0] - sphere[0], location[1] - sphere[1], location[2] - sphere[2])) {
        curYon = partYon;
    }

    if (!nextYon)
        return curYon;
    else
        return nextYon->AdjustYon(location, curYon);
}
