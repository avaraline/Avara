/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CAbstractActor.h
    Created: Sunday, November 20, 1994, 19:15
    Modified: Monday, September 16, 1996, 18:49
*/

#pragma once
#include "CAvaraGame.h"
#include "CDirectObject.h"
#include "ColorManager.h"
#include "CSoundHub.h"
#include "FastMat.h"
#include "LevelLoader.h"
#include "RayHit.h"
#include "Resource.h"

// define ENABLE_FPS_DEBUG in files where you want FPS_DEBUG output, BEFORE including this header
#ifdef ENABLE_FPS_DEBUG
#include <iostream>
#define FPS_DEBUG(stuff) std::cout << stuff
#else
#define FPS_DEBUG(stuff) // noop
#endif

#define MAXPARTS 6
#define DEFAULTHITVOLUME 25
#define DEFAULTBLASTVOLUME 40

#define kPlayerBit 1
#define kScoutBit 2
#define kRobotBit 4
#define kTeleportBit 8
#define kSolidBit 16
#define kTargetBit 32
#define kDoorIgnoreBit 64
#define kCollisionDamageBit 128

#define kTargetGroup1 256
#define kTargetGroup2 512
#define kTargetGroup3 1024
#define kTargetGroup4 2048
#define kTargetGroup5 4096
#define kTargetGroup6 8192
#define kBallSnapBit 16384
#define kCanPushBit 32768

#define kAllKindsBits -1

enum { kNeutralTeam, kGreenTeam, kYellowTeam, kRedTeam, kPinkTeam, kPurpleTeam, kBlueTeam, kOrangeTeam, kLimeTeam };

enum { kIsInactive = 0, kIsActive = 1, kIsGlowing = 2, kHasImpulse = 4, kHasMessage = 8 };

enum { kSnapNot = 0, kSnapAttract, kSnapHere };

enum {
    kNullSignal = 0,
    kBallReleaseSignal,
    kDoSelfDestruct,
    kDoRelease,
    kBallAttached,
    kDoReset,
    kHostDamage,
    kDidGoalSignal
};

class CAvaraGame;
class CSmartPart;
class CAbstractActor;

typedef struct {
    Vector blastPoint;
    Fixed blastPower;
    short team;
    short playerId;
} BlastHitRecord;

typedef union {
    CSmartPart *p;
    CAbstractActor *a;
} ActorOrPartLink;

class CAbstractActor : public CDirectObject {
public:
    CAvaraGame *itsGame;

    unsigned long searchCount;
    ActorLocator locLinks[4]; //	Location link table.

    CSmartPart *partList[MAXPARTS + 1];
    CSmartPart *cachePart; //	Collision detection cache part.
    Fixed partScale;
    Fixed partYon;

    ActorAttachment *attachmentList;

    ActorOrPartLink proximityList;

    CAbstractActor *nextActor;
    CAbstractActor *identLink;
    long ident;
    long sleepTimer;

    CAbstractActor *postMortemLink;
    Fixed blastPower;

    MaskType maskBits;

    Fixed mass;
    Fixed shields;
    SoundLink *itsSoundLink;

    short partCount;
    short isActive;

    short blastSound;
    Fixed blastVolume;

    short teamMask;
    short teamColor;

    long hitScore;
    short hitSoundId;
    short hitSoundVolume;

    long destructScore;

    short stepSound;

    MsgType hitMessage;
    MsgType destructMessage;
    MsgType stepOnMessage;

    short sliverCounts[kSliverSizes];
    short sliverLives[kSliverSizes];

    Boolean isInGame;

    Fixed traction;
    Fixed friction;

    virtual void LoadPart(short ind, short resId);
    virtual void LoadPartWithColors(short ind, short resId);
    virtual void IAbstractActor();
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void AdaptableSettings();
    virtual void AddToGame();
    virtual void FrameAction();

    virtual void LevelReset();
    virtual void ResumeLevel();
    virtual void PauseLevel();
    virtual void Dispose(); //	Simply go away

