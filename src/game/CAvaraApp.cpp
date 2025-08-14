/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CAvaraApp.c
    Created: Wednesday, November 16, 1994, 01:26
    Modified: Friday, September 20, 1996, 02:05
*/

#define MAINAVARAAPP

#include "CAvaraApp.h"

#include "AssetManager.h"
#include "AvaraScoreInterface.h"
#include "AvaraTCP.h"
#include "Beeper.h"
#include "CAvaraGame.h"
#include "CBSPWorld.h"
#include "CNetManager.h"
#include "CNetManager.h" // version.net
#include "CRC.h"
#include "CRUDsqlite.h"
#include "CSoundMixer.h"
#include "CViewParameters.h"
#include "CharWordLongPointer.h"
#include "ColorManager.h"
#include "CommandList.h"
#include "Debug.h"
#include "GitVersion.h" // version.git
#include "InfoMessages.h"
#include "KeyFuncs.h"
#include "LegacyOpenGLRenderer.h"
#include "LevelLoader.h"
#include "Messages.h"
#include "ModernOpenGLRenderer.h"
#include "Parser.h"
#include "Preferences.h"
#include "System.h"
#include "Tags.h"
#include "httplib.h"

#include <chrono>
#include <cmath>
#include <json.hpp>

// included while we fake things out
#include "CPlayerManager.h"

std::mutex trackerLock;

