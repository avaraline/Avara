/*
    Copyright ©1996-1997, Juri Munkki
    All rights reserved.

    File: CFreeSolid.h
    Created: Friday, August 9, 1996, 16:51
    Modified: Monday, March 10, 1997, 16:16
*/

#pragma once
#include "CRealMovers.h"

class CFreeSolid : public CRealMovers {
public:
    Fixed hitPower; //	Collisions cause damage relative to shotPower
    Fixed customGravity; //	Private gravity/frame
    Fixed acceleration; //	Private slowdown/frame while moving

    MessageRecord startMsg; //	Message to enable object (otherwise motionless)
    MessageRecord stopMsg; //	Message to disable object (to stop it)
    short status; //	Status is either active or stopped

    virtual void BeginScript(); //	Write default param values
    virtual CAbstractActor *EndScript(); //	Read back values changed by user script
    virtual void PlaceParts(); //	Move our single BSP part into place/orientation
    virtual void FrameAction(); //	Action for each frame of simulation
};
