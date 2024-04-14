/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CUfo.h
    Created: Monday, December 4, 1995, 03:02
    Modified: Saturday, August 10, 1996, 21:02
*/

#pragma once
#include "CRealShooters.h"

class CUfo : public CRealShooters {
public:
    //	These variables determine the "mood" of the ufo.
    //	The current bias determine how likely it is to go
    //	into a attacking position.
    Fixed happiness = 0;
    Fixed attack = 0;
    Fixed defense = 0;
    Fixed currentBias = 0;
    Fixed visionScore = 0;
    Fixed hideScore = 0;

    Fixed homeSick = 0;
    Fixed homeRange = 0;

    //	These variables control the range of motion.
    Fixed motionRange = 0;
    Fixed pitchRange = 0;
    Fixed verticalMin = 0;
    Fixed verticalMax = 0;

    //	These variables control the actual motion
    Vector homeBase = {0};
    Vector destination = {0};

    Vector course = {0};
    Fixed acceleration = 0;

    Fixed turnRate = 0;
    Fixed goodHeading = 0;
    Fixed goodPitch = 0;
    Fixed roll = 0;
    short pitch = 0;

    long moveClock = 0; //	Look for new position when this reaches 0
    long checkPeriod = 0; //	Period of moveClock

    MessageRecord startMsg = {0};
    MessageRecord stopMsg = {0};
    MessageRecord homeMsg = {0};
    Boolean status = 0;
    Boolean isMoving = 0;

    virtual void PlaceParts();
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();

    virtual Fixed EvaluatePosition(Fixed *position, Boolean doAttack);
    virtual void Navigate();
    virtual void ReTarget();
    virtual void WasHit(RayHitRecord *theHit, Fixed hitEnergy);
    virtual void FrameAction();
    virtual void ImmediateMessage();
};
