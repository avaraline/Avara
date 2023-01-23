/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CAbstractPlayer.h
    Created: Friday, March 10, 1995, 04:11
    Modified: Saturday, September 7, 1996, 18:04
*/

#pragma once
#include "CRealMovers.h"
//#include "LevelScoreRecord.h"
#include "CAvaraApp.h"
#include "GoodyRecord.h"
#include "KeyFuncs.h"
#include "PlayerConfig.h"

class CBSPPart;
class CScout;
class CPlayerManager;
class CIncarnator;
class CAbstractPlayer;

#define kDirIndBSP 204
#define kTargetOff 205
#define kTargetOk 206

#define kDefaultTeleportSound 410

#define SIGHTSAMBIENT FIX3(400)
#define GUNREADYAMBIENT FIX3(400)
#define GUNFULLAMBIENT FIX3(800)

//	Teleporter options:
enum {
    kSpinOption = 1,
    kFragmentOption = 2,
    kShowAlwaysOption = 4 //	Used by teleporter itself
};

class CAbstractPlayer : public CRealMovers {
public:
    CPlayerManager *itsManager;
    CAbstractPlayer *nextPlayer;
    PlayerConfigRecord defaultConfig;

    long longTeamColor; // Hull color in 0x00RRGGBB format.

    //	Shields & energy:
    Fixed energy;
    Fixed maxEnergy; //	Maximum stored energy level
    Fixed classicGeneratorPower, generatorPower; //	Energy gain/frame
    long boostEndFrame;
    long boostsRemaining;

    Fixed maxShields; //	Maximum shield energy
    Fixed classicShieldRegen, shieldRegen; //	Shield regeneration rate

    short missileCount;
    short grenadeCount;
    long nextGrenadeLoad;
    long nextMissileLoad;

    short missileLimit;
    short grenadeLimit;
    short boosterLimit;

    //	Movement related variables:
    Fixed baseMass;
    Fixed motors[2]; //	Left/right speed
    Fixed maxAcceleration;
    Fixed classicMotorFriction;
    Fixed motorFriction;
    Fixed classicMotorAcceleration;
    Fixed motorAcceleration;
    Fixed fpsMotorOffset;
    Fixed turningEffect; //	How far apart are the legs or wheels?
    Fixed movementCost; //	Cost of acceleration
    Fixed proximityRadius;

    Vector groundSlide;

    Fixed distance; //	Movement within one frame
    Fixed headChange;
    Boolean didBump;

    //	Guns:
    Fixed gunEnergy[2]; //	Left/right guns
    Fixed fullGunEnergy;
    Fixed activeGunEnergy;
    Fixed classicChargeGunPerFrame, chargeGunPerFrame;
    long mouseShootTime; //	To pace mouse button autofire.
    Vector gunOffset;
    Boolean fireGun;

    //	Weapons (Grenades & missiles)
    long weaponIdent;

    //	View related variables:
    CSmartPart *viewPortPart; //	We look out from this one.
    CScout *itsScout; //
    short scoutCommand;

    //	Control module (and view) orientation:
    Fixed viewYaw;
    Fixed viewPitch;
    Vector viewOffset;
    Fixed lookDirection;

    Fixed minPitch;
    Fixed maxPitch;
    Fixed maxYaw;
    Fixed oldYaw; //	We have to be able to undo the motion.
    Fixed oldPitch;

    Fixed yonBound;
    Fixed maxFOV;
    Fixed fieldOfView; //	Angle of field of view.
    Fixed scoutBaseHeight;

    SoundLink *teleportSoundLink;
    SoundLink *boostControlLink;

    //	Reincarnation:
    long limboCount;
    Fixed incarnateVolume;
    short incarnateSound;
    Boolean doIncarnateSound;
    Boolean reEnergize;
    Boolean didSelfDestruct;

    //	Winning/loosing:
    long winFrame;
    Quaternion winStart;
    Quaternion winEnd;
    Boolean isOut;
    short lives;

    short winSound;
    Fixed winVolume;

    short loseSound;
    Fixed loseVolume;

    short chatMode;

    long scoutIdent; //	true, if scout is out.
    Boolean scoutView; //	true = scout view, false = normal view
    Boolean isInLimbo;
    Boolean debugView;
    Boolean netDestruct;

    Fixed supportTraction;
    Fixed supportFriction;

    //	Hud parts:
    CBSPPart *dirArrow;
    Fixed dirArrowHeight;

    CBSPPart *targetOffs[2];
    CBSPPart *targetOns[2];

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void AdaptableSettings();
    virtual void LoadHUDParts();
    virtual void ReplacePartColors();
    virtual void SetSpecialColor(long specialColor);
    virtual void LoadParts();
    virtual void LoadScout();
    virtual void StartSystems();

    virtual void Dispose();

    virtual void ReturnWeapon(short theKind);
    virtual void ArmGrenade();
    virtual void ArmSmartMissile();

    virtual void PlayerAction();
    virtual void FrameAction();
    virtual void KeyboardControl(FunctionTable *ft);

    virtual void Slide(Fixed *direction);

    virtual void TractionControl();
    virtual void MotionControl();
    virtual void GunActions();

#if 0
    virtual	void			FindBestMovement(CSmartPart *bump);
#endif
    virtual void AvoidBumping();

    virtual void PlaceHUDParts();
    virtual void ControlSoundPoint();
    virtual void ControlViewPoint();

    virtual short GetActorScoringId();
    virtual void PostMortemBlast(short scoreTeam, short scoreId, Boolean doDispose);

    virtual void GoLimbo(long limboDelay);
    virtual void Reincarnate();
    virtual bool ReincarnateComplete(CIncarnator *newSpot);
    virtual void IncarnateSound();

    virtual Boolean TryTransport(Fixed *where, short soundId, Fixed volume, short options);
    virtual void ResumeLevel();

    virtual void Win(long winScore, CAbstractActor *teleport);
    virtual void WinAction();
    // virtual	void			FillGameResultRecord(TaggedGameResult *res);
    virtual void ReceiveConfig(PlayerConfigRecord *config);

    virtual Fixed GetTotalMass();

    virtual void PlayerWasMoved();

    virtual void TakeGoody(GoodyRecord *gr);
    virtual short GetPlayerPosition();

    virtual short GetBallSnapPoint(long theGroup,
        Fixed *ballLocation,
        Fixed *snapDest,
        Fixed *delta,
        CSmartPart **hostPart);
    virtual void WasHit(RayHitRecord *theHit, Fixed hitEnergy);

    virtual bool HandlesFastFPS() { return true; }
};
