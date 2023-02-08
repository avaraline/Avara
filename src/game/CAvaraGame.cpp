/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CAvaraGame.c
    Created: Sunday, November 20, 1994, 19:09
    Modified: Tuesday, September 17, 1996, 03:22
*/

#define MAINAVARAGAME

/*
#define	TIMING_GRAIN	4
#define MAX_ADVANCE     (3*TIMING_GRAIN)
#define MIN_AVDVANCE    (TIMING_GRAIN/2)
#define MAX_DEBT        (1*TIMING_GRAIN)
#define INIT_ADVANCE    (TIMING_GRAIN)
*/

#include "AvaraGL.h"
#include "CAvaraGame.h"
#include "CAvaraApp.h"
//#include "CGameWind.h"
#include "CCommManager.h"
#include "CDepot.h"
#include "CNetManager.h"
#include "CPlayerManager.h"
//#include "CInfoPanel.h"
#include "CAbstractActor.h"
#include "CAbstractPlayer.h"
#include "CBSPWorld.h"
#include "CSmartPart.h"
#include "CViewParameters.h"
#include "InfoMessages.h"
#include "Messages.h"
//#include "CCapMaster.h"
#include "AvaraDefines.h"
#include "CSoundHub.h"
#include "CSoundMixer.h"
#include "CommandList.h"
#include "KeyFuncs.h"
#include "Parser.h"
//#include "LowMem.h"
#include "CWorldShader.h"
//#include "Power.h"
#include "CScoreKeeper.h"
//#include "CRosterWindow.h"
//#include "Sound.h"
#include "CHUD.h"
#include "Preferences.h"
#include "Resource.h"
#include "ARGBColor.h"
#include "Debug.h"

#define kHighShadeCount 12

void CAvaraGame::InitMixer(Boolean silentFlag) {
    CSoundMixer *aMixer;

    soundHub->MixerDispose();

    aMixer = new CSoundMixer;
    aMixer->ISoundMixer(rate22khz, 64, 8, true, true, false);
    aMixer->SetStereoSeparation(true);
    aMixer->SetSoundEnvironment(FIX(400), FIX(5), CLASSICFRAMETIME);
    aMixer->SetVolume(gApplication->Get<uint8_t>(kSoundVolume));
    soundHub->AttachMixer(aMixer);
    soundHub->MuteFlag(silentFlag); //(soundOutputStyle < 0);
}

void CAvaraGame::InitLocatorTable() {
    long i;
    ActorLocator **p, *q;

    locatorListEnd.next = NULL;
    locatorListEnd.prev = NULL;
    locatorListEnd.me = NULL;

    q = &locatorListEnd;
    p = locatorTable;
    for (i = 0; i < LOCATORTABLESIZE; i++) {
        *p++ = q;
    }
}

CNetManager* CAvaraGame::CreateNetManager() {
    return new CNetManager;
}

CAvaraGame::CAvaraGame(FrameTime frameTime) {
    SetFrameTime(frameTime);
}
void CAvaraGame::IAvaraGame(CAvaraApp *theApp) {
    itsApp = theApp;

    itsNet = CreateNetManager();
    itsNet->INetManager(this);
    itsApp->SetNet(itsNet);

    searchCount = 0;
    locatorTable = (ActorLocator **)NewPtr(sizeof(ActorLocator *) * LOCATORTABLESIZE);
    InitLocatorTable();

    gameStatus = kAbortStatus;
    actorList = NULL;
    freshPlayerList = NULL;
    playerList = NULL;
    spectatePlayer = NULL;

    soundSwitches = 0xFF;

    // CalcGameRect();

    itsWorld = CreateCBSPWorld(100);

    hudWorld = CreateCBSPWorld(16);

    soundHub = CreateSoundHub();
    gHub = soundHub;

    InitMixer(true);

    scoreKeeper = new CScoreKeeper;
    scoreKeeper->IScoreKeeper(this);

    itsDepot = new CDepot;
    itsDepot->IDepot(this);

    itsView = new CViewParameters;
    itsView->IViewParameters();
    itsView->hitherBound = FIX3(600);
    itsView->yonBound = LONGYON;
    itsView->horizonBound = FIX(16000); //	16 km

    mapRes = GetResource(FUNMAPTYPE, FUNMAPID);

    // IGameTimer(frameTime/TIMING_GRAIN);

    worldShader = new CWorldShader;
    worldShader->IWorldShader(this);
    worldShader->skyShadeCount = kHighShadeCount;
    worldShader->Apply();

    hud = new CHUD(this);

    allowBackgroundProcessing = false;

    loadedTag = "";
    loadedLevel = "";
    loadedDesigner = "";
    loadedInfo = "";
    loadedTimeLimit = 600;
    timeInSeconds = 0;
    simpleExplosions = false;
    keysFromStdin = false;
    keysToStdout = false;

    statusRequest = -1; // who decided to make "playing" 0??

    nextPingTime = 0;

    // CalcGameRect();

    // vg = AvaraVGContext();
    // font = nvgCreateFont(vg, "sans", BundlePath("fonts/Roboto-Regular.ttf"));
}

