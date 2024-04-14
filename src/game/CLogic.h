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
    MessageRecord startMsg = {0};
    MessageRecord stopMsg = {0};
    MessageRecord in[LOGIC_IN] = {0};
    MsgType out[LOGIC_OUT] = {0};
    Boolean restart = 0;
    Boolean enabled = 0;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void Trigger();
    virtual ~CLogic();
    virtual void FrameAction();
};
