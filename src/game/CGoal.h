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
    Fixed motionRange;
    Fixed activeRange;
    Fixed deltas[3];
    long goalAction;
    long group;
    long goalScore;
    MsgType goalMsg;

    short goalSound;
    Fixed goalVolume;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual short GetBallSnapPoint(long theGroup,
        Fixed *ballLocation,
        Fixed *snapDest,
        Fixed *delta,
        CSmartPart **hostPart);
    virtual long ReceiveSignal(long theSignal, void *miscData);
};
