/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: CMarkerActor.h
    Created: Tuesday, November 22, 1994, 02:14
    Modified: Wednesday, March 15, 1995, 07:13
*/

#pragma once
#include "CGlowActors.h"
#include "FastMat.h"

#define kMarkerBSP 202

class CMarkerActor : public CGlowActors {
public:
    short markerColor;

    virtual void IAbstractActor();
    virtual CAbstractActor *EndScript();
    virtual void FrameAction();
};
