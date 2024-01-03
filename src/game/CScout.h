/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CScout.h
    Created: Wednesday, March 15, 1995, 07:14
    Modified: Friday, March 1, 1996, 17:27
*/

#pragma once
#include "CRealMovers.h"

class CAbstractPlayer;

enum {
    kScoutNullCommand,
    kScoutFollow,
    kScoutLead,
    kScoutLeft,
    kScoutRight,
    kScoutUp,
    kScoutDown,
    kScoutStop,
    kScoutReturn,
    kScoutIdle,
    kScoutInactive
};

class CScout : public CRealMovers {
public:
    Vector targetPosition;
    CAbstractPlayer *itsPlayer;
    short action;
    Boolean nextRotateFlag;

    virtual void IScout(CAbstractPlayer *thePlayer, short theTeam, ARGBColor longTeamColor);
    virtual void PlaceParts();
    virtual void FrameAction();
    virtual void ControlViewPoint();
    virtual void ToggleState(short command);

    virtual Fixed MoveToTarget();
    virtual bool HandlesFastFPS() { return true; }
};
