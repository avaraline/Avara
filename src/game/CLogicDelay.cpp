/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CLogicDelay.c
    Created: Wednesday, November 22, 1995, 08:57
    Modified: Saturday, January 27, 1996, 12:01
*/

#include "CLogicDelay.h"

void CLogicDelay::BeginScript() {
    CLogic::BeginScript();
    ProgramLongVar(iTimer, 16);
}

CAbstractActor *CLogicDelay::EndScript() {
    short i;

    for (i = 0; i < DELAY_PIPELINE; i++) {
        scheduledFrame[i] = 0;
    }

    if (CLogic::EndScript()) {
        theDelay = ReadLongVar(iTimer);
        if (theDelay < 0) {
            theDelay = -theDelay;
            sleepTimer = theDelay + 1;
        }

        if (theDelay)
            return this;
    }

    Dispose();
    return NULL;
}

void CLogicDelay::FrameAction() {
    short i;
    short theCount;
    long thisFrame = itsGame->frameNumber;
    long closestAlarm;
    Boolean foundAlarm;

    CLogic::FrameAction();

    for (i = 0; i < DELAY_PIPELINE; i++) {
        if (scheduledFrame[i] == thisFrame) {
            scheduledFrame[i] = 0;
            sleepTimer = 0;
            Trigger();
            if (!restart) {
                Dispose();
                return;
            }
        }
    }

    if ((isActive & kHasMessage) || (sleepTimer == 0)) {
        isActive &= ~kHasMessage;
        sleepTimer--;

        theCount = 0;
        for (i = 0; i < LOGIC_IN; i++) {
            theCount += in[i].triggerCount;
            in[i].triggerCount = 0;
        }

        if (!enabled)
            theCount = 0;

        while (theCount--) {
            for (i = 0; i < DELAY_PIPELINE; i++) {
                if (scheduledFrame[i] == 0) {
                    scheduledFrame[i] = itsGame->frameNumber + theDelay;
                    break;
                }
            }
        }

        closestAlarm = thisFrame + theDelay + theDelay;
        foundAlarm = false;

        for (i = 0; i < DELAY_PIPELINE; i++) {
            if (scheduledFrame[i] && scheduledFrame[i] < closestAlarm) {
                foundAlarm = true;
                closestAlarm = scheduledFrame[i];
            }
        }

        if (foundAlarm)
            sleepTimer = closestAlarm - thisFrame;
        else
            sleepTimer = -1;
    }
}
