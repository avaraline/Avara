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
#include "ColorManager.h"
#include "GoodyRecord.h"
#include "KeyFuncs.h"
#include "PlayerConfig.h"

class CBSPPart;
class CScaledBSP;
class CScout;
class CFreeCam;
class CPlayerManager;
class CIncarnator;
class CAbstractPlayer;

#define kDirIndBSP 204
#define kTargetOff 205
#define kTargetOk 206
#define kLockLight 207
#define kGroundDirArrow 190
#define kGroundDirArrowSlow 191
#define kGroundDirArrowFast 192
#define kFilledBox 400
#define kEmptyBox 720
#define kGaugeBSP 402
#define kMissileBSP 802
#define kGrenadeBSP 820
#define kEnergyBSP 604
#define kShieldBSP 603
#define kBoosterBSP 600

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

// HUD Preset options
enum HUDPreset {
    Close,
    Far 
};

class CAbstractPlayer : public CRealMovers {
public:
    std::shared_ptr<CPlayerManager> itsManager;
    CAbstractPlayer *nextPlayer = 0;
    PlayerConfigRecord defaultConfig {};
    HullConfigRecord hullConfig {};

    ARGBColor longTeamColor = 0;
    bool hasTeammates = false;

    //	Shields & energy:
    Fixed energy = 0;
    Fixed maxEnergy = 0; //	Maximum stored energy level
    Fixed classicGeneratorPower, generatorPower = 0; //	Energy gain/frame
    FrameNumber boostEndFrame = 0;
    long boostsRemaining = 0;

    Fixed maxShields = 0; //	Maximum shield energy
    Fixed classicShieldRegen, shieldRegen = 0; //	Shield regeneration rate

    short missileCount = 0;
    short grenadeCount = 0;
    FrameNumber nextGrenadeLoad = 0;
    FrameNumber nextMissileLoad = 0;
    FrameNumber nextPlasmaShot = 0;

    short missileLimit = 0;
    short grenadeLimit = 0;
    short boosterLimit = 0;

    //	Movement related variables:
    Fixed baseMass = 0;
    Fixed motors[2] = {0, 0}; //	Left/right speed
    Fixed maxAcceleration = 0;
    Fixed classicMotorFriction = 0;
    Fixed motorFriction = 0;
    Fixed classicMotorAcceleration = 0;
    Fixed motorAcceleration = 0;
    Fixed fpsMotorOffset = 0;
    Fixed turningEffect = 0; //	How far apart are the legs or wheels?
    Fixed movementCost = 0; //	Cost of acceleration
    Fixed proximityRadius = 0;

    Vector groundSlide = {0};

    Fixed distance = 0; //	Movement within one frame
    Fixed headChange = 0;
    Boolean didBump = false;

    //	Guns:
    Fixed gunEnergy[2] = {0, 0}; //	Left/right guns
    Fixed fullGunEnergy = 0;
    Fixed activeGunEnergy = 0;
    Fixed classicChargeGunPerFrame, chargeGunPerFrame = 0;
    long mouseShootTime = 0; //	To pace mouse button autofire.
    Vector gunOffset = {0};
    Boolean fireGun = 0;

    //	Weapons (Grenades & missiles)
    long weaponIdent = 0;

    //	View related variables:
    CSmartPart *viewPortPart = 0; //	We look out from this one.
    CScout *itsScout = 0; //
    short scoutCommand = 0;
    CFreeCam *itsFreeCam = 0;

    //	Control module (and view) orientation:
    Fixed viewYaw = 0;
    Fixed dYaw = 0;  // Delta Yaw - How far the yaw changed since the last frame
    Fixed viewPitch = 0;
    Fixed dPitch = 0; // Delta Pitch - How far the pitch changed since the last frame
    Fixed oldElevation = 0;
    Fixed dElevation = 0;
    Vector viewOffset = {0};
    Fixed lookDirection = 0;

    Fixed minPitch = 0;
    Fixed maxPitch = 0;
    Fixed maxYaw = 0;
    Fixed oldYaw = 0; //	We have to be able to undo the motion.
    Fixed oldPitch = 0;

    Fixed yonBound = 0;
    Fixed maxFOV = 0;
    Fixed fieldOfView = 0; //	Angle of field of view.
    Fixed scoutBaseHeight = 0;

    SoundLink *teleportSoundLink = 0;
    SoundLink *boostControlLink = 0;

    //	Reincarnation:
    long limboCount = 0;
    Fixed incarnateVolume = 0;
    short incarnateSound = 0;
    Boolean doIncarnateSound = 0;
    Boolean reEnergize = 0;
    Boolean didSelfDestruct = 0;
    bool didIncarnateMasked = false;

    //	Winning/loosing:
    FrameNumber winFrame = 0;
    Quaternion winStart = {0};
    Quaternion winEnd = {0};
    Boolean isOut = 0;
    short lives = 0;

    short winSound = 0;
    Fixed winVolume = 0;

    short loseSound = 0;
    Fixed loseVolume = 0;

