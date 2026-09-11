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

#include "AssetManager.h"
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
//#include "Power.h"
#include "CScoreKeeper.h"
//#include "CRosterWindow.h"
//#include "Sound.h"
#include "Preferences.h"
#include "ARGBColor.h"
#include "Debug.h"
#include "AbstractRenderer.h"

void CAvaraGame::InitMixer(Boolean silentFlag) {
    CSoundMixer *aMixer;

    soundHub->MixerDispose();

    aMixer = new CSoundMixer(rate22khz, 64, 8, true, true, false, soundHub->AudioEnabled());
    aMixer->SetStereoSeparation(true);
    aMixer->SetSoundEnvironment(FIX(400), FIX(5), CLASSICFRAMETIME);
    aMixer->SetVolume(gApplication ? gApplication->Get<uint8_t>(kSoundVolume) : 0);
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

void CAvaraGame::IncrementGameCounter() {
    extern Fixed FRandSeed;
    currentGameId = int(FRandSeed);
}

std::unique_ptr<CNetManager> CAvaraGame::CreateNetManager() {
    return std::make_unique<CNetManager>(this);
}

CAvaraGame::CAvaraGame(FrameTime frameTime) {
    SetFrameTime(frameTime);
    latencyTolerance = 0.0;
}
void CAvaraGame::IAvaraGame(CAvaraApp *theApp) {
    itsApp = theApp;

    itsNet = CreateNetManager();
    itsNet->InitializePlayers();
    itsApp->SetNet(itsNet.get());

    searchCount = 0;
    locatorTable = (ActorLocator **)NewPtr(sizeof(ActorLocator *) * LOCATORTABLESIZE);
    InitLocatorTable();

    gameStatus = kAbortStatus;
    actorList = NULL;
    freshPlayerList = NULL;
    playerList = NULL;
    spectatePlayer = NULL;
    freeCamState = 0;

    soundSwitches = 0xFF;

    // CalcGameRect();

    soundHub = CreateSoundHub();
    gHub = soundHub;

    InitMixer(true);

    scoreKeeper = new CScoreKeeper(this);

    itsDepot = new CDepot(this);

    // mapRes = GetResource(FUNMAPTYPE, FUNMAPID);

    // IGameTimer(frameTime/TIMING_GRAIN);

    allowBackgroundProcessing = false;

    loadedTimeLimit = 600;
    timeInSeconds = 0;
    simpleExplosions = false;
    keysFromStdin = false;
    keysToStdout = false;

    statusRequest = kNoVehicleStatus;

    nextPingTime = 0;
    nextLoadTime = 0;

    showNewHUD = gApplication ? gApplication->Get<bool>(kShowNewHUD) : false;
    hudLayout = gApplication ? gApplication->Get<int>(kHUDPreset) : 2;
    // CalcGameRect();

    // vg = AvaraVGContext();
    // font = nvgCreateFont(vg, "sans", BundlePath("fonts/Roboto-Regular.ttf"));
}

CSoundHub* CAvaraGame::CreateSoundHub() {
    return new CSoundHubImpl(32, 32);
}

CAvaraGame::~CAvaraGame() {
    CAbstractActor *nextActor;

    spectatePlayer = NULL;
    LevelReset(false);

    delete scoreKeeper;
    delete itsDepot;

    while (actorList) {
        nextActor = actorList->nextActor;
        delete actorList;
        actorList = nextActor;
    }

    while (freshPlayerList) {
        nextActor = freshPlayerList->nextActor;
        delete freshPlayerList;
        freshPlayerList = (CAbstractPlayer *)nextActor;
    }

    if (soundHub)
        delete soundHub;

    DisposePtr((Ptr)locatorTable);
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
        CPlayerManager *mgr = itsNet->playerTable[i].get();
        if (mgr && mgr->IsLocalPlayer()) {
            return mgr->GetPlayer();
        }
    }

    return NULL;
}

std::string CAvaraGame::GetPlayerName(short id) {
    if (id != -1) {
        CPlayerManager *mgr = itsNet->playerTable[id].get();
        return mgr->GetPlayerName();
    }
    return "";
}

