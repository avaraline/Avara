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
    Vector origLocation;
    ActorAttachment clamp;
    long hostIdent;
    long oldHost;
    long looseFrame;
    CSmartPart *hostPart;
    Vector localSnap;

    MessageRecord startMsg;
    MessageRecord stopMsg;
    MsgType goalMsg;
    MsgType captureMsg;

    long carryScore;
    long goalScore;

    Fixed pitchZ;
    Fixed pitchY;

    Fixed classicGravity, customGravity;
    Fixed classicAcceleration, acceleration;

    Fixed changeHolderPower;
    short holderScoreId;
    short holderTeam;
    Fixed holdShieldLimit;

    Fixed releaseHoldAccumulator;

    short status;
    long group;
    short actionCommand;

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