CSoundHub* CAvaraGame::CreateSoundHub() {
    CSoundHubImpl *soundHub = new CSoundHubImpl;
    soundHub->ISoundHub(32, 32);
    return soundHub;
}

CBSPWorld* CAvaraGame::CreateCBSPWorld(short initialObjectSpace) {
    CBSPWorldImpl *w = new CBSPWorldImpl;
    w->IBSPWorld(initialObjectSpace);
    return w;
}

void CAvaraGame::Dispose() {
    CAbstractActor *nextActor;

    spectatePlayer = NULL;
    LevelReset(false);

    scoreKeeper->Dispose();
    itsDepot->Dispose();

    while (actorList) {
        nextActor = actorList->nextActor;
        actorList->Dispose();
        actorList = nextActor;
    }

    while (freshPlayerList) {
        nextActor = freshPlayerList->nextActor;
        freshPlayerList->Dispose();
        freshPlayerList = (CAbstractPlayer *)nextActor;
    }

    if (itsWorld)
        itsWorld->Dispose();
    if (hudWorld)
        hudWorld->Dispose();
    if (itsView)
        itsView->Dispose();
    if (soundHub)
        soundHub->Dispose();
    if (itsNet)
        itsNet->Dispose();
    if (worldShader)
        worldShader->Dispose();
    ReleaseResource(mapRes);

    // DisposePolyWorld(&itsPolyWorld);
    DisposePtr((Ptr)locatorTable);

    CDirectObject::Dispose();
}

CAbstractActor *CAvaraGame::FindIdent(long ident) {
    CAbstractActor *theActor;

    if (ident) {
        theActor = identTable[ident & (IDENTTABLESIZE - 1)];
        while (theActor && theActor->ident != ident) {
            theActor = theActor->identLink;
        }
    } else
        theActor = NULL;

    return theActor;
}

void CAvaraGame::GetIdent(CAbstractActor *theActor) {
    CAbstractActor **hashLink;

    theActor->ident = ++curIdent;
    hashLink = identTable + (curIdent & (IDENTTABLESIZE - 1));
    theActor->identLink = *hashLink;
    *hashLink = theActor;
}

void CAvaraGame::RemoveIdent(long ident) {
    CAbstractActor *theActor;
    CAbstractActor **backLink;

    if (ident) {
        backLink = identTable + (ident & (IDENTTABLESIZE - 1));
        theActor = *backLink;

        while (theActor && theActor->ident != ident) {
            backLink = &theActor->identLink;
            theActor = *backLink;
        }

        if (theActor && theActor->ident == ident) {
            *backLink = theActor->identLink;
            theActor->identLink = NULL;
            theActor->ident = 0;
        }
    }
}

CAbstractPlayer *CAvaraGame::GetSpectatePlayer() {
    return spectatePlayer;
}

CAbstractPlayer *CAvaraGame::GetLocalPlayer() {
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        CPlayerManager *mgr = itsNet->playerTable[i];
        if (mgr && mgr->IsLocalPlayer()) {
            return mgr->GetPlayer();
        }
    }

    return NULL;
}

void CAvaraGame::AddActor(CAbstractActor *theActor) {
    short i;

    theActor->isInGame = true;
    theActor->nextActor = actorList;
    actorList = theActor;

    for (i = 0; i < theActor->partCount; i++) {
        itsWorld->AddPart(theActor->partList[i]);
    }

    GetIdent(theActor);
}

void CAvaraGame::RemoveActor(CAbstractActor *theActor) {
    CAbstractActor *anActor, **actPP;

    if (theActor->isInGame) {
        actPP = &actorList;
        anActor = actorList;

        while (anActor) {
            if (anActor == theActor) {
                short i;

                if (theActor == nextActor)
                    nextActor = theActor->nextActor;

                theActor->isInGame = false;
                theActor->UnlinkLocation();
                *actPP = anActor->nextActor;
                anActor->nextActor = NULL;

                for (i = 0; i < anActor->partCount; i++) {
                    itsWorld->RemovePart(anActor->partList[i]);
                }
                anActor = NULL;
            } else {
                actPP = &(anActor->nextActor);
                anActor = anActor->nextActor;
            }
        }
    }

    RemoveIdent(theActor->ident);
}

