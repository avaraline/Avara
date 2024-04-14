/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CTeleporter.h
    Created: Monday, July 3, 1995, 01:17
    Modified: Tuesday, August 27, 1996, 00:57
*/

#pragma once
#include "CPlacedActors.h"
#include "FastMat.h"

#define kTeleportBSP 230

class CAbstractPlayer;

class CTeleporter : public CPlacedActors {
public:
    unsigned long useCount = 0;
    short goTimer = 0;
    short options = 0;

    short noPullTimer = 0;
    short pullCounter = 0;

    long transportGroup = 0;
    long destGroup = 0;
    short soundId = 0;
    short watchTeams = 0;
    Fixed volume = 0;
    Fixed rotationSpeed = 0;
    Fixed activeRange = 0;
    Fixed deadRange = 0;
    long winScore = 0;

    MsgType didSendMsg = 0;
    MsgType didReceiveMsg = 0;

    MessageRecord startMsg = {0};
    MessageRecord stopMsg = {0};
    Boolean enabled = 0;

    Vector attraction = {0};

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void FrameAction();
    virtual void TeleportPlayer(CAbstractPlayer *thePlayer);
    virtual Boolean ReceivePlayer(CAbstractPlayer *thePlayer);
    virtual bool HandlesFastFPS() { return true; }
};