void TrackerPinger(CAvaraAppImpl *app) {
    while (true) {
        if (app->Number(kTrackerRegister) == 1 && app->GetNet()->netStatus == kServerNet) {
            std::string payload = app->TrackerPayload();
            if (payload.size() > 0) {
                // Probably not thread-safe.
                std::string address = app->String(kTrackerRegisterAddress);
                DBG_Log("tracker", "Pinging %s", address.c_str());
                size_t sepIndex = address.find(":");
                if (sepIndex != std::string::npos) {
                    std::string host = address.substr(0, sepIndex);
                    int port = std::stoi(address.substr(sepIndex + 1));
                    httplib::Client client(host.c_str(), port);
                    client.Post("/api/v1/games/", payload, "application/json");
                } else {
                    httplib::Client client(address.c_str(), 80);
                    client.Post("/api/v1/games/", payload, "application/json");
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

SDL_GameController *FindGameController() {
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            return SDL_GameControllerOpen(i);
        }
    }

    return nullptr;
}

void InitAxis(ControllerAxis &axis) {
    axis.last = 0.0f;
    axis.value = 0.0f;
    axis.active = 0;
    axis.last_active = 0;
    axis.toggled = 0;
}

void InitStick(ControllerStick &stick, uint16_t clamp_low = 4000, uint16_t clamp_high = 28000) {
    stick.clamp_inner = clamp_low;
    stick.clamp_outer = clamp_high;
    stick.elapsed = 0;
    InitAxis(stick.x);
    InitAxis(stick.y);
}

void InitTrigger(ControllerTrigger &trigger) {
    trigger.clamp_low = 3000;
    trigger.clamp_high = 30000;
    trigger.elapsed = 0;
    InitAxis(trigger.t);
}

int UpdateAxis(ControllerAxis &axis, float value) {
    axis.last_active = axis.active;
    if (abs(value) < 0.0001f) {
        value = 0.0f;
        axis.active = 0;
    }
    else {
        axis.active = 1;
    }
    axis.last = axis.value;
    axis.value = value;
    axis.toggled = (axis.value <= 0) != (axis.last <= 0);
    // We want to send the an event if the axis is active *or* it just became inactive.
    return axis.active + axis.last_active + axis.toggled;
}

int UpdateStick(ControllerStick &stick, int32_t x, int32_t y, uint32_t elapsed) {
    int32_t magnitude = sqrt((x * x) + (y * y));
    float scale = float(magnitude - stick.clamp_inner) / float(stick.clamp_outer - stick.clamp_inner);
    scale = std::clamp(scale, 0.0f, 1.0f) / magnitude;
    if (UpdateAxis(stick.x, x * scale) + UpdateAxis(stick.y, y * scale)) {
        stick.elapsed += elapsed;
        return 1;
    }
    else {
        stick.elapsed = 0;
        return 0;
    }
}

int UpdateTrigger(ControllerTrigger &trigger, uint16_t t, uint32_t elapsed) {
    float scaled = float(t - trigger.clamp_low) / float(trigger.clamp_high - trigger.clamp_low);
    scaled = std::clamp(scaled, 0.0f, 1.0f) * t;
    if (UpdateAxis(trigger.t, scaled)) {
        trigger.elapsed += elapsed;
        return 1;
    }
    else {
        trigger.elapsed = 0;
        return 0;
    }
}

CAvaraAppImpl::CAvaraAppImpl() : CApplication("Avara") {
    AssetManager::Init();

    controllerBaseEvent = SDL_RegisterEvents(1);
    lastControllerEvent = 0;
    controller = FindGameController();
    controllerPollMillis = 1000 / Number(kControllerPollRate);

    InitStick(sticks.left);
    InitStick(sticks.right);
    InitTrigger(triggers.left);
    InitTrigger(triggers.right);
    
    itsGame = std::make_unique<CAvaraGame>(Get<FrameTime>(kFrameTimeTag));
    gCurrentGame = itsGame.get();

    // use sqlite to persist stuff
    itsAPI = std::make_unique<CRUDsqlite>();

    if (mNVGContext) {
        ui = std::make_unique<CHUD>(gCurrentGame);
        ui->LoadImages(mNVGContext);
    }

#if defined(AVARA_GLES)
    gRenderer = new LegacyOpenGLRenderer(mSDLWindow);
#else
    if (Get(kUseLegacyRenderer)) {
        gRenderer = new LegacyOpenGLRenderer(mSDLWindow);
    } else {
        gRenderer = new ModernOpenGLRenderer(mSDLWindow);
    }
#endif

    gRenderer->UpdateViewRect(mSize.x, mSize.y, mPixelRatio);
    gRenderer->SetFOV(Number(kFOV));
    gRenderer->ResetLights();

    itsGame->IAvaraGame(this);

    gameNet->ChangeNet(kNullNet, "");

    previewAngle = 0;
    previewRadius = 0;
    animatePreview = false;

    setLayout(new nanogui::FlowLayout(nanogui::Orientation::Vertical, true, 20, 20));

    playerWindow = new CPlayerWindow(this);
    playerWindow->setFixedWidth(200);

    levelWindow = new CLevelWindow(this);
    levelWindow->setFixedWidth(200);

    networkWindow = new CNetworkWindow(this);
    networkWindow->setFixedWidth(200);

    serverWindow = new CServerWindow(this);
    serverWindow->setFixedWidth(200);

    trackerWindow = new CTrackerWindow(this);
    trackerWindow->setFixedWidth(325);

    rosterWindow = new CRosterWindow(this);

    performLayout();

    for (auto win : windowList) {
        win->restoreState();
    }

    nextTrackerUpdate = 0;
    trackerUpdatePending = false;
    trackerThread = new std::thread(TrackerPinger, this);
    trackerThread->detach();

    // register and handle text commands
    itsTui = new CommandManager(this);

    MessageLine(kmWelcome1, MsgAlignment::Center);
    AddMessageLine("Type /help and press return for a list of chat commands.", MsgAlignment::Center);

    // load up a random decent starting level

    if (this->Boolean(kPunchHoles)) {
        std::string host = String(kPunchServerAddress);
        uint16_t port = static_cast<uint16_t>(Number(kPunchServerPort));
        SDL_Log("Enabling UDP hole punching via %s:%d", host.c_str(), port);
        PunchSetup(host.c_str(), port);
    }
}

CAvaraAppImpl::~CAvaraAppImpl() {
    DeallocParser();
}

void CAvaraAppImpl::Done() {
    // This will trigger a clean disconnect if connected.
    gameNet->ChangeNet(kNullNet, "");
    if (controller)
        SDL_GameControllerClose(controller);
    CApplication::Done();
}

void CAvaraAppImpl::idle() {
    static double avg = 0;
    FrameNumber curFrame = itsGame->frameNumber;
    uint32_t procTime = SDL_GetTicks();

    CheckSockets();
    TrackerUpdate();

    // Poll for controller axis value at kControllerPollRate
    uint32_t elapsed = procTime - lastControllerEvent;
    if (controller && elapsed > controllerPollMillis) {
        int controllerActive = 0;
        
        int16_t leftX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        int16_t leftY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
        controllerActive += UpdateStick(sticks.left, leftX, leftY, elapsed);
        
        int16_t rightX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
        int16_t rightY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);
        controllerActive += UpdateStick(sticks.right, rightX, rightY, elapsed);
        
        int16_t leftT = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
        controllerActive += UpdateTrigger(triggers.left, leftT, elapsed);

        int16_t rightT = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
        controllerActive += UpdateTrigger(triggers.right, rightT, elapsed);

        if (controllerActive) {
            SDL_Event controllerEvent;
            controllerEvent.type = controllerBaseEvent;
            controllerEvent.user.data1 = &sticks;
            controllerEvent.user.data2 = &triggers;
            SDL_PushEvent(&controllerEvent);
        }
        
        lastControllerEvent = procTime;
    }

    if (itsGame->GameTick()) {
        RenderContents();
    }

    // output a coarse estimate of cpu time & percent every second when enabled
    if (curFrame > 1 && curFrame != itsGame->frameNumber && Debug::IsEnabled("cpu")) {
        procTime = SDL_GetTicks() - procTime;
        avg = 0.99 * avg + 0.01 * procTime;
        if (curFrame % (1000 / itsGame->frameTime) == 0) {
            DBG_Log("cpu", "%.1fms (%.0f%%)\n", avg, 100.0 * avg / itsGame->frameTime);
        }
    }
}

void CAvaraAppImpl::drawContents() {
    if (animatePreview) {
        auto vp = gRenderer->viewParams;
        Fixed radius = 0.6*previewRadius + 0.4*FMul(previewRadius, FOneCos(previewAngle*0.7));
        Fixed x = itsGame->extentCenter[0] + FMul(radius, FOneCos(previewAngle));
        Fixed y = itsGame->extentCenter[1] + FMul(std::min(itsGame->extentCenter[1], FIX(100)), FOneSin(previewAngle) + FIX1);
        Fixed z = itsGame->extentCenter[2] + FMul(radius, FOneSin(previewAngle));
        vp->LookFrom(x, y, z);
        vp->LookAt(itsGame->extentCenter[0], itsGame->extentCenter[1], itsGame->extentCenter[2]);
        vp->PointCamera();
        previewAngle += FIX3(1);
    }
    itsGame->Render();
    if (ui) {
        if (itsGame->showNewHUD) {
            ui->RenderNewHUD(mNVGContext);
        } else {
            ui->Render(mNVGContext);
        }
    }
}

// display only the game screen, not the widgets
void CAvaraAppImpl::RenderContents() {
    drawContents();
    gRenderer->RefreshWindow();
}

void CAvaraAppImpl::WindowResized(int width, int height) {
    // Only update if the resolution is actually changing
    if (gRenderer->viewParams->viewPixelDimensions.h != width || gRenderer->viewParams->viewPixelDimensions.v != height)
        gRenderer->UpdateViewRect(width, height, mPixelRatio);
    // performLayout();
}

void CAvaraAppImpl::PrefChanged(std::string name) {
    CApplication::PrefChanged(name);
    if (itsGame) itsGame->ReadGamePrefs();
    if (gRenderer) gRenderer->ApplyPrefs(name);
}

bool CAvaraAppImpl::handleSDLEvent(SDL_Event &event) {
    switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED:
            if (!controller) {
                controller = SDL_GameControllerOpen(event.cdevice.which);
            }
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            if (controller &&
                event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller))) {
                SDL_GameControllerClose(controller);
                controller = FindGameController();
            }
            break;
    }

    if (itsGame->IsPlaying()) {
        itsGame->HandleEvent(event);
        return true;
    } else {
        for (int i = 0; i < windowList.size(); i++) {
            if (windowList[i]->editing()) {
                CApplication::handleSDLEvent(event);
                return true;
            }
        }

        if (rosterWindow->handleSDLEvent(event))
            return true;

        return CApplication::handleSDLEvent(event);
    }
}