void CAvaraGame::RegisterReceiver(MessageRecord *theMsg, MsgType messageNum) {
    MessageRecord **head;

    if (messageNum) {
        head = &messageBoard[messageNum & (MESSAGEHASH - 1)];
        theMsg->next = *head;
        *head = theMsg;
    }

    theMsg->triggerCount = 0;
    theMsg->messageId = messageNum;
}

void CAvaraGame::RemoveReceiver(MessageRecord *theMsg) {
    MessageRecord **head;

    if (theMsg->messageId) {
        head = &messageBoard[theMsg->messageId & (MESSAGEHASH - 1)];

        while (*head) {
            if (*head == theMsg) {
                *head = theMsg->next;
                theMsg->next = NULL;
                theMsg->messageId = 0;
                theMsg->triggerCount = 0;

                break;
            }

            head = &(*head)->next;
        }
    }
}

void CAvaraGame::FlagMessage(MsgType messageNum) {
    MessageRecord *msgP;

    if (messageNum) {

        msgP = messageBoard[messageNum & (MESSAGEHASH - 1)];

        while (msgP) {
            if (msgP->messageId == messageNum) {
                msgP->triggerCount++;
                msgP->owner->isActive |= kHasMessage;
            }

            msgP = msgP->next;
        }
    }
}

void CAvaraGame::FlagImmediateMessage(MsgType messageNum) {
    MessageRecord *msgP;
    MessageRecord *nextMsgP;

    if (messageNum) {
        msgP = messageBoard[messageNum & (MESSAGEHASH - 1)];

        while (msgP) {
            nextMsgP = msgP->next;

            if (msgP->messageId == messageNum) {
                msgP->triggerCount++;
                msgP->owner->isActive |= kHasMessage;
                msgP->owner->ImmediateMessage();
            }

            msgP = nextMsgP;
        }
    }
}

void CAvaraGame::MessageCleanup(CAbstractActor *deadActor) {
    short i;
    MessageRecord *msgP, *nextP;
    MessageRecord **head;

    for (i = 0; i < MESSAGEHASH; i++) {
        head = &messageBoard[i];
        msgP = *head;

        while (msgP) {
            nextP = msgP->next;
            if (msgP->owner == deadActor) {
                *head = nextP;
                msgP->next = NULL;
                msgP->messageId = 0;
                msgP->triggerCount = 0;
            } else {
                head = &msgP->next;
            }
            msgP = nextP;
        }
    }
}

void CAvaraGame::Score(short team, short player, long points, Fixed energy, short hitTeam, short hitPlayer) {
    short tempSwap;

    if (scoreReason == ksiObjectCollision || (scoreReason == ksiKillBonus && lastReason == ksiObjectCollision)) {
        tempSwap = team;
        team = hitTeam;
        hitTeam = tempSwap;

        tempSwap = hitPlayer;
        hitPlayer = player;
        player = tempSwap;

        points = -points;
    }

    lastReason = scoreReason;

    if (hitTeam == team) {
        points = FMul(friendlyHitMultiplier, points);
    }

    if (player >= 0 && player < kMaxAvaraPlayers) {
        scores[player] += points;
    }

    scoreKeeper->Score(scoreReason, team, player, points, energy, hitTeam, hitPlayer);
}

void CAvaraGame::RunFrameActions() {
    // SDL_Log("CAvaraGame::RunFrameActions\n");
    CAbstractPlayer *thePlayer;

    itsDepot->FrameAction();

    postMortemList = NULL;

    RunActorFrameActions();

    itsNet->ProcessQueue();
    if (!latencyTolerance) {
        while (frameNumber > topSentFrame) {
            itsNet->FrameAction();
        }
    }

    thePlayer = playerList;
    while (thePlayer) { // itsNet->ProcessQueue();
        nextPlayer = thePlayer->nextPlayer;
        thePlayer->PlayerAction();
        thePlayer = nextPlayer;
    }
}

void CAvaraGame::RunActorFrameActions() {
    for (CAbstractActor *theActor = actorList;
         theActor != NULL;
         theActor = nextActor)
    {
        // some actors (e.g. weapons) can remove themselves from the list, so save the nextActor before running FrameAction()
        nextActor = theActor->nextActor;
        if (theActor->HandlesFastFPS() || isClassicFrame) {
            if (theActor->isActive || --(theActor->sleepTimer) == 0) {
              theActor->FrameAction();
            }
        }
    }
}

