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
    bool isMorphable;

    virtual void Scale(Fixed scale);
    virtual void ScaleXYZ(Fixed scaleX, Fixed scaleY, Fixed scaleZ);
    virtual void Reset();

    virtual void Dispose();
    static CScaledBSP *Create(Fixed scale, short resId, CAbstractActor *anActor, short aPartCode);
private:
    CScaledBSP() {};
    virtual void IScaledBSP(Fixed scale, short resId, CAbstractActor *anActor, short aPartCode);

};
