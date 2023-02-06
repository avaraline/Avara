/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CLogicTimer.h
    Created: Wednesday, November 22, 1995, 07:47
    Modified: Sunday, January 21, 1996, 05:06
*/

#pragma once
#include "CLogic.h"

class CLogicTimer : public CLogic {
public:
    FrameNumber theDelay;
    FrameNumber whenFrame;

    virtual void FrameAction();
    virtual void BeginScript();
    virtual void AdaptableSettings();
    virtual CAbstractActor *EndScript();
};
