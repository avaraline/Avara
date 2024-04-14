/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CAbstractYon.h
    Created: Thursday, August 22, 1996, 02:53
    Modified: Thursday, August 22, 1996, 05:00
*/

#pragma once
#include "CAbstractActor.h"

class CAbstractYon : public CAbstractActor {
public:
    Vector minBounds = {0};
    Vector maxBounds = {0};
    Vector sphere = {0};
    class CAbstractYon *nextYon = 0;

    MessageRecord startMsg = {0};
    MessageRecord stopMsg = {0};
    short status = 0;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void SetYon();
    virtual void FrameAction();
    virtual Fixed AdjustYon(Fixed *location, Fixed curYon);
};