void CAvaraAppImpl::drawAll() {
    if (!itsGame->IsPlaying()) {
        rosterWindow->UpdateRoster();
        CApplication::drawAll();
    }
}

void CAvaraAppImpl::GameStarted(LevelInfo &loadedLevel) {
    auto vp = gRenderer->viewParams;
    animatePreview = false;
    vp->showTransparent = false;
    itsGame->IncrementGameCounter();
    MessageLine(kmStarted, MsgAlignment::Center);
    levelWindow->UpdateRecents();
    if (controller) {
        auto color = itsGame->GetLocalTeamColor();
        SDL_GameControllerSetLED(controller, color.GetR(), color.GetG(), color.GetB());
    }
    itsAPI->RecordGameStart(itsGame);
}

void CAvaraAppImpl::Rumble(Fixed hitEnergy) {
    if (controller) {
        SDL_GameControllerRumble(controller, 0x00FF, 0xFF00, 250);
    }
}

bool CAvaraAppImpl::DoCommand(int theCommand) {
    std::string name = String(kPlayerNameTag);
    Str255 userName;
    userName[0] = name.length();
    BlockMoveData(name.c_str(), userName + 1, userName[0]);
    // SDL_Log("DoCommand %d\n", theCommand);
    switch (theCommand) {
        case kReportNameCmd:
            gameNet->NameChange(userName);
            return true;
            // break;
        case kStartGame:
        case kResumeGame:
            itsGame->SendStartCommand();
            return true;
            // break;
        case kGetReadyToStartCmd:
            break;
        default:
            break;
    }
    return false;
    /*
        default:
            if(	theCommand >= kLatencyToleranceZero &&
                theCommand <= kLatencyToleranceMax)
            {	WriteShortPref(kLatencyToleranceTag, theCommand - kLatencyToleranceZero);
            }
            else
            if(	theCommand >= kRetransmitMin &&
                theCommand <= kRetransmitMax)
            {	WriteShortPref(kUDPResendPrefTag, theCommand - kRetransmitMin);

                if(gameNet->itsCommManager)
                {	gameNet->itsCommManager->OptionCommand(theCommand);
                }
            }
            if(	theCommand >= kSlowestConnectionCmd && theCommand <= kFastestConnectionCmd)
            {	WriteShortPref(kUDPConnectionSpeedTag, theCommand - kSlowestConnectionCmd);

                if(gameNet->itsCommManager)
                {	gameNet->itsCommManager->OptionCommand(theCommand);
                }
            }
            else
                inherited::DoCommand(theCommand);
            break;
    }
    */
}

