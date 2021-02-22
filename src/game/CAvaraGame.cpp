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
//#include "CCapMaster.h"
#include "AvaraDefines.h"
#include "CCompactTagBase.h"
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

#define kHighShadeCount 12

static Fixed menuRates[] = {-1, //	index from 1
    0, //	system auto
    -1, //	dividing line
    rate11025hz,
    rate22050hz,
    rate44khz,
    -1,
    rate11khz,
    rate22khz};

void CAvaraGame::InitMixer(Boolean silentFlag) {
    CSoundMixer *aMixer;
    Fixed theRate;
    short maxChan;

    soundHub->MixerDispose();

    aMixer = new CSoundMixer;
    aMixer->ISoundMixer(rate22khz, 32, 4, true, true, false);
    aMixer->SetStereoSeparation(true);
    aMixer->SetSoundEnvironment(FIX(400), FIX(5), frameTime);
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

CAvaraGame::CAvaraGame(int frameTime) {
    this->frameTime = frameTime; // milliseconds
    this->latencyFrameTime = frameTime;
}
void CAvaraGame::IAvaraGame(CAvaraApp *theApp) {
    short i;


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

    loadedTag = 0;
    loadedDirectory = 0;
    loadedLevel[0] = 0;
    loadedDesigner[0] = 0;
    loadedInfo[0] = 0;
    loadedTimeLimit = 600;
    timeInSeconds = 0;
    simpleExplosions = false;

    statusRequest = -1; // who decided to make "playing" 0??

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
    short i;
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
    CAbstractPlayer *thePlayer;

    thePlayer = playerList;
    while (thePlayer) {
        if (thePlayer->itsManager && thePlayer->itsManager->IsLocalPlayer()) {
            break;
        }

        thePlayer = thePlayer->nextPlayer;
    }

    return thePlayer;
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
    CAbstractActor *theActor;
    CAbstractPlayer *thePlayer;

    itsDepot->FrameAction();

    postMortemList = NULL;
    theActor = actorList;

    while (theActor) {
        nextActor = theActor->nextActor;
        if (theActor->isActive || --(theActor->sleepTimer) == 0) {
            theActor->FrameAction();
            // itsNet->ProcessQueue();
            /*
            if(canPreSend && SDL_GetTicks() - nextScheduledFrame >= 0)
            {	canPreSend = false;
                itsNet->FrameAction();
            }
            */
        }

        theActor = nextActor;
    }

    itsNet->ProcessQueue();
    if (!latencyTolerance)
        while (frameNumber > topSentFrame)
            itsNet->FrameAction();

    thePlayer = playerList;
    while (thePlayer) { // itsNet->ProcessQueue();
        nextPlayer = thePlayer->nextPlayer;
        thePlayer->PlayerAction();
        thePlayer = nextPlayer;
    }
}

void CAvaraGame::ChangeDirectoryFile() {
    gHub->FreeUnusedSamples();
}

void CAvaraGame::LevelReset(Boolean clearReset) {
    short i;
    CAbstractActor *anActor, *nextActor;
    Size growBytes;
    Size freeBytes;

    gameStatus = kAbortStatus;

    gHub->FlagOldSamples();

    loadedTag = 0;
    loadedDirectory = 0;
    loadedLevel[0] = 0;
    loadedDesigner[0] = 0;
    loadedInfo[0] = 0;
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
    frameNumber = 0;
    topSentFrame = -1;

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
    Fixed intensity, angle1, angle2;
    Fixed x, y, z;

    gameStatus = kReadyStatus;
    // SetPolyWorld(&itsPolyWorld);
    worldShader->Apply();

    itsView->ambientLight = ReadFixedVar(iAmbient);
    AvaraGLSetAmbient(ToFloat(ReadFixedVar(iAmbient)));

    for (i = 0; i < 4; i++) {
        intensity = ReadFixedVar(iLightsTable + 3 * i);

        if (intensity >= 2048) {
            angle1 = ReadFixedVar(iLightsTable + 1 + 3 * i);
            angle2 = ReadFixedVar(iLightsTable + 2 + 3 * i);

            x = FMul(FDegCos(angle1), intensity);
            y = FMul(FDegSin(-angle1), intensity);
            z = FMul(FDegCos(angle2), x);
            x = FMul(FDegSin(-angle2), x);

            itsView->SetLightValues(i, x, y, z, kLightGlobalCoordinates);
            SDL_Log("Light from light table - idx: %d i: %f a: %f b: %f",
                    i, ToFloat(intensity), ToFloat(angle1), ToFloat(angle2));

            //The b angle is the compass reading and the a angle is the angle from the horizon.
            AvaraGLSetLight(i, ToFloat(intensity), ToFloat(angle1), ToFloat(angle2));
        } else {
            itsView->SetLightValues(i, 0, 0, 0, kLightOff);
            AvaraGLSetLight(i, 0, 0, 0);
        }
    }

    friendlyHitMultiplier = ReadFixedVar(iFriendlyHitMultiplier);

    ReadStringVar(iDesignerName, loadedDesigner);
    ReadStringVar(iLevelInformation, loadedInfo);
    loadedTimeLimit = ReadLongVar(iTimeLimit);

    groundTraction = ReadFixedVar(iDefaultTraction);
    groundFriction = ReadFixedVar(iDefaultFriction);
    gravityRatio = ReadFixedVar(iGravity) * FrameTimeScale(2);
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
        itsNet->SendStartCommand();
    } else if (gameStatus == kPauseStatus) {
        itsNet->SendResumeCommand();
    }
}

