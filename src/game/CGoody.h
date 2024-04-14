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
    MessageRecord startMsg = {0};
    MessageRecord stopMsg = {0};
    MsgType outMsg = 0;

    Fixed classicRotation = 0, rotationSpeed = 0;
    Fixed partRoll = 0;

    short grenades = 0;
    short missiles = 0;
    short boosters = 0;
    short lives = 0;
    short boostTime = 0;

    short soundId = 0;
    short openSoundId = 0;
    short closeSoundId = 0;
    short frequency = 0;
    Fixed volume = 0;

    Boolean showText = 0;
    Boolean enabled = 0;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void AdaptableSettings();
    virtual void FrameAction();
    virtual bool HandlesFastFPS() { return true; }
    virtual bool UseForRandomIncarnator() { return true; }
};
