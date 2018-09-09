/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CLogicAnd.c
    Created: Wednesday, November 22, 1995, 08:00
    Modified: Saturday, January 27, 1996, 11:59
*/

#include "CLogicAnd.h"

void CLogicAnd::BeginScript() {
    short i;

    CLogic::BeginScript();

    for (i = 0; i < LOGIC_IN; i++) {
        inCount[i] = 0;
    }
}

void CLogicAnd::FrameAction() {
    short i;

    CLogic::FrameAction();

    if (isActive & kHasMessage) {
        short minTriggers = 0;
        Boolean noTrigger = true;

        isActive &= ~kHasMessage;

        if (enabled) {
            for (i = 0; i < LOGIC_IN; i++) {
                if (in[i].messageId) {
                    inCount[i] += in[i].triggerCount;
                    in[i].triggerCount = 0;
                }
            }

            for (i = 0; i < LOGIC_IN; i++) {
                if (in[i].messageId && (noTrigger || (inCount[i] < minTriggers))) {
                    noTrigger = false;
                    minTriggers = inCount[i];
                }
            }

            if (minTriggers) {
                for (i = 0; i < LOGIC_IN; i++) {
                    if (in[i].messageId)
                        inCount[i] -= minTriggers;
                }

                while (minTriggers--) {
                    Trigger();

                    if (!restart) {
                        Dispose();
                        return;
                    }
                }
            }
        } else {
            for (i = 0; i < LOGIC_IN; i++) {
                in[i].triggerCount = 0;
            }
        }
    }
}
