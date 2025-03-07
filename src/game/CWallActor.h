/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CWallActor.h
    Created: Sunday, November 20, 1994, 19:37
    Modified: Thursday, August 22, 1996, 08:27
*/

#pragma once
#include "CAbstractActor.h"
#include "CSmartBox.h"

class CWallActor : public CAbstractActor {
public:
    virtual void MakeWallFromRect(Rect *theRect, Fixed height, short decimateWalls, Boolean isOrigWall);
    virtual void MakeWallFromDims(Vector dims, Fixed x, Fixed y, Fixed z);
    virtual bool HandlesFastFPS() { return true; }
};