    virtual void Shatter(short firstSliverType, short sizesCount, short *sCounts, short *sLives, Fixed speedFactor);

    virtual void Blast(); //	Blow up!

    virtual short GetActorScoringId();

    virtual void WasDestroyed();
    virtual void WasHit(RayHitRecord *theHit, Fixed hitEnergy);

    virtual Boolean GetActorCenter(Fixed *loc);
    virtual void GetBlastPoint(BlastHitRecord *theHit, RayHitRecord *rayHit);
    virtual void BlastHit(BlastHitRecord *theHit);

    virtual void Slide(Fixed *direction);
    virtual void Push(Fixed *direction);
    virtual void Accelerate(Fixed *direction);
    virtual void GetSpeedEstimate(Fixed *theSpeed);

    virtual void RadiateDamage(BlastHitRecord *blastRecord);
    virtual void PostMortemBlast(short scoreTeam, short scoreId, Boolean doDispose);
    virtual void SecondaryDamage(short scoreTeam, short scoreColor);

    virtual CSmartPart *DoCollisionTest(CSmartPart **hitList);
    virtual void BuildPartProximityList(Fixed *origin, Fixed range, MaskType filterMask);
    virtual void BuildActorProximityList(Fixed *origin, Fixed range, MaskType filterMask);
    virtual void BuildPartSpheresProximityList(MaskType filterMask);

    virtual void DoSound(short resId, Fixed *where, Fixed volume, Fixed rate);

    virtual void RegisterReceiver(MessageRecord *theMsg, MsgType messageNum);
    virtual void ImmediateMessage();

    virtual void OffsetParts(Fixed *offset);
    virtual void PlaceParts();

    virtual long Attach(ActorAttachment *a);
    virtual void Detach(ActorAttachment *a);
    virtual void ReleaseAttachment();
    virtual void ReleaseAllAttachments();
    virtual long SignalAttachments(long theSignal, void *miscData);
    virtual long ReceiveSignal(long theSignal, void *miscData);
    virtual Fixed GetTotalMass();

    virtual void StandingOn(CAbstractActor *who, Fixed *where, Boolean firstLeg);
    virtual void StandOn(CAbstractActor *who, Fixed *where, Boolean firstLeg, Boolean firstTouch);
    virtual void GetFrictionTraction(Fixed *tract, Fixed *frict);

    //	Location link entry handling.
    virtual void InitLocationLinks();
    void LinkSphere(Fixed *origin, Fixed range);
    void LinkBox(Fixed minX, Fixed minZ, Fixed maxX, Fixed maxZ);
    void LinkPartBoxes();
    void LinkPartSpheres();
    void UnlinkLocation();

    virtual void RayTest(RayHitRecord *hitRec, MaskType testMask);
    virtual void RayTestWithGround(RayHitRecord *hitRec, MaskType testMask);
    virtual short GetPlayerPosition();

    uint32_t GetTeamColorOr(uint32_t defaultColor);

    virtual short GetBallSnapPoint(long theGroup,
        Fixed *ballLocation,
        Fixed *snapDest,
        Fixed *delta,
        CSmartPart **hostPart);

    // subclasses override if they can handle all the frames when running faster than CLASSICFRAMETIME
    virtual bool HandlesFastFPS() { return false; }
    void FpsCoefficients(Fixed classicCoeff1, Fixed classicCoeff2,
                         Fixed* fpsCoeff1, Fixed* fpsCoeff2, Fixed* fpsOffset = NULL);
    Fixed FpsCoefficient1(Fixed classicMultiplier1);
    Fixed FpsCoefficient2(Fixed classicMultiplier2);
    Fixed FpsOffset(Fixed classicCoeff2);
    long FpsFramesPerClassic(long classicFrames = 1);
    Fixed ClassicCoefficient2(Fixed fpsValue);
private:
    virtual double FpsCoefficient1(double classicCoeef1, double fpsScale);
};
