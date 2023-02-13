/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CAvaraGame.h
    Created: Sunday, November 20, 1994, 19:01
    Modified: Sunday, September 15, 1996, 21:09
*/

#pragma once
#include "AvaraDefines.h"
#include "AvaraGL.h"
#include "AvaraScoreInterface.h"
#include "AvaraTypes.h"
#include "CDirectObject.h"
#include "Types.h"
#include "CNetManager.h"

#include <SDL2/SDL.h>
#include <string>

#define IDENTTABLESIZE 512

#define MESSAGEHASH 128
#define LOCATORTABLEBITS 6
#define LOCATORTABLESIZE (1L << (2 * LOCATORTABLEBITS))
#define LOCATORRECTSCALE (16 + 5) /*	Each locator square is 32 x 32 metres */
#define LOCATORRECTSIZE (1L << LOCATORRECTSCALE)
#define LOCCOORDMASK (((1L << LOCATORTABLEBITS) - 1) << LOCATORRECTSCALE)
#define LOCCOORDTOINDEX(coord) (((coord)&LOCCOORDMASK) >> LOCATORRECTSCALE)
#define LOCTOTABLE(x, z) \
    ((((x)&LOCCOORDMASK) >> (LOCATORRECTSCALE - LOCATORTABLEBITS)) | (((z)&LOCCOORDMASK) >> LOCATORRECTSCALE))

#define kJoystickMode 1
#define kFlipAxis 2
#define kSimulChatMode 4

#define kAmbientSoundToggle 1
#define kTuijaToggle 2
#define kInterpolateToggle 4
#define kMissileLoopToggle 8
#define kFootStepToggle 16
#define kMusicToggle 32

#define INACTIVE_LOOP_REFRESH 16

enum { kPlayingStatus, kAbortStatus, kReadyStatus, kPauseStatus, kNoVehicleStatus, kWinStatus, kLoseStatus };

class CAbstractActor;
class CAbstractPlayer;
class CPlayerManager;

class CBSPWorld;
class CBSPPart;
class CViewParameters;

// class	CStatusDisplay;
// class	CInfoPanel;
class CDepot;

class CAvaraApp;

class CSoundHub;
class CIncarnator;
class CWorldShader;
class CScoreKeeper;
class CAbstractYon;

class CHUD;

class CAvaraGame : public CDirectObject {
public:
    std::string loadedTag = "";
    std::string loadedLevel = "";
    std::string loadedSet = "";
    std::string loadedDesigner = "";
    std::string loadedInfo = "";
    long loadedTimeLimit;
    int32_t timeInSeconds;
    FrameNumber frameNumber;
    bool isClassicFrame;
    int32_t frameAdjust;

    FrameNumber topSentFrame;

    FrameTime frameTime; //	In milliseconds.
    double fpsScale;  // 0.25 => CLASSICFRAMETIME / 4

    short gameStatus;
    short statusRequest;
    short pausePlayer;

    CAbstractActor *actorList;
    CAbstractActor *nextActor = {0};

    CAbstractPlayer *playerList;
    CAbstractPlayer *nextPlayer = {0};
    CAbstractPlayer *spectatePlayer = {0};
    long playersStanding;
    short teamsStandingMask;
    short teamsStanding;

    CIncarnator *incarnatorList;
    CAbstractPlayer *freshPlayerList;

    CAbstractActor *postMortemList;

    ActorLocator **locatorTable;
    ActorLocator locatorListEnd;
    unsigned long searchCount;

    Fixed friendlyHitMultiplier;
    long scores[kMaxAvaraPlayers];

    long curIdent;
    CAbstractActor *identTable[IDENTTABLESIZE];

    CAvaraApp *itsApp;
    // WindowPtr		itsWindow;
    // PolyWorld		itsPolyWorld;
    CBSPWorld *itsWorld;
    CBSPWorld *hudWorld;
    CViewParameters *itsView;
    CAbstractYon *yonList;

    // UI
    // NVGcontext      *vg;
    int font;
    int windowWidth, windowHeight, bufferWidth, bufferHeight;
    float pxRatio;

    Rect gameRect;

    MessageRecord *messageBoard[MESSAGEHASH];
    Fixed *baseLocation;

    Fixed gravityRatio;

    Fixed groundTraction;
    Fixed groundFriction;

    //	Object oriented submanagers:

    // CInfoPanel		*infoPanel;
    CDepot *itsDepot; //	Storage maintenance for ammo
    CSoundHub *soundHub; //	Sound playback and control hub
    std::unique_ptr<CNetManager> itsNet; //	Networking management
    CWorldShader *worldShader; //	Manages ground and sky colors.
    CScoreKeeper *scoreKeeper;
    CHUD *hud;

