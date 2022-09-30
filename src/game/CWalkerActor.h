/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CWalkerActor.h
    Created: Monday, February 6, 1995, 11:59
    Modified: Saturday, September 7, 1996, 19:40
*/

#pragma once
#include "CAbstractPlayer.h"

#define kHeadBSP 210
#define kLegHighBSP 211
#define kLegLowBSP 212
#define kHullBSPBase 215

typedef struct {
    Fixed x;
    Fixed y;
    Fixed highAngle;
    Fixed lowAngle;
    long touchIdent;
    Fixed where[3];
} LegInfo;

class CWalkerActor : public CAbstractPlayer {
public:
    CBSPPart *markerCube;
    LegInfo legs[2] = {};   // init all zeroes
    LegInfo legUndo[2] = {};
    Fixed legSpeeds[2] = {};

    Fixed viewPortHeight;

    Fixed targetHeight;
    Fixed baseFriction;
    Fixed elevation;
    Fixed stance;
    Fixed crouch;

    Fixed headHeight;
    Fixed legPhase;
    Fixed legConstant;
    Fixed absAvgSpeed;

    Fixed phaseUndo;
    Fixed stanceUndo;
    Fixed crouchUndo;

    Fixed speedLimit;
    Fixed jumpBasePower;

    Boolean jumpFlag;
    Boolean tractionFlag;
    Boolean oldTractionFlag;

    virtual void StartSystems();
    virtual void LoadParts();
    virtual void Dispose();

    virtual void KeyboardControl(FunctionTable *ft);

#if 0
    virtual	void			Push(Fixed *direction);
#endif
    virtual void TractionControl();
    virtual void MotionControl();
    virtual void DoStandingTouches();
    virtual void DoLegTouches();
    virtual void MoveLegs();
    virtual void ExtendLeg(LegInfo *theLeg);

    virtual void PlayerWasMoved();

    virtual void PlaceParts();
    virtual void AvoidBumping();

    virtual void PrepareForLegUndo();
    virtual void UndoLegs();

    virtual void ReceiveConfig(PlayerConfigRecord *config);
};
