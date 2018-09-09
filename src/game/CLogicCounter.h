/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CLogicCounter.h
    Created: Tuesday, November 21, 1995, 20:07
    Modified: Tuesday, November 21, 1995, 20:13
*/

#pragma once
#include "CLogic.h"

class CLogicCounter : public CLogic {
public:
    long theCount;
    long trigCount;

    virtual void FrameAction();
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
};
