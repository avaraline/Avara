/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CDoorActor.h
    Created: Sunday, December 4, 1994, 09:59
    Modified: Saturday, February 17, 1996, 02:47
*/

#pragma once
#include "CGlowActors.h"

#define kDefaultDoorBSP 550
#define kDoorClosed 0
#define kDoorOpen 65536

#define kDoorSpeed FIX3(20)

enum { kDoorStopped, kDoorOpening, kDoorClosing };

class CDoorActor : public CGlowActors {
public:
    Fixed doorStatus = 0;
    Fixed classicOpenSpeed = 0, openSpeed = 0;
    Fixed classicCloseSpeed = 0, closeSpeed = 0;

    Fixed deltas[3] = {0};
    Fixed twists[3] = {0};

    MessageRecord openActivator = {0};
    MessageRecord closeActivator = {0};

    MsgType didOpen = 0;
    MsgType didClose = 0;

    long closeCounter = 0;
    long openCounter = 0;

    FrameNumber classicOpenDelay = 0, openDelay = 0;
    FrameNumber classicCloseDelay = 0, closeDelay = 0;
    FrameNumber classicGuardDelay = 0, collisionGuardTime = 0;

    short action = 0;

    short openSoundId = 0;
    short closeSoundId = 0;
    short stopSoundId = 0;
    Fixed doorSoundVolume = 0;

    Fixed hitPower = 0;

    Fixed lastMovement[3] = {0};

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void AdaptableSettings();
    virtual ~CDoorActor();
    
    virtual CSmartPart *CollisionTest();
    virtual void TouchDamage();
    virtual void PlaceParts();
    virtual void FrameAction();

    virtual void DoorSound();

    virtual void ProgramDoorVariables();
    virtual void ReadDoorVariables();

    virtual void StandingOn(CAbstractActor *who, Fixed *where, Boolean firstLeg);

    virtual bool HandlesFastFPS() { return true; }
};
