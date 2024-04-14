/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CBall.h
    Created: Monday, February 26, 1996, 15:36
    Modified: Tuesday, September 3, 1996, 22:04
*/

#pragma once
#include "CRealShooters.h"

class CBall : public CRealShooters {
public:
    Vector origLocation = {0};
    ActorAttachment clamp = {0};
    long hostIdent = 0;
    long oldHost = 0;
    long looseFrame = 0;
    CSmartPart *hostPart = 0;
    Vector localSnap = {0};

    MessageRecord startMsg = {0};
    MessageRecord stopMsg = {0};
    MsgType goalMsg = 0;
    MsgType captureMsg = 0;

    long carryScore = 0;
    long goalScore = 0;

    Fixed pitchZ = 0;
    Fixed pitchY = 0;

    Fixed classicGravity, customGravity;
    Fixed classicAcceleration, acceleration;

    Fixed changeHolderPower = 0;
    short holderScoreId = 0;
    short holderTeam = 0;
    Fixed holdShieldLimit = 0;

    Fixed releaseHoldAccumulator = 0;

    short status = 0;
    long group = 0;
    short actionCommand = 0;

    Fixed carryScoreAccumulator;

    long ownerChangeCount;
    long ownerChangeTime;
    short ownerScoringId;
    short lastOwner;
    ARGBColor origLongColor = 0;

    short origTeam;
    short origWatchTeams;

    Fixed shieldsCharge;
    Fixed maxShields;
    Fixed shootShields;
    Fixed dropDamage;
    Fixed ejectPitch;
    Fixed ejectPower;
    short shapeRes;

    short buzzSound;
    Fixed buzzVolume;
    SoundLink *buzzLink;

    short snapSound;
    Fixed snapVolume;

    short ejectSound;
    Fixed ejectVolume;

    short reprogramSound;
    Fixed reprogramVolume;

    Boolean playerAttach;

    CBall();
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual ~CBall();
    virtual void AdaptableSettings(); // Read settings that can change with frame rate

    virtual void ChangeOwnership(short ownerId, short ownerTeamColor);

    virtual void PlaceParts();
    virtual void ReleaseAttachment();
    virtual void FrameAction();

    virtual void BuzzControl(Boolean doBuzz);
    virtual void MagnetAction();
    virtual long ReceiveSignal(long theSignal, void *miscData);
    virtual void WasHit(RayHitRecord *theHit, Fixed hitEnergy);
    virtual void ReleaseDamage(Fixed hitEnergy);
    virtual short GetShooterId();
    virtual bool HandlesFastFPS() { return true; }
};
