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
    MessageRecord startMsg;
    MessageRecord stopMsg;

    MsgType enterMsg;
    MsgType exitMsg;

    MaskType watchBits;
    short watchTeams;
    Fixed spinSpeed;
    Fixed force[3];
    Boolean isWorking;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void FrameAction();
};