    //	Sound related variables:
    int soundTime;
    short soundOutputStyle; //	Mono, speakers stereo, headphones stereo
    short sound16BitStyle; //	true = try 16 bit, false = 8 bit
    short soundSwitches; //	kAmbientSoundToggle & kTuijaToggle
    short groundStepSound;

    //	Networking & user control related stuff:
    Handle mapRes; //	Keyboard mapping resource handle.

    short moJoOptions; //	Mouse and Joystick options.
    double sensitivity;

    double latencyTolerance;

    ScoreInterfaceReasons scoreReason;
    ScoreInterfaceReasons lastReason;

    uint32_t nextScheduledFrame;
    uint32_t nextPingTime;
    long lastFrameTime;
    Boolean canPreSend;

    Boolean didWait;
    Boolean longWait;
    Boolean veryLongWait;
    Boolean allowBackgroundProcessing;
    Boolean simpleExplosions;
    Boolean keysFromStdin;
    Boolean keysToStdout;

    // Moved here from GameLoop so it can run on the normal event loop
    // long            frameCredit;
    // long            frameAdvance;
    // short           consecutiveSkips;
    long oldPlayersStanding;
    short oldTeamsStanding;

    CAvaraGame(FrameTime frameTime = 64);
    //	Methods:
    virtual void IAvaraGame(CAvaraApp *theApp);
    virtual CBSPWorld* CreateCBSPWorld(short initialObjectSpace);
    virtual CSoundHub* CreateSoundHub();
    virtual std::unique_ptr<CNetManager> CreateNetManager();

    virtual void InitLocatorTable();

    virtual CAbstractActor *FindIdent(long ident);
    virtual void GetIdent(CAbstractActor *theActor);
    virtual void RemoveIdent(long ident);

    virtual CAbstractPlayer *GetSpectatePlayer();
    virtual CAbstractPlayer *GetLocalPlayer();

    virtual void AddActor(CAbstractActor *theActor);
    virtual void RemoveActor(CAbstractActor *theActor);

    virtual void ResumeActors();
    virtual void PauseActors();
    virtual void RunFrameActions();
    virtual void RunActorFrameActions();

    virtual void Score(short team, short player, long points, Fixed energy, short hitTeam, short hitPlayer);

    virtual void ChangeDirectoryFile();
    virtual void LevelReset(Boolean clearReset);
    virtual void EndScript();

    virtual void ReadGamePrefs();

    virtual void SendStartCommand();
    virtual void StartIfReady();
    virtual void ResumeGame();
    virtual bool IsPlaying();
    virtual void HandleEvent(SDL_Event &event);
    virtual void GameStart();
    virtual bool GameTick();
    virtual void GameStop();
    virtual void Dispose();

    virtual void SpectateNext();
    virtual void SpectatePrevious();
    virtual bool canBeSpectated(CAbstractPlayer *player);


    virtual void UpdateViewRect(int width, int height, float pixelRatio);

    virtual void RegisterReceiver(MessageRecord *theMsg, MsgType messageNum);
    virtual void RemoveReceiver(MessageRecord *theMsg);
    virtual void FlagMessage(MsgType messageNum);
    virtual void FlagImmediateMessage(MsgType messageNum);
    virtual void MessageCleanup(CAbstractActor *deadActor);

    virtual void StopGame();
    virtual void Render(NVGcontext *ctx);
    virtual void ViewControl();

    virtual void InitMixer(Boolean silentFlag);

    virtual CPlayerManager *GetPlayerManager(CAbstractPlayer *thePlayer);
    virtual CPlayerManager *FindPlayerManager(CAbstractPlayer *thePlayer);

    virtual long RoundTripToFrameLatency(long rtt);
    virtual void SetFrameLatency(short newFrameLatency, short maxChange = 2, CPlayerManager *slowPlayer = nullptr);
    virtual FrameNumber TimeToFrameCount(long timeInMsec);
    virtual FrameNumber NextFrameForPeriod(long period, long referenceFrame = 0);
    virtual void SetFrameTime(int32_t ft);
    virtual void IncrementFrame(bool firstFrame = false);
    virtual FrameNumber FramesFromNow(FrameNumber classicFrames);

    void SetKeysFromStdin() { keysFromStdin = true; };
    void SetKeysToStdout() { keysToStdout = true; };
};

#ifndef MAINAVARAGAME
extern CAvaraGame *gCurrentGame;
extern CSoundHub *gHub;
#else
CAvaraGame *gCurrentGame = NULL;
CSoundHub *gHub = NULL;
#endif
