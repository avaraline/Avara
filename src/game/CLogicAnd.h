/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CLogicAnd.h
    Created: Wednesday, November 22, 1995, 07:47
    Modified: Saturday, January 27, 1996, 11:59
*/

#pragma once
#include "CLogic.h"

class CLogicAnd : public CLogic {
public:
    long inCount[LOGIC_IN];

    virtual void BeginScript();
    virtual void FrameAction();
};