void CAvaraGame::ChangeDirectoryFile() {
    gHub->FreeUnusedSamples();
}

void CAvaraGame::LevelReset(Boolean clearReset) {
    short i;
    CAbstractActor *anActor, *nextActor;

    gameStatus = kAbortStatus;

    gHub->FlagOldSamples();

    loadedLevel = "";
    loadedDesigner = "";
    loadedInfo = "";
    loadedTimeLimit = 600;
    timeInSeconds = 0;

    worldShader->Reset();
    if (clearReset)
        worldShader->skyShadeCount = kHighShadeCount;
    worldShader->Apply();

    // freeBytes = MaxMem(&growBytes);

    // SetPort(itsWindow);
    // InvalRect(&itsWindow->portRect);

    itsNet->LevelReset();

    incarnatorList = NULL;
    IncrementFrame(true);
    topSentFrame = -1 / fpsScale;

    // ResetView();

    itsDepot->LevelReset();

    for (i = 0; i < MESSAGEHASH; i++) {
        messageBoard[i] = NULL;
    }

    anActor = actorList;
    while (anActor) {
        nextActor = anActor->nextActor;
        anActor->LevelReset();
        anActor = nextActor;
    }

    while (freshPlayerList) {
        nextActor = freshPlayerList->nextActor;
        freshPlayerList->Dispose();
        freshPlayerList = (CAbstractPlayer *)nextActor;
    }

    hudWorld->DisposeParts();
    itsWorld->DisposeParts();

    curIdent = 0;
    for (i = 0; i < IDENTTABLESIZE; i++) {
        identTable[i] = NULL;
    }

    friendlyHitMultiplier = 0;
    for (i = 0; i < kMaxAvaraPlayers; i++) {
        scores[i] = 0;
    }

    itsApp->LevelReset();

    baseLocation = NULL;

    yonList = NULL;
    scoreReason = ksiNoReason;
    lastReason = ksiNoReason;
    InitLocatorTable();
}

void CAvaraGame::EndScript() {
    short i;
    ARGBColor color = 0;
    Fixed intensity, angle1, angle2;
    Fixed x, y, z;

    gameStatus = kReadyStatus;
    // SetPolyWorld(&itsPolyWorld);
    worldShader->Apply();

    itsView->ambientLight = ReadFixedVar(iAmbient);
    itsView->ambientLightColor = ARGBColor::Parse(ReadStringVar(iAmbientColor))
        .value_or(DEFAULT_LIGHT_COLOR);
    AvaraGLSetAmbient(ToFloat(itsView->ambientLight), itsView->ambientLightColor);

    for (i = 0; i < 4; i++) {
        intensity = ReadFixedVar(iLightsTable + 4 * i);

        if (intensity >= 2048) {
            angle1 = ReadFixedVar(iLightsTable + 1 + 4 * i);
            angle2 = ReadFixedVar(iLightsTable + 2 + 4 * i);

            x = FMul(FDegCos(angle1), intensity);
            y = FMul(FDegSin(-angle1), intensity);
            z = FMul(FDegCos(angle2), x);
            x = FMul(FDegSin(-angle2), x);
            color = ARGBColor::Parse(ReadStringVar(iLightsTable + 3 + 4 * i))
                .value_or(DEFAULT_LIGHT_COLOR);

            itsView->SetLightValues(i, x, y, z, kLightGlobalCoordinates);
            // SDL_Log("Light from light table - idx: %d i: %f a: %f b: %f c: %x",
            //        i, ToFloat(intensity), ToFloat(angle1), ToFloat(angle2), color);

            //The b angle is the compass reading and the a angle is the angle from the horizon.
            AvaraGLSetLight(i, ToFloat(intensity), ToFloat(angle1), ToFloat(angle2), color);
        } else {
            itsView->SetLightValues(i, 0, 0, 0, kLightOff);
            AvaraGLSetLight(i, 0, 0, 0, DEFAULT_LIGHT_COLOR);
        }
    }

    color = *ARGBColor::Parse(ReadStringVar(iMissileArmedColor));
    ColorManager::setMissileArmedColor(color);
    color = *ARGBColor::Parse(ReadStringVar(iMissileLaunchedColor));
    ColorManager::setMissileLaunchedColor(color);

    friendlyHitMultiplier = ReadFixedVar(iFriendlyHitMultiplier);

    loadedDesigner = ReadStringVar(iDesignerName);
    loadedInfo = ReadStringVar(iLevelInformation);
    loadedTimeLimit = ReadLongVar(iTimeLimit);

    groundTraction = ReadFixedVar(iDefaultTraction);
    groundFriction = ReadFixedVar(iDefaultFriction);
    gravityRatio = ReadFixedVar(iGravity);
    groundStepSound = ReadLongVar(iGroundStepSound);
    gHub->LoadSample(groundStepSound);

    itsDepot->EndScript();

    gHub->FreeOldSamples();

    scoreKeeper->EndScript();
}

