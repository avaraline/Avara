/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CLogic.c
    Created: Tuesday, November 21, 1995, 19:51
    Modified: Wednesday, April 10, 1996, 14:12
*/

#include "CLogic.h"

void CLogic::BeginScript() {
    short i;

    CAbstractActor::BeginScript();

    ProgramLongVar(iOutVar, 0);
    ProgramReference(iFirstOut, iOutVar);
    for (i = 1; i < LOGIC_OUT; i++) {
        ProgramLongVar(iFirstOut + i, 0);
    }

    ProgramLongVar(iInVar, 0);
    ProgramReference(iFirstIn, iInVar);
    for (i = 1; i < LOGIC_IN; i++) {
        ProgramLongVar(iFirstIn + i, 0);
    }

    ProgramMessage(iStartMsg, iStartMsg);
    ProgramLongVar(iStopMsg, 0);
    ProgramLongVar(iStatus, true);

    ProgramLongVar(iRestartFlag, true);
}

CAbstractActor *CLogic::EndScript() {
    short i;

    RegisterReceiver(&startMsg, ReadLongVar(iStartMsg));
    RegisterReceiver(&stopMsg, ReadLongVar(iStopMsg));
    enabled = ReadLongVar(iStatus);

    for (i = 0; i < LOGIC_OUT; i++) {
        out[i] = ReadLongVar(iFirstOut + i);
    }

    for (i = 0; i < LOGIC_IN; i++) {
        MsgType msg;

        msg = ReadLongVar(iFirstIn + i);
        RegisterReceiver(&in[i], msg);
    }

    restart = ReadLongVar(iRestartFlag);
    isActive = false;

    return CAbstractActor::EndScript();
}

void CLogic::Trigger() {
    short i;

    if (enabled) {
        for (i = 0; i < LOGIC_OUT; i++) {
            itsGame->FlagMessage(out[i]);
        }
    }
}

void CLogic::Dispose() {
    short i;

    itsGame->RemoveReceiver(&startMsg);
    itsGame->RemoveReceiver(&stopMsg);

    for (i = 0; i < LOGIC_IN; i++) {
        itsGame->RemoveReceiver(&in[i]);
    }

    CAbstractActor::Dispose();
}

void CLogic::FrameAction() {
    if (stopMsg.triggerCount) {
        stopMsg.triggerCount = 0;
        enabled = true;
    }

    if (startMsg.triggerCount) {
        startMsg.triggerCount = 0;
        enabled = true;
    }
}
