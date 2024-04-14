/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CSliverPart.h
    Created: Monday, November 28, 1994, 04:07
    Modified: Monday, August 12, 1996, 15:57
*/

#pragma once
#include "ColorManager.h"
#include "CSmartPart.h"

// class CSliverPart;

class CSliverPart final : public CSmartPart {
public:
    CSliverPart *nextSliver = 0;
    Vector speed = {0};
    short lifeCount = 0;
    ARGBColor borrowedColors = 0;
    ARGBColor *fakeMaster = 0;
    Fixed friction = 0, gravity = 0;
    Fixed fpsScale = 0, fpsFriction = 0, fpsGravity = 0;
    CSliverPart(short partNum);
    virtual void Activate(Fixed *origin,
        Fixed *direction,
        Fixed scale,
        Fixed speedFactor,
        short spread,
        short age,
        CBSPPart *fromObject);
    virtual Boolean SliverAction();
#if 0
    virtual	void		GenerateColorLookupTable();
#endif
private:
    void Create() {}
};
