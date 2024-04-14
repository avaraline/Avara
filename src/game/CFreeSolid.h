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
    Fixed hitPower = 0; //	Collisions cause damage relative to shotPower
    Fixed classicGravity = 0, customGravity = 0; //	Private gravity/frame
    Fixed classicAcceleration = 0, acceleration = 0; //	Private slowdown/frame while moving

    MessageRecord startMsg = {0}; //	Message to enable object (otherwise motionless)
    MessageRecord stopMsg = {0}; //	Message to disable object (to stop it)
    short status = 0; //	Status is either active or stopped

    virtual void BeginScript(); //	Write default param values
    virtual CAbstractActor *EndScript(); //	Read back values changed by user script
    virtual void AdaptableSettings(); // Read settings that can change with frame rate
    virtual void PlaceParts(); //	Move our single BSP part into place/orientation
    virtual void FrameAction(); //	Action for each frame of simulation
    virtual bool HandlesFastFPS() { return true; }
};
