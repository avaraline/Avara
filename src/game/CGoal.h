/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CGoal.h
    Created: Sunday, August 25, 1996, 08:20
    Modified: Tuesday, September 3, 1996, 21:44
*/

#pragma once
#include "CGlowActors.h"

class CGoal : public CGlowActors {
public:
    Fixed motionRange = 0;
    Fixed activeRange = 0;
    Fixed deltas[3] = {0};
    long goalAction = 0;
    long group = 0;
    long goalScore = 0;
    MsgType goalMsg = 0;

    short goalSound = 0;
    Fixed goalVolume = 0;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual short GetBallSnapPoint(long theGroup,
        Fixed *ballLocation,
        Fixed *snapDest,
        Fixed *delta,
        CSmartPart **hostPart);
    virtual long ReceiveSignal(long theSignal, void *miscData);
};