void CAvaraGame::ResumeActors() {
    CAbstractActor *theActor;

    playerList = NULL;
    theActor = actorList;

    while (theActor) {
        CAbstractActor *next;

        next = theActor->nextActor;
        theActor->ResumeLevel();
        theActor = next;
    }
}

void CAvaraGame::PauseActors() {
    CAbstractActor *theActor;

    playerList = NULL;
    theActor = actorList;

    while (theActor) {
        CAbstractActor *next;

        next = theActor->nextActor;
        theActor->PauseLevel();
        theActor = next;
    }
}

void CAvaraGame::SendStartCommand() {
    if (gameStatus == kReadyStatus) {
        SDL_Log("CAvaraGame::SendStartCommand(), gameStatus = kReadyStatus\n");
        itsNet->SendStartCommand();
    } else if (gameStatus == kPauseStatus) {
        SDL_Log("CAvaraGame::SendStartCommand(), gameStatus = kPauseStatus\n");
        CAbstractPlayer *player = GetLocalPlayer();
        if(player != NULL && player->lives > 0) {
            SDL_Log("  calling SendResumeCommand()\n");
            itsNet->SendResumeCommand();
        }
    }
}

void CAvaraGame::StartIfReady() {
    // server sends the start command if everyone is "ready"
    if (itsNet->itsCommManager->myId == 0) {
        bool allReady = true;
        for (int i = 0; i < kMaxAvaraPlayers; i++) {
            CPlayerManager *mgr = itsNet->playerTable[i];
            if (mgr && mgr->LoadingStatus() == kLLoaded) {
                allReady = false;
                break;
            }
        }
        if (allReady) {
            SendStartCommand();
        }
    }
}

#define MAXSKIPSINAROW 2

void CAvaraGame::ReadGamePrefs() {
    moJoOptions = 0;
    /*
    if (itsApp->Boolean(kJoystickModeTag)) {
        moJoOptions += kJoystickMode;
    }
    */
    if (itsApp->Boolean(kInvertYAxisTag)) {
        moJoOptions += kFlipAxis;
    }
    sensitivity = pow(2.0, gApplication->Get<double>(kMouseSensitivityTag));
    SDL_Log("mouse sensitivity multiplier = %.2lf\n", sensitivity);
    latencyTolerance = gApplication->Get<double>(kLatencyToleranceTag);
}

void CAvaraGame::ResumeGame() {
    Boolean doStart;
    Boolean freshMission = (gameStatus == kReadyStatus);
    if (freshMission) {
        spectatePlayer = NULL;
    }
    // TODO: gathering players dialog

    ReadGamePrefs();
    doStart = itsNet->GatherPlayers(freshMission);
    SDL_Log("CAvaraGame::ResumeGame (start=%d)\n", doStart);

    if (doStart) {
        if (freshMission) {
            itsApp->GameStarted(loadedSet,
                                loadedLevel);
            itsNet->AttachPlayers((CAbstractPlayer *)freshPlayerList);
            freshPlayerList = NULL;
            InitMixer(false);
        } else {
            itsApp->MessageLine(kmRestarted, MsgAlignment::Center);
            soundHub->MuteFlag(false);
        }

        // TODO: syncing start dialog
        scoreKeeper->StartResume(gameStatus == kReadyStatus);

        // SetPort(itsWindow);
        // SetPolyWorld(&itsPolyWorld);
        // ClearRegions();
        // CalcGameRect();
        // ResizeRenderingArea(&gameRect);
        // ResetView();

        // Start the game, run the first tick
        GameStart();
        GameTick();

        // while(statusRequest == kPlayingStatus)
        //    GameTick();
    }

    // SetPort(itsWindow);
    // InvalRect(&itsWindow->portRect);
}

bool CAvaraGame::IsPlaying() {
    return gameStatus == kPlayingStatus;
}

