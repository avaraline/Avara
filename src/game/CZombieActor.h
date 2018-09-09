/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CZombieActor.h
    Created: Tuesday, October 31, 1995, 14:56
    Modified: Saturday, December 2, 1995, 05:21
*/

#pragma once
#include "CGlowActors.h"

#define SMARTMEMSIZE 16

class CZombieActor : public CGlowActors {
public:
    Vector oldLoc;
    Fixed oldHeading;
    Fixed goodHeading;
    Fixed watchHeading;
    short watchFlip;

    Vector speed;
    Vector accel;

    Fixed bigRadius;
    Fixed cornerAngle;

    short checkCounter;
    Fixed happiness; //	is a warm gun...or so they say.
    Fixed collisionPain;

    Vector smartMem[SMARTMEMSIZE];
    short nextSmart;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void FrameAction();
    virtual void PlaceParts();
    virtual Fixed ScanDirection(Fixed direction, RayHitRecord *target);

    virtual void MoveForward();
    virtual void SavePosition();
    virtual void UndoMove();

    virtual void RememberExperience(Fixed *place, Fixed weight);
    virtual Fixed JudgePlace(Fixed *place);

    virtual void WasHit(RayHitRecord *theHit, Fixed hitEnergy);
};
