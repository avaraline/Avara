/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CLogicDelay.h
    Created: Wednesday, November 22, 1995, 07:47
    Modified: Wednesday, November 22, 1995, 08:58
*/

#pragma once
#include "CLogic.h"

#define DELAY_PIPELINE 32

class CLogicDelay : public CLogic {
public:
    long theDelay;
    long scheduledFrame[DELAY_PIPELINE];

    virtual void FrameAction();
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
};
