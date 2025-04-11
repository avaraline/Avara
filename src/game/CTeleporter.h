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
    unsigned long useCount;
    short goTimer;
    short options;

    short noPullTimer;
    short pullCounter;

    long transportGroup;
    long destGroup;
    short soundId;
    short watchTeams;
    Fixed volume;
    Fixed rotationSpeed;
    Fixed activeRange;
    Fixed deadRange;
    long winScore;

    MsgType didSendMsg;
    MsgType didReceiveMsg;

    MessageRecord startMsg;
    MessageRecord stopMsg;
    Boolean enabled;

    Vector attraction;

    virtual void BeginScript() override;
    virtual CAbstractActor *EndScript() override;
    virtual void FrameAction() override;
    virtual void TeleportPlayer(CAbstractPlayer *thePlayer);
    virtual Boolean ReceivePlayer(CAbstractPlayer *thePlayer);
    virtual bool HandlesFastFPS() override { return true; }
};
