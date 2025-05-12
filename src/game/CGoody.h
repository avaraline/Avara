/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CGoody.h
    Created: Friday, February 23, 1996, 13:53
    Modified: Tuesday, August 27, 1996, 04:15
*/

#pragma once
#include "CPlacedActors.h"

class CGoody : public CPlacedActors {
public:
    MessageRecord startMsg;
    MessageRecord stopMsg;
    MsgType outMsg;

    Fixed classicRotation, rotationSpeed;
    Fixed partRoll;

    short grenades;
    short missiles;
    short boosters;
    short lives;
    short boostTime;

    short soundId;
    short openSoundId;
    short closeSoundId;
    short frequency;
    Fixed volume;

    Boolean showText;
    Boolean enabled;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void AdaptableSettings();
    virtual void FrameAction();
    virtual bool HandlesFastFPS() { return true; }
    virtual bool UseForExtent();
};
