/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CScaledBSP.h
    Created: Thursday, March 28, 1996, 18:22
    Modified: Thursday, March 28, 1996, 18:23
*/

#pragma once
#include "CSmartPart.h"

class CScaledBSP : public CSmartPart {
public:
    virtual void IScaledBSP(Fixed scale, short resId, CAbstractActor *anActor, short aPartCode);
    virtual void Dispose();
};
