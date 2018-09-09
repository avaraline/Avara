/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CLogic.h
    Created: Tuesday, November 21, 1995, 19:49
    Modified: Sunday, January 21, 1996, 05:07
*/

#pragma once
#include "CAbstractActor.h"

#define LOGIC_IN 10
#define LOGIC_OUT 10

class CLogic : public CAbstractActor {
public:
    MessageRecord startMsg;
    MessageRecord stopMsg;
    MessageRecord in[LOGIC_IN];
    MsgType out[LOGIC_OUT];
    Boolean restart;
    Boolean enabled;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void Trigger();
    virtual void Dispose();
    virtual void FrameAction();
};
