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
#include "ColorManager.h"
#include "AvaraGL.h"
#include "AvaraScoreInterface.h"
#include "AvaraTCP.h"
#include "CAvaraGame.h"
#include "CBSPWorld.h"
#include "CharWordLongPointer.h"
#include "CNetManager.h"
#include "CRC.h"
#include "CSoundMixer.h"
#include "CViewParameters.h"
#include "CommandList.h"
#include "KeyFuncs.h"
#include "LevelLoader.h"
#include "Parser.h"
#include "Preferences.h"
#include "System.h"
#include "InfoMessages.h"
#include "Messages.h"
#include "Beeper.h"
#include "httplib.h"
#include <chrono>
#include <json.hpp>
#include "Tags.h"
#include "Debug.h"

// included while we fake things out
#include "CPlayerManager.h"

std::mutex trackerLock;

void TrackerPinger(CAvaraAppImpl *app) {
    while (true) {
        if(app->Number(kTrackerRegister) == 1 && app->GetNet()->netStatus == kServerNet) {
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
                }
                else {
                    httplib::Client client(address.c_str(), 80);
                    client.Post("/api/v1/games/", payload, "application/json");
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

CAvaraAppImpl::CAvaraAppImpl() : CApplication("Avara") {
    AssetManager::Init();
    itsGame = std::make_unique<CAvaraGame>(Get<FrameTime>(kFrameTimeTag));
    gCurrentGame = itsGame.get();
    itsGame->IAvaraGame(this);
    itsGame->UpdateViewRect(mSize.x, mSize.y, mPixelRatio);
    itsGame->LoadImages(mNVGContext);

    AvaraGLSetFOV(Number(kFOV));

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
    AddMessageLine(
        "Type /help and press return for a list of chat commands.",
        MsgAlignment::Center
    );

    // load up a random decent starting level

    if (Boolean(kPunchHoles)) {
        std::string host = String(kPunchServerAddress);
        uint16_t port = static_cast<uint16_t>(Number(kPunchServerPort));
        SDL_Log("Enabling UDP hole punching via %s:%d", host.c_str(), port);
        PunchSetup(host.c_str(), port);
    }
}

CAvaraAppImpl::~CAvaraAppImpl() {
    itsGame->Dispose();
    DeallocParser();
}

void CAvaraAppImpl::Done() {
    // This will trigger a clean disconnect if connected.
    gameNet->ChangeNet(kNullNet, "");
    CApplication::Done();
}

void CAvaraAppImpl::idle() {
    CheckSockets();
    TrackerUpdate();
    if(itsGame->GameTick()) {
        RenderContents();
    }
}

void CAvaraAppImpl::drawContents() {
    if(animatePreview) {
        Fixed x = overhead[0] + FMul(previewRadius, FOneCos(previewAngle));
        Fixed y = overhead[1] + FMul(FMul(extent[3], FIX(2)), FOneSin(previewAngle) + FIX1);
        Fixed z = overhead[2] + FMul(previewRadius, FOneSin(previewAngle));
        itsGame->itsView->LookFrom(x, y, z);
        itsGame->itsView->LookAt(overhead[0], overhead[1], overhead[2]);
        itsGame->itsView->PointCamera();
        previewAngle += FIX3(1);
    }
    itsGame->Render(mNVGContext);
}

// display only the game screen, not the widgets
void CAvaraAppImpl::RenderContents() {
    glClearColor(mBackground[0], mBackground[1], mBackground[2], mBackground[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    drawContents();
    SDL_GL_SwapWindow(mSDLWindow);
}

void CAvaraAppImpl::WindowResized(int width, int height) {
    itsGame->UpdateViewRect(width, height, mPixelRatio);
    //performLayout();
}

bool CAvaraAppImpl::handleSDLEvent(SDL_Event &event) {
    if(itsGame->IsPlaying()) {
        itsGame->HandleEvent(event);
        return true;
    }
    else {
        for (int i = 0; i < windowList.size(); i++) {
            if(windowList[i]->editing()) {
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

void CAvaraAppImpl::GameStarted(std::string set, std::string level) {
    animatePreview = false;
    itsGame->itsView->showTransparent = false;
    itsGame->IncrementGameCounter();
    MessageLine(kmStarted, MsgAlignment::Center);
    levelWindow->AddRecent(set, level);
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
    itsGame->LevelReset(false);
    gCurrentGame = itsGame.get();
    itsGame->loadedSet = set;

    ColorManager::refresh(this);

    std::string levelName;
    OSErr result = AssetManager::LoadLevel(set, levelTag, levelName);

    if (result == noErr) {
        playerWindow->RepopulateHullOptions();
        itsGame->loadedLevel = levelName;
        itsGame->loadedFilename  = levelTag;
        itsGame->loadedTags = Tags::GetTagsForLevel(Tags::LevelURL(itsGame->loadedSet, itsGame->loadedLevel));
        std::string msgStr = "Loaded";
        if (sendingPlayer != NULL) {
            msgStr = sendingPlayer->GetPlayerName() + " loaded";
        }
        msgStr += " \"" + itsGame->loadedLevel + "\" from \"" + set + "\".";
        if (!itsGame->loadedTags.empty()) {
            msgStr += " (tags:";
            for (auto tag: itsGame->loadedTags) {
                msgStr += " " + tag;
            }
            msgStr += ")";
        }
        AddMessageLine(msgStr);

        levelWindow->SelectLevel(set, itsGame->loadedLevel);

        itsGame->itsWorld->OverheadPoint(overhead, extent);
        itsGame->itsView->yonBound = FIX(10000);
        itsGame->itsView->showTransparent = true;

        previewAngle = 0;
        previewRadius = std::max(extent[1] - extent[0], extent[5] - extent[4]);
        animatePreview = true;
    }

    return result;
}


void CAvaraAppImpl::NotifyUser() {
    // TODO: Bell sound(s)
    SDL_Log("BEEP!!!\n");
    Beep();
}

CAvaraGame* CAvaraAppImpl::GetGame() {
    return itsGame.get();
}

CNetManager* CAvaraAppImpl::GetNet() {
    return gameNet;
}

CommandManager* CAvaraAppImpl::GetTui() {
    return itsTui;
}

void CAvaraAppImpl::SetNet(CNetManager *theNet) {
    gameNet = theNet;
}

void CAvaraAppImpl::AddMessageLine(
    std::string lines,
    MsgAlignment align,
    MsgCategory category
    ) {
    std::istringstream iss(lines);
    std::string line;
    MsgLine msg;

    msg.align = align;
    msg.category = category;
    msg.gameId = itsGame->currentGameId;

    // split string on newlines
    while(std::getline(iss, line)) {
        SDL_Log("Message: %s", line.c_str());
        msg.text = line;
        messageLines.push_back(msg);
        if (messageLines.size() > 500) {
            messageLines.pop_front();
        }
    }
}

void CAvaraAppImpl::MessageLine(short index, MsgAlignment align) {
    //SDL_Log("CAvaraAppImpl::MessageLine(%d)\n", index);
    switch(index) {
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
            AddMessageLine(
                "ALERT: Reality fragmentation detected!",
                align,
                MsgCategory::Error
            );
            break;
        case kmRefusedLogin:
            AddMessageLine(
                "Login refused.",
                align,
                MsgCategory::Error
            );
            break;
    }

}

std::deque<MsgLine>& CAvaraAppImpl::MessageLines() {
    return messageLines;
}
void CAvaraAppImpl::LevelReset() {}
void CAvaraAppImpl::ParamLine(short index, MsgAlignment align, StringPtr param1, StringPtr param2) {
    SDL_Log("CAvaraAppImpl::ParamLine(%d)\n", index);
    std::stringstream buffa;
    std::string a = std::string((char *)param1 + 1, param1[0]);
    std::string b;
    MsgCategory category = MsgCategory::System;
    if (param2) b = std::string((char *)param2 + 1, param2[0]);

    switch(index) {
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
    if (SDL_GetTicks() < nextTrackerUpdate) return;
    if (gameNet->netStatus != kServerNet) return;

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
    if(trackerState["players"].empty()) {
        trackerState["players"].push_back(String(kPlayerNameTag));
    }
    trackerState["description"] = String(kServerDescription);
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