    short chatMode = 0;

    long scoutIdent = 0; //	true, if scout is out.
    Boolean scoutView = 0; //	true = scout view, false = normal view
    long freeCamIdent = 0;
    Boolean freeView = 0;
    Boolean isInLimbo = 0;
    Boolean debugView = 0;
    Boolean netDestruct = 0;

    Fixed supportTraction = 0;
    Fixed supportFriction = 0;

    std::deque<float> freeCamDBG;

    //	Hud parts:
    CBSPPart *dirArrow = 0;
    Fixed dirArrowHeight = 0;

    CBSPPart *targetOffs[2] = {0, 0};
    CBSPPart *targetOns[2] = {0, 0};

    // Dashboard HUD
    RayHitRecord dashboardOrigin;
    Fixed dashboardSpinHeading;
    Fixed dashboardSpinSpeed;
    float hudRestingX;
    float hudRestingY;
    float hudTargetX;
    float hudTargetY;
    PidMotion pMotionX;
    PidMotion pMotionY;
    CScaledBSP *lockLight = 0;
    CScaledBSP *groundDirArrow = 0;
    CScaledBSP *groundDirArrowSlow = 0;
    CScaledBSP *groundDirArrowFast = 0;
    CScaledBSP *shieldGauge = 0;
    CScaledBSP *shieldGaugeBackLight = 0;
    CScaledBSP *energyGauge = 0;
    CScaledBSP *energyGaugeBackLight = 0;
    CScaledBSP *energyLabel = 0;
    CScaledBSP *shieldLabel = 0;
    CScaledBSP *grenadeLabel = 0;
    CScaledBSP *missileLabel = 0;
    CScaledBSP *boosterLabel = 0;
    CScaledBSP *livesLabel = 0;
    CScaledBSP *grenadeBox[4] = {0, 0, 0, 0};
    CScaledBSP *missileBox[4] = {0, 0, 0, 0};
    CScaledBSP *boosterBox[4] = {0, 0, 0, 0};
    CScaledBSP *livesBox[4] = {0, 0, 0, 0};
    CScaledBSP *grenadeMeter[4] = {0, 0, 0, 0};
    CScaledBSP *missileMeter[4] = {0, 0, 0, 0};
    CScaledBSP *boosterMeter[4] = {0, 0, 0, 0};
    CScaledBSP *livesMeter[4] = {0, 0, 0, 0};

    // HUD Layout Prefs
    int layout;
    float layoutScale;
    Fixed hudAlpha;
    int gaugeBSP;
    float arrowDistance;
    float arrowScale;
    float livesPosition[2];
    float boosterPosition[2];
    float grenadePosition[2];
    float missilePosition[2];
    float shieldPosition[2];
    float energyPosition[2];
    float offsetMultiplier;
    float boosterSpacing;
    float livesSpacing;
    float weaponSpacing;
    Boolean showHud; // Store pref to detect if the user changes it during a game
    int hudPreset; // Store pref to detect if the user changes it during a game

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void AdaptableSettings();
    virtual void LoadHUDParts();
    virtual void ReplacePartColors();
    virtual void SetSpecialColor(ARGBColor specialColor);
    virtual void LoadParts();
    virtual void LoadScout();
    virtual void LoadFreeCam();
    virtual void WriteDBG(float val);
    virtual void StartSystems();
    virtual void LevelReset();

    virtual ~CAbstractPlayer();
    virtual void DisposeDashboard();

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
    
    //
    virtual void DashboardReloadCheck();
    virtual void LoadDashboardParts();
    virtual CScaledBSP* DashboardPart(uint16_t id);
    virtual CScaledBSP* DashboardPart(uint16_t id, Fixed scale);
    virtual void RenderDashboard();
    virtual void DashboardPosition(CScaledBSP *part, float x, float y, bool useZOffset);
    virtual void DashboardPosition(CScaledBSP *part, bool autoRot, float x, float y, bool useZOffset);
    virtual void DashboardPosition(CScaledBSP *part, bool autoRot, float x, float y, bool useZOffset, Fixed x_rot, Fixed y_rot, Fixed z_rot);
    virtual void DashboardFixedPosition(CScaledBSP *part, float dist, Fixed angle);
    virtual void DashboardFixedPosition(CScaledBSP *part, float dist, Fixed angle, float height, Fixed x_rot, Fixed y_rot, Fixed z_rot);
    virtual void ResetDashboard();
    //

    virtual void ToggleFreeCam();
    virtual void SetFreeCamState(Boolean state);
    virtual Boolean IsFreeCamAttached();
    virtual void ControlSoundPoint();
    virtual void ControlViewPoint();
    virtual void RecalculateViewDistance();
    virtual void ResetCamera();

    virtual short GetActorScoringId();
    virtual void PostMortemBlast(short scoreTeam, short scoreId, Boolean doDispose);

    virtual void GoLimbo(FrameNumber limboDelay);
    virtual void Incarnate();
    virtual Fixed ClosestOpponentDistance(Vector &location);
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