void CAvaraGame::AddActor(CAbstractActor *theActor) {
    short i;

    theActor->isInGame = true;
    theActor->nextActor = actorList;
    actorList = theActor;

    for (i = 0; i < theActor->partCount; i++) {
        gRenderer->AddPart(theActor->partList[i]);
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
                    gRenderer->RemovePart(anActor->partList[i]);
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

// extent of all visible actors, this drives the level previews and random incarnators
void CAvaraGame::CalculateExtent() {
    int foundLocations = 0;
    extentMin[0] = extentMin[1] = extentMin[2] = std::numeric_limits<Fixed>::max();
    extentMax[0] = extentMax[1] = extentMax[2] = std::numeric_limits<Fixed>::min();

    // get the extent of all the CPlacedActors
    for (CAbstractActor* actor = actorList;
         actor != nullptr;
         actor = actor->nextActor) {
        // check CPlacedActors that set UseForExtent() which includes incarnators, goodies, ramps
        CPlacedActors* placedActor = dynamic_cast<CPlacedActors*>(actor);
        if (placedActor != nullptr && placedActor->UseForExtent()) {
            extentMin[0] = std::min(extentMin[0], placedActor->location[0]);
            extentMin[1] = std::min(extentMin[1], placedActor->location[1]);
            extentMin[2] = std::min(extentMin[2], placedActor->location[2]);
            extentMax[0] = std::max(extentMax[0], placedActor->location[0]);
            extentMax[1] = std::max(extentMax[1], placedActor->location[1]);
            extentMax[2] = std::max(extentMax[2], placedActor->location[2]);
            foundLocations++;
        }
    }

    // use "reasonable" defaults if there aren't enough actor locations to calculate the extent
    const Fixed DEFAULT_OFFSET = FIX(9);
    if (foundLocations == 0) {
        extentMin[0] = extentMin[2] = -DEFAULT_OFFSET;
        extentMax[0] = extentMax[2] = +DEFAULT_OFFSET;
        extentMin[1] = 0;
        extentMax[1] = DEFAULT_OFFSET;
    } else if (foundLocations == 1) {
        extentMin[0] -= DEFAULT_OFFSET; extentMin[1] -= 0;              extentMin[0] -= DEFAULT_OFFSET;
        extentMax[0] += DEFAULT_OFFSET; extentMax[1] += DEFAULT_OFFSET; extentMax[0] += DEFAULT_OFFSET;
    }

    extentCenter[0] = (extentMax[0] + extentMin[0]) / 2;
    extentCenter[1] = (extentMax[1] + extentMin[1]) / 2;
    extentCenter[2] = (extentMax[2] + extentMin[2]) / 2;
    extentRadius = std::max(extentMax[0] - extentMin[0], extentMax[2] - extentMin[2]) / 2;
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
        points = LMul(points, friendlyHitMultiplier);
    }

    if (player >= 0 && player < kMaxAvaraPlayers) {
        scores[player] += points;
    }

    scoreKeeper->Score(scoreReason, team, player, points, energy, hitTeam, hitPlayer);
}

void CAvaraGame::AddScoreNotify(ScoreInterfaceEvent event) {
    event.frameNumber = frameNumber;
    event.gameId = currentGameId;
    scoreEventList.push_back(event);
    if (scoreEventList.size() > 20) {
        scoreEventList.pop_front();
    }
}

void CAvaraGame::RunFrameActions() {
    // SDL_Log("CAvaraGame::RunFrameActions\n");
    CAbstractPlayer *thePlayer;

    itsDepot->FrameAction();

    postMortemList = NULL;

    RunActorFrameActions();

    itsNet->ProcessQueue();

    while (topSentFrame <= FramesFromNow(latencyTolerance)) {
        itsNet->FrameAction();   // increments topSentFrame while sending frame packet
    }

    thePlayer = playerList;
    while (thePlayer) {
        nextPlayer = thePlayer->nextPlayer;
        thePlayer->PlayerAction();
        thePlayer = nextPlayer;
    }

    // Time out old score events
    if (!scoreEventList.empty()) {
        ScoreInterfaceEvent event = scoreEventList.front();
        if (event.frameNumber < FramesFromNow(-120) || event.gameId != currentGameId) {
            scoreEventList.pop_front();
        }
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

void CAvaraGame::PreSendFrameActions() {
    // When called, send up to preSendCount frames early based on whether the clock is "behind".
    // "behind" is defined as our current time being beyond the projected time of topSentFrame minus an offset.
    // This allows us to increase the effective LT by as many as FRAME_OFFSET frames during a wait loop.
    const float FRAME_OFFSET = 2.0;
    const int MAX_DYNAMIC_LT = 4;  // don't send anything extra above this LT
    while (preSendCount > 0 &&
           topSentFrame - frameNumber < MAX_DYNAMIC_LT / fpsScale &&
           SDL_GetTicks() >= nextScheduledFrame + frameTime*(topSentFrame-frameNumber-FRAME_OFFSET)) {
        itsNet->FrameAction();
        DBG_Log("presend", "fn=%d, preSent frame #%d, N=%d, ahead=%d, start=%d time=%d nSF=%d\n", frameNumber, topSentFrame, preSendCount, topSentFrame - frameNumber, frameStart, SDL_GetTicks(), nextScheduledFrame);
        preSendCount--;
    }
}

void CAvaraGame::ChangeDirectoryFile() {
    // No-op.
}

void CAvaraGame::LevelReset(Boolean clearReset) {
    short i;
    CAbstractActor *anActor, *nextActor;

    gameStatus = kAbortStatus;

    loadedTimeLimit = 600;
    timeInSeconds = 0;

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
        delete freshPlayerList;
        freshPlayerList = (CAbstractPlayer *)nextActor;
    }

    gRenderer->LevelReset();

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
    Fixed intensity, angle1, angle2, celestialRadius;
    bool applySpecular;
    short mode;
    auto vp = gRenderer->viewParams;

    gameStatus = kReadyStatus;
    // SetPolyWorld(&itsPolyWorld);

    vp->ambientLight = ReadFixedVar(iAmbient);
    vp->ambientLightColor = ARGBColor::Parse(ReadStringVar(iAmbientColor))
        .value_or(DEFAULT_LIGHT_COLOR);

    for (i = 0; i < 4; i++) {
        intensity = ReadFixedVar(iLightsTable + 6 * i);
        angle1 = ReadFixedVar(iLightsTable + 1 + 6 * i);
        angle2 = ReadFixedVar(iLightsTable + 2 + 6 * i);
        color = ARGBColor::Parse(ReadStringVar(iLightsTable + 3 + 6 * i))
            .value_or(DEFAULT_LIGHT_COLOR);
        celestialRadius = ReadFixedVar(iLightsTable + 4 + 6 * i);
        applySpecular = ReadLongVar(iLightsTable + 5 + 6 * i);
        mode = (intensity >= 2048) ? kLightGlobalCoordinates : kLightOff;

        // SDL_Log("Light from light table - idx: %d i: %f a: %f b: %f c: %x",
        //        i, ToFloat(intensity), ToFloat(angle1), ToFloat(angle2), color);

        //The b angle is the compass reading and the a angle is the angle from the horizon.
        vp->SetLight(i, angle1, angle2, intensity, color, celestialRadius, applySpecular, mode);
    }
    gRenderer->ApplyLights();
    gRenderer->ApplySky();

    color = *ARGBColor::Parse(ReadStringVar(iMissileArmedColor));
    ColorManager::setMissileArmedColor(color);
    color = *ARGBColor::Parse(ReadStringVar(iMissileLaunchedColor));
    ColorManager::setMissileLaunchedColor(color);

    friendlyHitMultiplier = ReadFixedVar(iFriendlyHitMultiplier);

    loadedLevelInfo->designer    = ReadStringVar(iDesignerName);
    loadedLevelInfo->information = ReadStringVar(iLevelInformation);

    loadedTimeLimit = ReadLongVar(iTimeLimit);

    groundTraction = ReadFixedVar(iDefaultTraction);
    groundFriction = ReadFixedVar(iDefaultFriction);
    gravityRatio = ReadFixedVar(iGravity);
    groundStepSound = ReadLongVar(iGroundStepSound);

    // Preload sounds.
    auto _ = AssetManager::GetOgg(groundStepSound);

    itsDepot->EndScript();

    scoreKeeper->EndScript();
    
    gRenderer->PostLevelLoad();
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
    // if the server is not playing, we want initial LT to be calculated by a player who IS playing
    int firstReady = kMaxAvaraPlayers;
    bool allReady = true;
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        CPlayerManager *mgr = itsNet->playerTable[i].get();
        if (mgr) {
            if (mgr->IsLoaded() && mgr->Presence() == kzAvailable) {
                allReady = false;
                break;
            } else if (mgr->IsReady() && i < firstReady) {
                firstReady = i;
            }
        }
    }
    if (allReady && firstReady == itsNet->itsCommManager->myId) {
        // to avoid race conditions, only the first active player sends the start request
        SendStartCommand();
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
    //SDL_Log("mouse sensitivity multiplier = %.2lf\n", sensitivity);
    showNewHUD = gApplication->Get<bool>(kShowNewHUD);
    hudLayout = gApplication->Get<int>(kHUDPreset);
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
            itsNet->AttachPlayers((CAbstractPlayer *)freshPlayerList);
            itsApp->GameStarted(*loadedLevelInfo);
            freshPlayerList = NULL;
            InitMixer(false);
        } else {
            itsApp->MessageLine(kmRestarted, MsgAlignment::Center);
            soundHub->MuteFlag(false);
        }

        // TODO: syncing start dialog
        scoreKeeper->StartResume(gameStatus == kReadyStatus);

        // init time-based stat vars
        lastFrameTime = SDL_GetTicks() - frameTime;
        lastFramePackets = itsNet->itsCommManager->TotalPacketsSent();
        nextStatTime = SDL_GetTicks();
        SDL_Log("lastFrameTime = %ld, lastFramePackets = %u\n", lastFrameTime, uint32_t(lastFramePackets));

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
        // init stat vars
        msecPerFrame = frameTime;
        packetsPerFrame = 1.0;
        effectiveLT = latencyTolerance + 1;
    }

    playersStanding = 0;
    teamsStanding = 0;
    nextScheduledFrame = SDL_GetTicks(); // Run next frame immediately

    // frameAdvance = INIT_ADVANCE;
    // frameCredit = frameAdvance << 16;
    preSendCount = 0;

    // The difference between the last frame's time and frameTime
    frameAdjust = 0;

    while (FramesFromNow(latencyTolerance) > topSentFrame) {
        itsNet->FrameAction();
    }

    // SDL_ShowCursor(SDL_DISABLE);
    // SDL_CaptureMouse(SDL_TRUE);

    SDL_SetRelativeMouseMode(SDL_TRUE);
#ifdef WIN32
#else
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1");
#endif
    // HideCursor();
    // FlushEvents(everyEvent, 0);

    // change the event processing time during the game (0 = poll)
#ifdef _WIN32
    nanogui::throttle = 0;   // let 'er rip
#else
    FrameTime throttle = static_cast<FrameTime>(itsApp->Number(kThrottle));
    if (Debug::IsEnabled("cpu")) {
        // when measuring "cpu", disable throttle so it doesn't thow off the cpu usage calculation
        throttle = 0;
    }
    nanogui::throttle = std::min(throttle, frameTime);
#endif
    SDL_Log("CAvaraGame::GameStart, throttle = %d\n", nanogui::throttle);
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

    // event wait timeout used by mainloop()
    nanogui::throttle = INACTIVE_LOOP_REFRESH;
    SDL_Log("CAvaraGame::GameStop throttle = %d, frameNumber = %d\n", nanogui::throttle, frameNumber);
}