// Run when the game is started or resumed
void CAvaraGame::GameStart() {
    SDL_Log("CAvaraGame::GameStart\n");
    latencyTolerance = 0;
    didWait = false;
    longWait = false;

    topSentFrame = FramesFromNow(-1);

    statusRequest = kPlayingStatus;
    itsNet->ResumeGame();
    scoreKeeper->PlayerIntros();

    gameStatus = kPlayingStatus;

    // ResetTiming();
    ResumeActors();

    if (frameNumber == 0) {
        FlagMessage(iStartMsg + firstVariable);
    }

    playersStanding = 0;
    teamsStanding = 0;
    nextScheduledFrame = SDL_GetTicks(); // Run next frame immediately

    // frameAdvance = INIT_ADVANCE;
    // frameCredit = frameAdvance << 16;
    canPreSend = false;

    // The difference between the last frame's time and frameTime
    frameAdjust = 0;

    while (FramesFromNow(latencyTolerance) > topSentFrame) {
        itsNet->FrameAction();
    }

    loadedLevel = "";

    // SDL_ShowCursor(SDL_DISABLE);
    // SDL_CaptureMouse(SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1");

    // HideCursor();
    // FlushEvents(everyEvent, 0);

    nanogui::pollEvents = itsApp->Boolean(kPollEvents);
}

// Run when the game is paused or aborted
void CAvaraGame::GameStop() {
    // Mute the mixer
    soundHub->MuteFlag(true);

    if (statusRequest == kPauseStatus) {
        PauseActors();
    }

    gameStatus = statusRequest;
    StopGame();

    itsNet->StopGame(gameStatus);

    if (gameStatus == kAbortStatus) {
        itsApp->MessageLine(kmAborted, MsgAlignment::Center);
    } else if (gameStatus == kPauseStatus) {
        itsApp->ParamLine(kmPaused, MsgAlignment::Center, itsNet->playerTable[pausePlayer]->PlayerName(), NULL);
    }

    scoreKeeper->StopPause(gameStatus == kPauseStatus);

    itsNet->UngatherPlayers();
    
    nanogui::pollEvents = false;
}

void CAvaraGame::HandleEvent(SDL_Event &event) {
    // Queue any keyboard/mouse events for the next frame.
    if (IsPlaying())
        itsNet->HandleEvent(event);
}

bool CAvaraGame::GameTick() {
    uint32_t startTime = SDL_GetTicks();

    // No matter what, process any pending network packets
    itsNet->ProcessQueue();

    if (startTime > nextPingTime) {
        uint32_t pingInterval = 2000; //msec
        if (statusRequest == kPlayingStatus) {
            // experimental: '/dbg ping' will turn pings on/off during game
            if (Debug::IsEnabled("ping")) {
                itsNet->SendPingCommand(2);
            }
        } else {
            // less frequent larger bursts seems to give better LT estimates when not playing
            // (this is better than sending 2 every 0.5 sec)
            itsNet->SendPingCommand(8);
        }
        nextPingTime = startTime + pingInterval;
    }

    // Not playing? Nothing to do!
    if (statusRequest != kPlayingStatus)
        return false;

    // Not time to process the next frame yet
    if (startTime < nextScheduledFrame)
        return false;

    // SDL_Log("CAvaraGame::GameTick frame=%d dt=%d start=%d end=%d\n", frameNumber, SDL_GetTicks() - lastFrameTime,
    // startTime, endTime); lastFrameTime = SDL_GetTicks();

    oldPlayersStanding = playersStanding;
    oldTeamsStanding = teamsStanding;

    didWait = false;
    longWait = false;
    veryLongWait = false;
    playersStanding = 0;
    teamsStanding = 0;
    teamsStandingMask = 0;

    ViewControl(); // This was called by itsApp->theGameWind->DoUpdate() calling RefreshWindow

    soundHub->HouseKeep();
    soundTime = soundHub->ReadTime();

    // SetPort(itsWindow);
    // SetPolyWorld(&itsPolyWorld);
    // TrackWindow();

    RunFrameActions();

    if (playersStanding == 1 && oldPlayersStanding > 1) {
        FlagMessage(iWin + firstVariable);
    }

    if (teamsStanding == 1 && oldTeamsStanding > 1) {
        FlagMessage(iWinTeam + firstVariable);
    }

    // do latency adjustement before frameNumber increments
    itsNet->AutoLatencyControl(frameNumber, longWait);

    // increment frameNumber, set nextScheduledFrame time
    IncrementFrame();

    timeInSeconds = frameNumber * frameTime / 1000;

    if (latencyTolerance)
        while (FramesFromNow(latencyTolerance) > topSentFrame)
            itsNet->FrameAction();

    canPreSend = true;

    // if the game hasn't kept up with the frame schedule, reset the next frame time (prevents chipmunk mode, unless player is dead)
    if (nextScheduledFrame < startTime && itsNet->IAmAlive()) {
        nextScheduledFrame = startTime + frameTime;
    }

    itsDepot->RunSliverActions();
    itsApp->StartFrame(frameNumber);
    ViewControl();

    if ((itsNet->activePlayersDistribution & itsNet->deadOrDonePlayers) == itsNet->activePlayersDistribution) {
        statusRequest = kWinStatus; //	Just a guess, really. StopGame will change this.
    }

    if (statusRequest != kPlayingStatus)
        GameStop();

    return true;
}

