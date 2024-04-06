/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CIncarnator.c
    Created: Tuesday, May 23, 1995, 17:43
    Modified: Wednesday, April 10, 1996, 14:11
*/

#include "CIncarnator.h"

IncarnatorOrder CIncarnator::order = (IncarnatorOrder)gApplication->Number(kIncarnatorOrder, kiHybrid);

void CIncarnator::SetOrder(IncarnatorOrder newValue) {
    // to be called after server sends possible new value
}

void CIncarnator::BeginScript() {
    CPlacedActors::BeginScript();

    ProgramVariable(iMask, -1);

    ProgramReference(iStartMsg, iOpenMsg);
    ProgramReference(iStopMsg, iCloseMsg);

    ProgramLongVar(iOpenMsg, 0);
    ProgramLongVar(iCloseMsg, 0);
    ProgramLongVar(iStatus, true);
}

CAbstractActor *CIncarnator::EndScript() {
    CIncarnator **append;

    CPlacedActors::EndScript();

    colorMask = ReadLongVar(iMask);
    useCount = 0;

    append = &itsGame->incarnatorList;

    while (*append) {
        append = &(*append)->nextIncarnator;
    }

    *append = this;
    nextIncarnator = NULL;

    RegisterReceiver(&startMsg, ReadLongVar(iStartMsg));
    RegisterReceiver(&stopMsg, ReadLongVar(iStopMsg));
    enabled = ReadLongVar(iStatus);

    isActive = false;

    return this;
}

void CIncarnator::FrameAction() {
    CPlacedActors::FrameAction();

    if (isActive & kHasMessage) {
        if (stopMsg.triggerCount) {
            stopMsg.triggerCount = 0;
            enabled = false;
        }

        if (startMsg.triggerCount) {
            startMsg.triggerCount = 0;
            enabled = true;
        }

        isActive &= ~kHasMessage;
    }
}
