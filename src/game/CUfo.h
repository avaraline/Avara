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
    Fixed happiness;
    Fixed attack;
    Fixed defense;
    Fixed currentBias;
    Fixed visionScore;
    Fixed hideScore;

    Fixed homeSick;
    Fixed homeRange;

    //	These variables control the range of motion.
    Fixed motionRange;
    Fixed pitchRange;
    Fixed verticalMin;
    Fixed verticalMax;

    //	These variables control the actual motion
    Vector homeBase;
    Vector destination;

    Vector course;
    Fixed acceleration;

    Fixed turnRate;
    Fixed goodHeading;
    Fixed goodPitch;
    Fixed roll;
    short pitch;

    long moveClock; //	Look for new position when this reaches 0
    long checkPeriod; //	Period of moveClock

    MessageRecord startMsg;
    MessageRecord stopMsg;
    MessageRecord homeMsg;
    Boolean status;
    Boolean isMoving;

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
