/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CAvaraApp.c
    Created: Wednesday, November 16, 1994, 01:26
    Modified: Friday, September 20, 1996, 02:05
*/

#define MAINAVARAAPP

#include "CAvaraApp.h"

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
#include "Resource.h"
#include "System.h"
#include "InfoMessages.h"
#include "Messages.h"
#include "Beeper.h"
#include "httplib.h"
#include <chrono>
#include <json.hpp>

// included while we fake things out
#include "CPlayerManager.h"

std::mutex trackerLock;

void TrackerPinger(CAvaraAppImpl *app) {
    while (true) {
        if(app->Number(kTrackerRegister) == 1) {
            std::string payload = app->TrackerPayload();
            if (payload.size() > 0) {
                // Probably not thread-safe.
               std::string address = app->String(kTrackerRegisterAddress);
                SDL_Log("Pinging %s", address.c_str());
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
    AvaraGLInitContext();
    itsGame = new CAvaraGame(gApplication->Number(kFrameTimeTag));
    gCurrentGame = itsGame;
    itsGame->IAvaraGame(this);
    itsGame->UpdateViewRect(win_size_x, win_size_y, pixel_ratio);

    AvaraGLSetFOV(Number(kFOV));

    gameNet->ChangeNet(kNullNet, "");

    previewAngle = 0;
    previewRadius = 0;
    animatePreview = false;
    nextTrackerUpdate = 0;
    trackerUpdatePending = false;
    trackerThread = new std::thread(TrackerPinger, this);
    trackerThread->detach();


    itsGUI = new CGUI(this);
    LoadDefaultOggFiles();

    // register and handle text commands
    itsTui = new CommandManager(this);

    MessageLine(kmWelcome1, MsgAlignment::Center);
    AddMessageLine(
        "Type /help and press return for a list of chat commands.",
        MsgAlignment::Center
    );
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
    itsGame->GameTick();
    itsGUI->Update();
}

void CAvaraAppImpl::drawContents() {
    if(animatePreview) {
        Fixed x = overhead[0] + FMul(previewRadius, FOneCos(previewAngle));
        Fixed y = overhead[1] + FMul(FMul(extent[3], FIX(2)), FOneSin(previewAngle) + FIX(1));
        Fixed z = overhead[2] + FMul(previewRadius, FOneSin(previewAngle));
        itsGame->itsView->LookFrom(x, y, z);
        itsGame->itsView->LookAt(overhead[0], overhead[1], overhead[2]);
        itsGame->itsView->PointCamera();
        previewAngle += FIX3(itsGame->fpsScale);
    }
    itsGame->Render(nvg_context);
    itsGUI->Render(nvg_context);
}

void CAvaraAppImpl::WindowResized(int width, int height) {
    //SDL_Log("Resize event: %i %i", width, height);
    itsGame->UpdateViewRect(width, height, pixel_ratio);
}

bool CAvaraAppImpl::handleSDLEvent(SDL_Event &event) {
    if (CApplication::handleSDLEvent(event))
        return true;
    else if(itsGame->IsPlaying()) {
        itsGame->HandleEvent(event);
        return true;
    }
    else if (itsGUI->handleSDLEvent(event))
        return true;
    else 
    return false;
}

void CAvaraAppImpl::GameStarted(std::string set, std::string level) {
    animatePreview = false;
    itsGame->itsView->showTransparent = false;
    MessageLine(kmStarted, MsgAlignment::Center);
}

bool CAvaraAppImpl::DoCommand(int theCommand) {
    std::string name = String(kPlayerNameTag);
    Str255 userName;
    userName[0] = name.length();
    BlockMoveData(name.c_str(), userName + 1, name.length());
    SDL_Log("DoCommand %d\n", theCommand);
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
    gCurrentGame = itsGame;
    itsGame->loadedSet = set;
    UseLevelFolder(set);

    OSErr result = fnfErr;
    json setManifest = GetManifestJSON(set);
    if(setManifest == -1) {
        SDL_Log("File read error");
        return result;
    }
    if(setManifest.find("LEDI") == setManifest.end()){
        SDL_Log("LEDI key not found in set.json for %s", set.c_str());
        return result;
    } 

    json ledi = NULL;
    for (auto &ld : setManifest["LEDI"].items()) {
        if (ld.value()["Alf"] == levelTag)
            ledi = ld.value();
    }
    if(ledi == NULL) {
        SDL_Log("LEDI for %s in %s not found.", levelTag.c_str(), set.c_str());
        return result;
    }

    if(LoadALF(GetALFPath(levelTag))) result = noErr;

    if (result == noErr) {
        itsGame->loadedLevel = ledi["Name"];
        itsGame->loadedTag  = levelTag;
        std::string msgPrefix = "Loaded";
        if(sendingPlayer != NULL)
            msgPrefix = sendingPlayer->GetPlayerName() + " loaded";
        AddMessageLine(msgPrefix + " \"" + itsGame->loadedLevel + "\" from \"" + set + "\".");
        //levelWindow->SelectLevel(set, itsGame->loadedLevel);

        itsGame->itsWorld->OverheadPoint(overhead, extent);
        itsGame->itsView->yonBound = FIX(10000);
        itsGame->itsView->showTransparent = true;

        previewAngle = 0;
        previewRadius = std::max(extent[1] - extent[0], extent[5] - extent[4]);
        //animatePreview = true;
    }
    else {
        SDL_Log("ALF load error");
    }

    return result;
}


void CAvaraAppImpl::NotifyUser() {
    // TODO: Bell sound(s)
    SDL_Log("BEEP!!!\n");
    Beep();
}

CAvaraGame* CAvaraAppImpl::GetGame() {
    return itsGame;
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

    // split string on newlines
    while(std::getline(iss, line)) {
        SDL_Log("Message: %s", line.c_str());
        msg.text = line;
        messageLines.push_back(msg);
        if (messageLines.size() > 5) {
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
void CAvaraAppImpl::StartFrame(long frameNum) {}

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

    SDL_Log("TrackerUpdate: %s", trackerState.dump().c_str());

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
void CAvaraAppImpl::BrightBox(long frameNum, short position) {}
