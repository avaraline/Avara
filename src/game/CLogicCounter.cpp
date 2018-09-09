/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CLogicCounter.c
    Created: Tuesday, November 21, 1995, 20:09
    Modified: Saturday, January 27, 1996, 12:01
*/

#include "CLogicCounter.h"

void CLogicCounter::BeginScript() {
    CLogic::BeginScript();
    ProgramLongVar(iCount, 1);
}

CAbstractActor *CLogicCounter::EndScript() {
    trigCount = ReadLongVar(iCount);
    theCount = 0;

    return CLogic::EndScript();
}

void CLogicCounter::FrameAction() {
    short i;

    CLogic::FrameAction();

    if (isActive & kHasMessage) {
        isActive &= ~kHasMessage;

        for (i = 0; i < LOGIC_IN; i++) {
            if (enabled)
                theCount += in[i].triggerCount;
            in[i].triggerCount = 0;
        }

        while (theCount >= trigCount) {
            theCount -= trigCount;
            Trigger();

            if (!restart) {
                Dispose();
                return;
            }
        }
    }
}
