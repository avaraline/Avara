/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CForceField.h
    Created: Saturday, January 20, 1996, 04:35
    Modified: Tuesday, June 11, 1996, 14:08
*/

#pragma once
#include "CPlacedActors.h"

class CForceField : public CPlacedActors {
public:
    MessageRecord startMsg = {0};
    MessageRecord stopMsg = {0};

    MsgType enterMsg = 0;
    MsgType exitMsg = 0;

    MaskType watchBits = 0;
    short watchTeams = 0;
    Fixed spinSpeed = 0;
    Fixed force[3] = {0};
    Boolean isWorking = 0;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void FrameAction();
};