void CAvaraGame::ViewControl() {
    if(spectatePlayer != NULL && FindPlayerManager(spectatePlayer) != NULL) {
        FindPlayerManager(spectatePlayer)->ViewControl();
    }
    else {
        itsNet->ViewControl();
    }
}

bool CAvaraGame::canBeSpectated(CAbstractPlayer *player) {
    if(player == NULL)
        return false;

    if(player->isInLimbo == false) {
        return true;
    }

    if(player == GetLocalPlayer() && player->scoutView == true && player->lives == 0) {
        return true;
    }

    return false;
}

void CAvaraGame::SpectateNext() {
    if(spectatePlayer == NULL)
        spectatePlayer = GetLocalPlayer();

    CAbstractPlayer *nextPlayer = NULL;
    CAbstractPlayer *firstPlayer = NULL;
    CAbstractPlayer *currentPlayer = NULL;
    bool foundCurrentSpectate = false;
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        currentPlayer = itsNet->playerTable[i]->GetPlayer();
        if(currentPlayer != NULL) {
            if(firstPlayer == NULL && canBeSpectated(currentPlayer)) {
                firstPlayer = currentPlayer;
            }
            if(currentPlayer == spectatePlayer) {
                foundCurrentSpectate = true;
            }
            else if(foundCurrentSpectate == true && nextPlayer == NULL && canBeSpectated(currentPlayer)) {
                nextPlayer = currentPlayer;
            }
        }
    }

    spectatePlayer = nextPlayer;
    if(spectatePlayer == NULL)
        spectatePlayer = firstPlayer;
}

void CAvaraGame::SpectatePrevious() {
    if(spectatePlayer == NULL)
        spectatePlayer = GetLocalPlayer();

    CAbstractPlayer *previousPlayer = NULL;
    CAbstractPlayer *currentPlayer = NULL;
    bool found = false;
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        currentPlayer = itsNet->playerTable[i]->GetPlayer();
        if(currentPlayer != NULL) {
            if(currentPlayer == spectatePlayer) {
                if(previousPlayer != NULL) {
                    found = true;
                    spectatePlayer = previousPlayer;
                }
            }
            else if(canBeSpectated(currentPlayer)) {
                previousPlayer = currentPlayer;
            }
        }
    }

    if(!found && previousPlayer != NULL)
        spectatePlayer = previousPlayer;
}

CPlayerManager *CAvaraGame::FindPlayerManager(CAbstractPlayer *thePlayer) {
    for (int i = 0; i < kMaxAvaraPlayers; i++)
        if(itsNet->playerTable[i] != NULL && itsNet->playerTable[i]->GetPlayer() == thePlayer)
            return itsNet->playerTable[i];

    return NULL;
}

void CAvaraGame::StopGame() {
    soundHub->HushFlag(true);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_StartTextInput();
    // SDL_CaptureMouse(SDL_FALSE);
    // SDL_ShowCursor(SDL_ENABLE);
}

void CAvaraGame::UpdateViewRect(int width, int height, float pixelRatio) {
    itsView->SetViewRect(width, height, width / 2, height / 2);
    itsView->viewPixelRatio = pixelRatio;
    itsView->CalculateViewPyramidCorners();
}

void CAvaraGame::Render(NVGcontext *ctx) {
    worldShader->ShadeWorld(itsView);

    //if (gameStatus == kPlayingStatus || gameStatus == kPauseStatus || gameStatus == kWinStatus || gameStatus == kLoseStatus) {
    ViewControl();
    itsWorld->Render(itsView);
    AvaraGLSetAmbient(.7, UINT_MAX);
    AvaraGLSetDepthTest(false);
    hudWorld->Render(itsView);
    AvaraGLSetAmbient(ToFloat(itsView->ambientLight), itsView->ambientLightColor);
    hud->Render(itsView, ctx);
    AvaraGLSetDepthTest(true);
}

