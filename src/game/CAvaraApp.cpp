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
#include "Beeper.h"
#include "httplib.h"
#include <chrono>
#include <json.hpp>
#include <random>


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
    itsGame = new CAvaraGame(gApplication->Number(kFrameTimeTag));
    gCurrentGame = itsGame;
    itsGame->IAvaraGame(this);
    itsGame->UpdateViewRect(mSize.x, mSize.y, mPixelRatio);

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


    LoadDefaultOggFiles();
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
        glClearColor(mBackground[0], mBackground[1], mBackground[2], mBackground[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        drawContents();
        SDL_GL_SwapWindow(mSDLWindow);
    }
}

void CAvaraAppImpl::drawContents() {
    if(animatePreview) {
        Fixed x = overhead[0] + FMul(previewRadius, FOneCos(previewAngle));
        Fixed y = overhead[1] + FMul(FMul(extent[3], FIX(2)), FOneSin(previewAngle) + FIX(1));
        Fixed z = overhead[2] + FMul(previewRadius, FOneSin(previewAngle));
        itsGame->itsView->LookFrom(x, y, z);
        itsGame->itsView->LookAt(overhead[0], overhead[1], overhead[2]);
        itsGame->itsView->PointCamera();
        previewAngle += FIX3(1);
    }
    itsGame->Render(mNVGContext);
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
    MessageLine(kmStarted, centerAlign);
    levelWindow->AddRecent(set, level);
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
    if(setManifest == -1) return result;
    if(setManifest.find("LEDI") == setManifest.end()) return result;

    json ledi = NULL;
    for (auto &ld : setManifest["LEDI"].items()) {
        if (ld.value()["Alf"] == levelTag)
            ledi = ld.value();
    }
    if(ledi == NULL) return result;

    if(LoadALF(GetALFPath(levelTag))) result = noErr;

    if (result == noErr) {
        itsGame->loadedLevel = ledi["Name"];
        itsGame->loadedTag  = levelTag;
        std::string msgPrefix = "Loaded";
        if(sendingPlayer != NULL)
            msgPrefix = sendingPlayer->GetPlayerName() + " loaded";
        AddMessageLine(msgPrefix + " \"" + itsGame->loadedLevel + "\" from \"" + set + "\".");
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
    return itsGame;
}

CNetManager* CAvaraAppImpl::GetNet() {
    return gameNet;
}
void CAvaraAppImpl::SetNet(CNetManager *theNet) {
    gameNet = theNet;
}

void CAvaraAppImpl::AddMessageLine(std::string line) {
    SDL_Log("Message: %s", line.c_str());
    messageLines.push_back(line);
    if (messageLines.size() > 5) {
        messageLines.pop_front();
    }
}

void CAvaraAppImpl::ChatCommand(std::string chatText, CPlayerManager* player) {
    if(player->CalculateIsLocalPlayer()) {
        SDL_Log("status=%hi", player->LoadingStatus());

        if(chatText == "/r" || chatText == "/random") {
            //load random level
            std::vector<std::string> levelSets = LevelDirNameListing();
            
            std::random_device rd; // obtain a random number from hardware
            std::mt19937 gen(rd()); // seed the generator
            std::uniform_int_distribution<> distr(0, levelSets.size() - 1); // define the range
            
            std::string set = levelSets[distr(gen)];
            levelWindow->SelectSet(set);
            
            nlohmann::json levels = LoadLevelListFromJSON(set);

            if(levels.is_array())
                SDL_Log("ARRAY");
            
            std::random_device rdLevel;
            std::mt19937 genLevel(rdLevel());
            std::uniform_int_distribution<> distrLevel(0, levels.size() - 1);

            nlohmann::json jsonLevel = levels[distrLevel(genLevel)];
            std::string level = jsonLevel.at("Name");
            
            levelWindow->SelectLevel(set, level);
            levelWindow->SendLoad();
        }
        else if(chatText == "/active" || chatText == "/a") {
            short status = kLNotPlaying;

            if(player->LoadingStatus() == kLNotPlaying) {
                if(itsGame->loadedLevel.length() > 0) {
                    status = kLLoaded;
                    SDL_Log("STATUS=Loaded");
                }
                else {
                    status = kLConnected;
                    SDL_Log("STATUS=Connected");
                }
            }
            else {
                SDL_Log("STATUS=Not Playing");
            }
            player->SetPlayerStatus(status, -1);
            gameNet->StatusChange();
        }
        else if(chatText.rfind("/pref ", 0) == 0 || chatText.rfind("/p ", 0) == 0) {
            std::string pref;
            std::string value;
            std::stringstream chatSS(chatText);
            getline(chatSS, pref, ' '); //skip "/p "
            getline(chatSS, pref, ' ');
            getline(chatSS, value, ' ');

            if(value.length() == 0) {
                //read prefs
                AddMessageLine(pref + " = " + this->Get(pref).dump());
            }
            else {
                //write prefs
                nlohmann::json currentValue = this->Get(pref); //get current type
                if(currentValue.is_string()) {
                    SDL_Log("STRING");
                    this->Set(pref, value);
                }
                else if (currentValue.is_number_float() || value.find('.') != std::string::npos ) {
                    SDL_Log("FLOAT  %s %f", value.c_str(), stof(value));
                    nlohmann::json jsonFloat = nlohmann::json(stof(value));
                    this->Set(pref, jsonFloat);
                }
                else if (currentValue.is_number_integer()) {
                    SDL_Log("INT");
                    this->Set(pref, stoi(value));
                }
                else if (currentValue.is_boolean()) {
                    SDL_Log("BOOL");
                    nlohmann::json bvalue = nlohmann::json(value == "true" ? true : false);
                    this->Set(pref, bvalue);
                }

                AddMessageLine(pref + " set to = " + value);
                CApplication::PrefChanged(pref);
            }
        }
        SDL_Log("new status=%hi", player->LoadingStatus());

    }    
}

void CAvaraAppImpl::MessageLine(short index, short align) {
    SDL_Log("CAvaraAppImpl::MessageLine(%d)\n", index);
    switch(index) {
        case kmWelcome1:
        case kmWelcome2:
        case kmWelcome3:
        case kmWelcome4:
            AddMessageLine("Welcome to Avara.");
            break;
        case kmStarted:
            AddMessageLine("Starting new game.");
            break;
        case kmRestarted:
            AddMessageLine("Resuming game.");
            break;
        case kmAborted:
            AddMessageLine("Aborted.");
            break;
        case kmWin:
            AddMessageLine("Mission complete.");
            break;
        case kmGameOver:
            AddMessageLine("Game over.");
            break;
        case kmSelfDestruct:
            AddMessageLine("Self-destruct activated.");
            break;
        case kmFragmentAlert:
            AddMessageLine("ALERT: Reality fragmentation detected!");
            break;
        case kmRefusedLogin:
            AddMessageLine("Login refused.");
            break;
    }

}

std::deque<std::string>& CAvaraAppImpl::MessageLines() {
    return messageLines;
}
void CAvaraAppImpl::LevelReset() {}
void CAvaraAppImpl::ParamLine(short index, short align, StringPtr param1, StringPtr param2) {
    SDL_Log("CAvaraAppImpl::ParamLine(%d)\n", index);
    std::stringstream buffa;
    std::string a = std::string((char *)param1 + 1, param1[0]);
    std::string b;
    if (param2) b = std::string((char *)param2 + 1, param2[0]);

    switch(index) {
        case kmPaused:
            buffa << "Game paused by " << a << ".";
            break;
        case kmWaitingForPlayer:
            buffa << "Waiting for " << a << ".";
            break;
        case kmAKilledBPlayer:
            buffa << a << " killed " << b << ".";
            break;
        case kmUnavailableNote:
            buffa << a << " is busy.";
            break;
        case kmStartFailure:
            buffa << a << " wasn't ready.";
            break;
    }

    AddMessageLine(buffa.str());
}
void CAvaraAppImpl::StartFrame(long frameNum) {}

void CAvaraAppImpl::StringLine(std::string theString, short align) {
    AddMessageLine(theString.c_str());
}

void CAvaraAppImpl::ComposeParamLine(StringPtr destStr, short index, StringPtr param1, StringPtr param2) {
    ParamLine(index, 0, param1, param2);
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