OSErr CAvaraAppImpl::LoadLevel(std::string set, std::string levelTag, CPlayerManager *sendingPlayer) {
    SDL_Log("LOADING LEVEL %s FROM %s\n", levelTag.c_str(), set.c_str());
    if (itsGame->itsFilm->HasUnflushedFrames()) {
        // film sql
        itsAPI->RecordFrames(itsGame);
        
        // film file test
        char *ppath = SDL_GetPrefPath("Avaraline", "Avara");
        const auto ts(std::chrono::steady_clock::now());
        const size_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(ts.time_since_epoch()).count();
        auto path = std::string(ppath) + std::to_string(ms) + "-film.bin";
        std::ofstream filmFile(path, std::ios::out | std::ios::binary);
        itsGame->itsFilm->Serialize(filmFile, itsGame);
        filmFile.close();
        
        std::ifstream testInputFilm(path, std::ios::in | std::ios::binary);
        itsGame->itsFilm->DeserializeTest(testInputFilm, itsGame);
        testInputFilm.close();
    }
    
    itsGame->LevelReset(false);
    gCurrentGame = itsGame.get();

    ColorManager::resetOverrides();

    itsGame->loadedLevelInfo = std::make_unique<LevelInfo>(set, "loading...", levelTag);
    OSErr result = AssetManager::LoadLevel(set, levelTag, itsGame->loadedLevelInfo->levelName);

    if (result == noErr) {
        playerWindow->RepopulateHullOptions();

        std::string msgStr = "Loaded";
        if (sendingPlayer != NULL) {
            msgStr = sendingPlayer->GetPlayerName() + " loaded";
        }
        auto &level = itsGame->loadedLevelInfo->levelName;
        msgStr += " \"" + level + "\" from \"" + set + "\".";
        AddMessageLine(msgStr);

        levelWindow->SelectLevel(set, level);

        // calculate the extent of visible objects in the level (used for level previews and random incarnators)
        itsGame->CalculateExtent();

        auto vp = gRenderer->viewParams;
        vp->yonBound = FIX(10000);
        vp->showTransparent = true;

        previewAngle = 0;
        previewRadius = itsGame->extentRadius * 1.75;

        animatePreview = true;
    }

    return result;
}

void CAvaraAppImpl::NotifyUser() {
    // TODO: Bell sound(s)
    SDL_Log("BEEP!!!\n");
    Beep();
}

CAvaraGame *CAvaraAppImpl::GetGame() {
    return itsGame.get();
}

CNetManager *CAvaraAppImpl::GetNet() {
    return gameNet;
}

CommandManager *CAvaraAppImpl::GetTui() {
    return itsTui;
}

void CAvaraAppImpl::SetNet(CNetManager *theNet) {
    gameNet = theNet;
}

void CAvaraAppImpl::AddMessageLine(std::string lines, MsgAlignment align, MsgCategory category) {
    std::istringstream iss(lines);
    std::string line;
    MsgLine msg;

    msg.align = align;
    msg.category = category;
    msg.gameId = itsGame->currentGameId;

    // split string on newlines
    while (std::getline(iss, line)) {
        SDL_Log("Message: %s", line.c_str());
        msg.text = line;
        messageLines.push_back(msg);
        if (messageLines.size() > 500) {
            messageLines.pop_front();
        }
    }
}