void CAvaraGame::HandleEvent(SDL_Event &event) {
    // Queue any keyboard/mouse events for the next frame.
    if (IsPlaying())
        itsNet->HandleEvent(event);
}

bool CAvaraGame::GameTick() {
    frameStart = SDL_GetTicks();

    // No matter what, process any pending network packets
    itsNet->ProcessQueue();

    if (frameStart > nextPingTime) {
        // 3 pings every second, 1 ping used by each client for RTT calc (last ping not used)
        static uint32_t pingInterval = 1000; // msec
        itsNet->SendPingCommand(4);
        nextPingTime = frameStart + pingInterval;
    }

    // Not playing? Nothing to do!
    if (statusRequest != kPlayingStatus)
        return false;

    // Not time to process the next frame yet
    if (frameStart < nextScheduledFrame)
        return false;

    // SDL_Log("CAvaraGame::GameTick frame=%d dt=%d start=%d end=%d\n", frameNumber, SDL_GetTicks() - lastFrameTime,
    // frameStart, endTime); lastFrameTime = SDL_GetTicks();

    if (Debug::IsEnabled("stats")) {
        DoStats(SDL_GetTicks(), Debug::GetValue("stats"));
    }

    oldPlayersStanding = playersStanding;
    oldTeamsStanding = teamsStanding;

    didWait = false;
    longWait = false;
    veryLongWait = false;
    playersStanding = 0;
    teamsStanding = 0;
    teamsStandingMask = 0;
    preSendCount = 2;

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

    // if the game hasn't kept up with the frame schedule, reset the next frame time (prevents chipmunk mode, unless player is dead)
    if (nextScheduledFrame < frameStart && itsNet->IAmAlive()) {
        // at the start of this frame we were ALREADY a full frame or more behind...
        // the further back we can stay, the closer we are to original frame rate, the better it is for
        // smoothness.  But that has to be weighed against micro-jitter.  Ideally we want to minimze the
        // percentage of time the frame boundary is adjusted because that is perceived as jitter.  That
        // is traded off with reducing overall wait time.  Sometimes it's better to wait longer if we
        // have fewer interruptions.
        uint32_t prevNSF = nextScheduledFrame;
        nextScheduledFrame = frameStart + 0.25*frameTime;
        DBG_Log("presend", "fn=%d, frame reset %u --> %u = +%d\n", frameNumber, prevNSF, nextScheduledFrame, nextScheduledFrame - prevNSF);
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

void CAvaraGame::ToggleFreeCam(Boolean state) {
    freeCamState = state;
}

void CAvaraGame::ViewControl() {
    if(spectatePlayer != NULL && FindPlayerManager(spectatePlayer) != NULL) {
        if (freeCamState) {
            // The player has control of the camera when in free cam mode while spectating
            itsNet->ViewControl();
        } else {
            FindPlayerManager(spectatePlayer)->ViewControl();
        }
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
            return itsNet->playerTable[i].get();

    return nullptr;
}

void CAvaraGame::StopGame() {
    soundHub->HushFlag(true);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    SDL_StartTextInput();
    // SDL_CaptureMouse(SDL_FALSE);
    // SDL_ShowCursor(SDL_ENABLE);
}

void CAvaraGame::Render() {
    //if (gameStatus == kPlayingStatus || gameStatus == kPauseStatus || gameStatus == kWinStatus || gameStatus == kLoseStatus) {
    ViewControl();
    gRenderer->RenderFrame();
}

CPlayerManager *CAvaraGame::GetPlayerManager(CAbstractPlayer *thePlayer) {
    CPlayerManager *theManager = nullptr;

    if (itsNet->playerCount < kMaxAvaraPlayers) {
        theManager = itsNet->playerTable[itsNet->playerCount].get();
        theManager->SetPlayer(thePlayer);
        itsNet->playerCount++;
    }

    return theManager;
}

// FrameLatency is slightly different than LatencyTolerance.  It is in terms of integer frames
// at the current frame rate.
short CAvaraGame::RoundTripToFrameLatency(long roundTrip) {
    // half of the roundTripTime in units of frameTime, rounded up (ceil)
    return std::max(0.0, std::ceil(roundTrip/2.0/frameTime) - 1);
}

// "frameLatency" is the integer number of frames to delay;
// latencyTolerance is the number of classic (64ms) frames (= frameLatency * fpsScale).
void CAvaraGame::SetFrameLatency(short newFrameLatency, CPlayerManager* slowPlayer) {
    double newLatency = newFrameLatency * fpsScale;
    if (latencyTolerance != newLatency) {
        static const double MAX_LATENCY = 8.0;

        double oldLatency = latencyTolerance;
        latencyTolerance = newLatency;

        // make sure it's always between 0 and MAX_LATENCY
        latencyTolerance = std::min(std::max(latencyTolerance, 0.0), MAX_LATENCY);

        // make prettier version of the LT string (C++ sucks with strings)
        std::ostringstream ltOss;
        ltOss << std::fixed << std::setprecision(int(1/(2*fpsScale))) << latencyTolerance;

        // if it changed
        if (latencyTolerance != oldLatency && statusRequest == kPlayingStatus) {
            std::ostringstream oss;
            int gameSeconds = frameNumber * frameTime / 1000;
            int gameMinutes = gameSeconds / 60;
            gameSeconds = gameSeconds % 60;
            oss << "T+" << std::setfill('0') << std::setw(2) << gameMinutes << ":" << std::setw(2) << gameSeconds << "> LT set to " << ltOss.str();
            if (slowPlayer != nullptr) {
                oss << " (" << slowPlayer->GetPlayerName() << ")";
            }
            itsApp->AddMessageLine(oss.str());
        }
    }
}

short CAvaraGame::FrameLatency(void) {
    // example for 16ms frame: 1.75 LT --> 7 frames
    return latencyTolerance / fpsScale;
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

FrameNumber CAvaraGame::FramesFromNow(double classicFrameCount) {
    return frameNumber + classicFrameCount / fpsScale;
}

void CAvaraGame::SetSpawnOrder(SpawnOrder order) {
    std::string types[] = {"Random", "Usage", "Distance", "Hybrid"};
    spawnOrder = SpawnOrder(order % ksNumSpawnOrders); // guard bad inputs
    std::ostringstream oss;
    SDL_Log("spawnOrder = %d [%s]\n", spawnOrder, types[spawnOrder].c_str());
    if (gApplication) {
        gApplication->Set(kSpawnOrder, spawnOrder);
    }
}

void CAvaraGame::DoStats(uint32_t startTime, int interval) {
    static float ALPHA = 0.01;

    if (interval <= 0) {
        interval = 5000;
    } else if (interval < 100) {
        // assume seconds if a small number
        interval *= 1000;
    }

    float deltaT = (startTime - lastFrameTime);
    msecPerFrame = msecPerFrame*(1-ALPHA) + ALPHA*deltaT;

    RolloverCounter<uint32_t> curPackets = itsNet->itsCommManager->TotalPacketsSent();
    packetsPerFrame = packetsPerFrame*(1-ALPHA) + ALPHA*(curPackets - lastFramePackets);

    effectiveLT = effectiveLT*(1-ALPHA) + ALPHA*(fpsScale * (topSentFrame - frameNumber));

    lastFrameTime = startTime;
    lastFramePackets = curPackets;

    if (startTime > nextStatTime) {
        char statBuf[64];
        std::snprintf(statBuf, sizeof(statBuf), "fps=%.1lf ppf=%.1lf rtt=%d lt=%.2lf",
                      1000.0 / msecPerFrame,
                      packetsPerFrame,
                      (int)itsNet->itsCommManager->GetMaxRoundTrip(itsNet->activePlayersDistribution),
                      effectiveLT);
        itsApp->AddMessageLine(statBuf);

        nextStatTime = startTime + interval;
    }
}

ARGBColor CAvaraGame::GetLocalTeamColor() {
    return GetLocalPlayer()->GetTeamColorOr(ColorManager::getDefaultTeamColor());
}