#define MAXSKIPSINAROW 2

static Boolean takeShot = false;

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
    sensitivity = itsApp->Number(kMouseSensitivityTag);
    latencyTolerance = gApplication->Number(kLatencyToleranceTag);
    AdjustFrameTime();
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
        Byte oldCursorScale;
        short resRef;
        long powerGestalt;
        short oldEventMask;

        if (freshMission) {
            itsApp->GameStarted(std::string((char *)loadedSet),
                                std::string((char *)loadedLevel + 1, loadedLevel[0]));
            itsNet->AttachPlayers((CAbstractPlayer *)freshPlayerList);
            freshPlayerList = NULL;
            InitMixer(false);
        } else {
            itsApp->MessageLine(kmRestarted, centerAlign);
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
    // consecutiveSkips = 0;
    latencyTolerance = 0;
    didWait = false;
    longWait = false;

    topSentFrame = frameNumber - 1;

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

    while (frameNumber + latencyTolerance > topSentFrame) {
        itsNet->FrameAction();
    }

    // SDL_ShowCursor(SDL_DISABLE);
    // SDL_CaptureMouse(SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1");

    // HideCursor();
    // FlushEvents(everyEvent, 0);
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
        itsApp->MessageLine(kmAborted, centerAlign);
    } else if (gameStatus == kPauseStatus) {
        itsApp->ParamLine(kmPaused, centerAlign, itsNet->playerTable[pausePlayer]->PlayerName(), NULL);
    }

    scoreKeeper->StopPause(gameStatus == kPauseStatus);

    itsNet->UngatherPlayers();
}

void CAvaraGame::HandleEvent(SDL_Event &event) {
    // Queue any keyboard/mouse events for the next frame.
    if (IsPlaying())
        itsNet->HandleEvent(event);
}