CPlayerManager *CAvaraGame::GetPlayerManager(CAbstractPlayer *thePlayer) {
    CPlayerManager *theManager = NULL;

    if (itsNet->playerCount < kMaxAvaraPlayers) {
        theManager = itsNet->playerTable[itsNet->playerCount];
        theManager->SetPlayer(thePlayer);
        itsNet->playerCount++;
    }

    return theManager;
}

// FrameLatency is slightly different than LatencyTolerance.  It is in terms of integer frames
// at the current frame rate.
long CAvaraGame::RoundTripToFrameLatency(long roundTrip) {
    // half of the roundTripTime in units of frameTime, rounded
    return (roundTrip + frameTime) / (2*frameTime);
}

// "frameLatency" is the integer number of frames to delay;
// latencyTolerance is the number of classic (64ms) frames (= frameLatency * fpsScale).
void CAvaraGame::SetFrameLatency(short newFrameLatency, short maxChange, CPlayerManager* slowPlayer) {
    double newLatency = newFrameLatency * fpsScale;
    if (latencyTolerance != newLatency) {
        #define MAX_LATENCY (8)   // in classic units
        if (maxChange < 0) {
            // allow latency to jump to any value
            maxChange = MAX_LATENCY;
        }

        double oldLatency = latencyTolerance;
        if (newLatency < latencyTolerance) {
            latencyTolerance = std::max(latencyTolerance-maxChange, std::max(newLatency, double(0.0)));
        } else {
            latencyTolerance = std::min(latencyTolerance+maxChange, std::min(newLatency, double(MAX_LATENCY)));
        }

        // make prettier version of the LT string (C++ sucks with strings)
        std::ostringstream ltOss;
        ltOss << std::fixed << std::setprecision(int(1/(2*fpsScale))) << latencyTolerance;

        // save as application preference (which also makes it show up on the UI)
        gApplication->Set(kLatencyToleranceTag, latencyTolerance);

        // if it changed
        if (latencyTolerance != oldLatency && statusRequest == kPlayingStatus) {
            std::ostringstream oss;
            std::time_t t = std::time(nullptr);
            oss << std::put_time(std::localtime(&t), "%H:%M:%S> LT set to ") << ltOss.str();
            if (slowPlayer != nullptr) {
                oss << " (" << slowPlayer->GetPlayerName() << ")";
            }
            itsApp->AddMessageLine(oss.str());
        }
    }
}

FrameNumber CAvaraGame::TimeToFrameCount(long timeInMsec) {
    // how many frames occur in timeInMsec?
    return FrameNumber(timeInMsec / frameTime);
}

FrameNumber CAvaraGame::NextFrameForPeriod(long period, long referenceFrame) {
    // Jump forward to the next full period.
    // For example, if we are changing latencies such that we start with 48 frames/period
    // and we're moving to 60 frames/period, and we're on frame 48, we want to
    // move forward to frame 120 and NOT frame 60.
    FrameNumber periodFrames = TimeToFrameCount(period);
    return periodFrames * ceil(double(referenceFrame + periodFrames) / periodFrames);
}

void CAvaraGame::SetFrameTime(int32_t ft) {
    if (ft != 2 && ft != 4 && ft != 8 && ft != 16 && ft != 32 && ft != 64) {
      SDL_Log("ERROR! frameTime MUST be 2, 4, 8, 16, 32 or 64 msec");
      exit(1); // is exit too dramatic?
    }
    SDL_Log("CAvaraGame::SetFrameTime(frameTime = %i)\n", ft);
    this->frameTime = ft;
    this->fpsScale = double(frameTime)/CLASSICFRAMETIME;
    if (gApplication) gApplication->Set(kFrameTimeTag, frameTime);
}

void CAvaraGame::IncrementFrame(bool firstFrame) {
    if (firstFrame) {
        frameNumber = 0;
        nextScheduledFrame = SDL_GetTicks(); // Run next frame immediately
    } else {
        frameNumber++;
        nextScheduledFrame += frameTime;
    }
    isClassicFrame = (frameNumber % (CLASSICFRAMETIME / frameTime) == 0);
}

FrameNumber CAvaraGame::FramesFromNow(FrameNumber classicFrameCount) {
    return frameNumber + classicFrameCount / fpsScale;
}