void CAvaraAppImpl::MessageLine(short index, MsgAlignment align) {
    // SDL_Log("CAvaraAppImpl::MessageLine(%d)\n", index);
    switch (index) {
        case kmWelcome1:
        case kmWelcome2:
        case kmWelcome3:
        case kmWelcome4:
            AddMessageLine("Welcome to Avara.", align);
            break;
        case kmStarted:
            AddMessageLine("Starting new game.", align);
            break;
        case kmRestarted:
            AddMessageLine("Resuming game.", align);
            break;
        case kmAborted:
            AddMessageLine("Aborted.", align);
            break;
        case kmWin:
            AddMessageLine("Mission complete.", align);
            break;
        case kmGameOver:
            AddMessageLine("Game over.", align);
            break;
        case kmSelfDestruct:
            AddMessageLine("Self-destruct activated.", align);
            break;
        case kmFragmentAlert:
            AddMessageLine("ALERT: Reality fragmentation detected!", align, MsgCategory::Error);
            break;
        case kmRefusedLogin:
            AddMessageLine("Login refused.", align, MsgCategory::Error);
            break;
    }
}

std::deque<MsgLine> &CAvaraAppImpl::MessageLines() {
    return messageLines;
}
void CAvaraAppImpl::LevelReset() {}
void CAvaraAppImpl::ParamLine(short index, MsgAlignment align, StringPtr param1, StringPtr param2) {
    SDL_Log("CAvaraAppImpl::ParamLine(%d)\n", index);
    std::stringstream buffa;
    std::string a = ToString(param1);
    std::string b;
    MsgCategory category = MsgCategory::System;
    if (param2)
        b = ToString(param2);

    switch (index) {
        case kmPaused:
            buffa << "Game paused by " << a << ".";
            break;
        case kmWaitingForPlayer:
            buffa << "Waiting for " << a << "... (abort to exit)";
            category = MsgCategory::Error;
            break;
        case kmAKilledBPlayer:
            buffa << a << " killed " << b << ".";
            break;
        case kmKilledByCollision:
            buffa << a << " was killed by the world.";
            break;
        case kmUnavailableNote:
            buffa << a << " is busy.";
            category = MsgCategory::Error;
            break;
        case kmStartFailure:
            buffa << a << " wasn't ready.";
            category = MsgCategory::Error;
            break;
    }

    AddMessageLine(buffa.str(), align, category);
}
void CAvaraAppImpl::StartFrame(FrameNumber frameNum) {}

void CAvaraAppImpl::StringLine(std::string theString, MsgAlignment align) {
    AddMessageLine(theString.c_str(), align, MsgCategory::Level);
}

void CAvaraAppImpl::ComposeParamLine(StringPtr destStr, short index, StringPtr param1, StringPtr param2) {
    ParamLine(index, MsgAlignment::Left, param1, param2);
}

void CAvaraAppImpl::TrackerUpdate() {
    if (SDL_GetTicks() < nextTrackerUpdate)
        return;
    if (gameNet->netStatus != kServerNet)
        return;

    const std::lock_guard<std::mutex> lock(trackerLock);
    long freq = Number(kTrackerRegisterFrequency);

    trackerState.clear();

    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        unsigned char *p = gameNet->playerTable[i]->PlayerName();
        std::string playerName((char *)p + 1, p[0]);
        if (playerName.size() > 0) {
            trackerState["players"].push_back(playerName);
        }
    }
    if (trackerState["players"].empty()) {
        trackerState["players"].push_back(String(kPlayerNameTag));
    }
    std::string gitv = std::string(GIT_VERSION);
    trackerState["version"]["git"] = gitv;
    trackerState["version"]["net"] = kAvaraNetVersion;
    trackerState["description"] = gitv.substr(0, 6) + (gitv.length() > 8 ? "\\*: " : ": ") + String(kServerDescription);
    trackerState["password"] = String(kServerPassword).length() > 0 ? true : false;

    DBG_Log("tracker", "%s", trackerState.dump().c_str());

    trackerUpdatePending = true;
    nextTrackerUpdate = SDL_GetTicks() + (freq * 1000);
}

std::string CAvaraAppImpl::TrackerPayload() {
    const std::lock_guard<std::mutex> lock(trackerLock);
    std::string payload = trackerUpdatePending && trackerState.size() > 0 ? trackerState.dump() : "";
    trackerUpdatePending = false;
    return payload;
}

void CAvaraAppImpl::SetIndicatorDisplay(short i, short v) {}
void CAvaraAppImpl::NumberLine(long theNum, short align) {}
void CAvaraAppImpl::DrawUserInfoPart(short i, short partList) {}
void CAvaraAppImpl::BrightBox(FrameNumber frameNum, short position) {}
