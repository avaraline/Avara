/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CLogicBase.h
    Created: Tuesday, July 16, 1996, 00:55
    Modified: Thursday, September 12, 1996, 00:22
*/

#pragma once
#include "CLogic.h"

class CLogicBase : public CLogic {
public:
    Fixed location[5];

    virtual void FrameAction();
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
};