bool CAvaraGame::GameTick() {
    int32_t startTime = SDL_GetTicks();

    // No matter what, process any pending network packets
    itsNet->ProcessQueue();

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

    frameNumber++;

    timeInSeconds = FMulDivNZ(frameNumber, frameTime, 1000);

    itsNet->AutoLatencyControl(frameNumber, longWait);

    // SDL_Log("latencyTolerance = %ld, latencyFrameTime = %ld\n", latencyTolerance, latencyFrameTime);
    if (latencyTolerance)
        while (frameNumber + latencyTolerance > topSentFrame)
            itsNet->FrameAction();

    canPreSend = true;

    nextScheduledFrame += latencyFrameTime;

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

    if (true || gameStatus == kPlayingStatus || gameStatus == kPauseStatus || gameStatus == kWinStatus ||
        gameStatus == kLoseStatus) {
        ViewControl();
        itsWorld->Render(itsView);

        AvaraGLSetDepthTest(false);
        hudWorld->Render(itsView);
        hud->Render(itsView, ctx);
        AvaraGLSetDepthTest(true);
    }
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

double CAvaraGame::FrameTimeScale(double exponent) {
    return pow(double(frameTime)/CLASSICFRAMETIME, exponent);
}

double CAvaraGame::LatencyFrameTimeScale() {
    return double(latencyFrameTime)/frameTime;
}


long CAvaraGame::RoundTripToFrameLatency(long roundTrip) {
    // half of the roundTripTime in units of frameTime, rounded
    SDL_Log("CAvaraGame::RoundTripToFrameLatency roundTrip=%ld, LT(unrounded)=%.2lf\n", roundTrip, (roundTrip) / (2.0*frameTime));
    return (roundTrip + frameTime) / (2*frameTime);
}

void CAvaraGame::SetLatencyTolerance(long newLatency, int maxChange, const char* slowPlayer) {
    if (latencyTolerance != newLatency) {
        #define MAX_LATENCY ((long)8)
        if (maxChange < 0) {
            // allow latency to jump to any value
            maxChange = MAX_LATENCY;
        }
        if (newLatency < latencyTolerance) {
            latencyTolerance = std::max(latencyTolerance-maxChange, std::max(newLatency, (long)0));
        } else {
            latencyTolerance = std::min(latencyTolerance+maxChange, std::min(newLatency, MAX_LATENCY));
        }
        gApplication->Set(kLatencyToleranceTag, latencyTolerance);
        SDL_Log("*** LT set to %ld\n", latencyTolerance);

        if (slowPlayer != nullptr) {
            std::ostringstream oss;
            std::time_t t = std::time(nullptr);
            oss << std::put_time(std::localtime(&t), "%H:%M:%S> LT set to ") << std::to_string(latencyTolerance) << " (" << slowPlayer << ")";
            itsApp->AddMessageLine(oss.str());
        }

        AdjustFrameTime();
    }
}

void CAvaraGame::AdjustFrameTime() {
    // Why this adjustment?  We want it to be a somewhat parabolic function because of the
    // N^2 nature of the number of messages sent for the game... (players*(players-1)).
    // So, theoretically, the game will tend to slow down as a square of the number of players.
    // In choosing the adjustment you have to consider playability (low latency) vs
    // recoverability (reduced message sending).
    // If it errs too much on the side of playability then it may not recover from big latency spikes.
    // If it errs too much on the side of recoverability, then it may not be playable for moderate latencies.

    // The frameTimeMultipliers below were chosen to keep LT=5 at a barely playable level.
    // The numbers at the higher levels are not intended to be very playable but to keep the game chugging
    // along at a lower message rate to give it a chance to catch up and, hopefully, come back down below LT=5.

    // Here is how the different latencies affect playability and recoverability:
    static float frameTimeMultiplier[MAX_LATENCY+1] = {
               //  LT frameTime(ms) latency(ms)  messages/sec (approx)
        1.00,  //  0  64            0            16   <-- most playable (LAN)
        1.00,  //  1  64            64           16   <-- good internet
        1.00,  //  2  64            128          16
        1.00,  //  3  64            192          16
        1.00,  //  4  64            256          16   <-- somewhat playable, laggy
        1.25,  //  5  80            400          13   <-- barely playable / begin recovering
        1.50,  //  6  96            576          10   <-- hope to never see LT > 5
        2.50,  //  7  160           1120         6
        3.75,  //  8  240           1920         4    <-- trying to recover!  If it were a horse, I'd shoot it.
    };

    latencyFrameTime = long(frameTime * frameTimeMultiplier[latencyTolerance]);
    SDL_Log("*** latencyFrameTime = %ld\n", latencyFrameTime);
}


long CAvaraGame::TimeToFrameCount(long timeInMsec) {
    // how many frames occur in timeInMsec?
    return timeInMsec / latencyFrameTime;
}

long CAvaraGame::NextFrameForPeriod(long period, long referenceFrame) {
    // Jump forward to the next full period.
    // For example, if we are changing latencies such that we start with 48 frames/period
    // and we're moving to 60 frames/period, and we're on frame 48, we want to
    // move forward to frame 120 and NOT frame 60.
    long periodFrames = TimeToFrameCount(period);
    return periodFrames * ceil(double(referenceFrame + periodFrames) / periodFrames);
}
