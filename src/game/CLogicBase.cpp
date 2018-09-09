/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CLogicBase.c
    Created: Tuesday, July 16, 1996, 00:47
    Modified: Thursday, September 12, 1996, 00:22
*/

#include "CLogicBase.h"

void CLogicBase::BeginScript() {
    CLogic::BeginScript();

    ProgramFixedVar(iHeight, 0);
    ProgramLongVar(iVerticalRangeMin, -1);
    ProgramLongVar(iVerticalRangeMax, -1);
}

CAbstractActor *CLogicBase::EndScript() {
    if (CLogic::EndScript() == this) {
        GetLastArcLocation(location);
        location[1] = ReadFixedVar(iHeight) + ReadFixedVar(iBaseHeight);
        location[3] = ReadFixedVar(iVerticalRangeMax);
        location[4] = ReadFixedVar(iVerticalRangeMin);

        return this;
    } else
        return NULL;
}

void CLogicBase::FrameAction() {
    short i;
    Boolean triggered = false;

    CLogic::FrameAction();

    if (isActive & kHasMessage) {
        isActive &= ~kHasMessage;

        for (i = 0; i < LOGIC_IN; i++) {
            if (in[i].triggerCount) {
                triggered = true;
            }

            in[i].triggerCount = 0;
        }

        itsGame->baseLocation = location;
        if (enabled && triggered) {
            for (i = 0; i < LOGIC_OUT; i++) {
                itsGame->FlagImmediateMessage(out[i]);
            }

            if (!restart) {
                Dispose();
                return;
            }
        }
    }
}
