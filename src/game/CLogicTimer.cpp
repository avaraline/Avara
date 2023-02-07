/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CLogicTimer.c
    Created: Wednesday, November 22, 1995, 08:13
    Modified: Saturday, January 27, 1996, 12:04
*/

#include "CLogicTimer.h"

void CLogicTimer::BeginScript() {
    CLogic::BeginScript();
    ProgramLongVar(iTimer, 16);
}

CAbstractActor *CLogicTimer::EndScript() {
    if (CLogic::EndScript()) {
        theDelay = FrameNumber(ReadLongVar(iTimer));

        if (theDelay)
            return this;
    }

    Dispose();
    return NULL;
}

void CLogicTimer::AdaptableSettings() {
    if (itsGame->frameNumber == 0) {
        if (theDelay < 0) {
            theDelay = -theDelay;
            sleepTimer = FpsFramesPerClassic(theDelay + 1);
        }
    }
}

void CLogicTimer::FrameAction() {
    short i;

    CLogic::FrameAction();

    if (isActive & kHasMessage) {
        isActive &= ~kHasMessage;

        for (i = 0; i < LOGIC_IN; i++) {
            if (in[i].triggerCount) {
                if (enabled) {
                    sleepTimer = theDelay;
                    whenFrame = itsGame->FramesFromNow(sleepTimer);
                }
                in[i].triggerCount = 0;
            }
        }
    } else if (whenFrame == itsGame->frameNumber) { //	Triggered!

        Trigger();

        if (!restart) {
            Dispose();
            return;
        }
    }
}
