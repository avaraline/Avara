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
#include "CommDefs.h"

char clearChatLine[1] = {'\x1B'};


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
    
    AddMessageLine("Welcome to Avara.");
    AddMessageLine("Type /help and press return for a list of chat commands.");

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

/*

 Chat commands

 Commands are executed by typing the command in chat and pressing return.

 Available commands:

    /help
    /h
        Display a list of commands

    /random
    /r
        Load a random level.

    /active
    /a
        Toggle active state. A player will not be loaded into a game if they are inactive.

    /load
    /l
        Load levels by name. Full level name is not required. Case insensitive.

    /pref
    /p
        Display and set preference values. usage: /p prefName prefValue

    /beep
    /b
        Make notification sound.

    /clear
    /c
        Clear chat text.

    /kick <player slot number>
    /k
        Kick player in given slot. Slots start at 1.

 */
void CAvaraAppImpl::ChatCommand(std::string chatText, CPlayerManager* player) {
    if(chatText == "/b" || chatText == "/beep") {
        NotifyUser();
        
        if(player->CalculateIsLocalPlayer()) {
            ChatCommandHistory(chatText);
        }
    }
    else if(chatText == "/c" || chatText == "/clear") {
        player->LineBuffer().clear();
        
        if(player->CalculateIsLocalPlayer()) {
            ChatCommandHistory(chatText);
        }
    }

    if(player->CalculateIsLocalPlayer()) {

        if(chatText == "/help" || chatText == "/h") {
            ChatCommandHistory(chatText);

            AddMessageLine("Execute commands from chat. Type the command and press return. Available commands:");
            AddMessageLine("    /random (load random level from matching set names or all sets)");
            AddMessageLine("    /load (load level by name, full name not required)");
            AddMessageLine("    /kick <player slot number>, /beep, /clear (clear chat text)");
            AddMessageLine("    /pref (read and write preferences), /active (toggle active)");
        }
        else if(chatText.rfind("/kick ", 0) == 0 || chatText.rfind("/k ", 0) == 0) {
            ChatCommandHistory(chatText);
            std::string slotString;
            std::stringstream chatSS(chatText);
            int slot;
            getline(chatSS, slotString, ' '); //skip "/kick"
            getline(chatSS, slotString, ' ');

            if(player->Slot() != 0) {
                AddMessageLine("Only the host can issue kick commands.");
            }
            else if(!slotString.empty() && std::all_of(slotString.begin(), slotString.end(), ::isdigit)) {
                slot = std::stoi(slotString) - 1;

                if(slot == 0) {
                    AddMessageLine("Host cannot be kicked.");
                }
                else {
                    AddMessageLine("Kicking player in slot " + slotString);
                    gameNet->itsCommManager->SendPacket(kdEveryone, kpKillConnection, slot, 0, 0, 0, NULL);
                }
            }
            else {
                AddMessageLine("Invalid Kick command.");
            }
        }
        else if(chatText.find("/r", 0) == 0 || chatText.find("/random", 0) == 0) {
            //load random level
            ChatCommandHistory(chatText);
            std::vector<std::string> levelSets = LevelDirNameListing();

            size_t matchIndex = std::min(chatText.find(" ", 0), chatText.length() - 1);
            // matchStr will be "" for "/r", and "xyz" for "/r xyz"
            std::string matchStr = chatText.substr(matchIndex+1);
            std::vector<std::string> matchingSets;
            for (auto setName : levelSets) {
                if (setName.find(matchStr, 0) != std::string::npos) {
                    matchingSets.push_back(setName);
                }
            }

            AddMessageLine((std::ostringstream() << "Choosing random level from " << matchingSets.size() << " sets").str());

            if (matchingSets.size() > 0) {
                std::random_device rd; // obtain a random number from hardware
                std::mt19937 gen(rd()); // seed the generator
                std::uniform_int_distribution<> distr(0, matchingSets.size() - 1); // define the range

                std::string set = matchingSets[distr(gen)];
                levelWindow->SelectSet(set);

                nlohmann::json levels = LoadLevelListFromJSON(set);

                std::random_device rdLevel;
                std::mt19937 genLevel(rdLevel());
                std::uniform_int_distribution<> distrLevel(0, levels.size() - 1);

                nlohmann::json jsonLevel = levels[distrLevel(genLevel)];
                std::string level = jsonLevel.at("Name");

                levelWindow->SelectLevel(set, level);
                levelWindow->SendLoad();
            }
        }
        else if(chatText.rfind("/active", 0) == 0 || chatText.rfind("/a", 0) == 0) {
            ChatCommandHistory(chatText);
            short status = kLNotPlaying;
            std::string slotString = "";
            std::string command;
            std::stringstream chatSS(chatText);
            int slot;
            getline(chatSS, command, ' '); //skip "/active"
            getline(chatSS, slotString, ' ');

            if(slotString.length() == 0 || std::all_of(slotString.begin(), slotString.end(), ::isdigit)) {
                if(slotString.length() > 0)
                    slot = std::stoi(slotString) - 1;
                else
                    slot = player->Slot();

                CPlayerManager* playerToChange = gameNet->playerTable[slot];
                std::string playerName((char *)playerToChange->PlayerName() + 1, playerToChange->PlayerName()[0]);

                if (playerName.length() > 0) {
                    if(playerToChange->LoadingStatus() == kLNotPlaying) {
                        if(itsGame->loadedLevel.length() > 0) {
                            status = kLLoaded;
                        }
                        else {
                            status = kLConnected;
                            // bug . play a game, pause. go inactive then active and status is connected isntead of paused
                        }
                    }

                    playerToChange->SetPlayerStatus(status, -1);
                    gameNet->StatusChange(slot);
                }
            }
            else {
                AddMessageLine("Invalid active command.");
            }
        }
        else if(chatText.rfind("/pref ", 0) == 0 || chatText.rfind("/p ", 0) == 0) {
            ChatCommandHistory(chatText);
            std::string pref;
            std::string value;
            std::string currentValue;
            std::stringstream chatSS(chatText);
            getline(chatSS, pref, ' '); //skip "/p "
            getline(chatSS, pref, ' ');
            getline(chatSS, value, ' ');

            try {
                currentValue = this->Get(pref).dump();
            } catch (json::out_of_range &e) {
                AddMessageLine(pref + " is not a valid preference");
                return;
            }

            if(value.length() == 0) {
                //read prefs
                AddMessageLine(pref + " = " + currentValue);
            }
            else {
                //write prefs
                nlohmann::json currentValueJSON = this->Get(pref); //get current type

                if (currentValueJSON.is_string()) {
                    this->Set(pref, value);
                }
                else if (currentValueJSON.is_number_float() || value.find('.') != std::string::npos ) {
                    nlohmann::json jsonFloat = nlohmann::json(stof(value));
                    this->Set(pref, jsonFloat);
                }
                else if (currentValueJSON.is_number_integer()) {
                    this->Set(pref, stoi(value));
                }
                else if (currentValueJSON.is_boolean()) {
                    nlohmann::json bvalue = nlohmann::json(value == "true" ? true : false);
                    this->Set(pref, bvalue);
                }

                AddMessageLine(pref + " changed from " + currentValue + " to " + value);
                CApplication::PrefChanged(pref);
            }
        }
        else if(chatText.rfind("/load ", 0) == 0 || chatText.rfind("/l ", 0) == 0) {
            ChatCommandHistory(chatText);
            std::vector<std::string> levelSets = LevelDirNameListing();
            std::string levelPrefix;
            std::stringstream chatSS(chatText);
            getline(chatSS, levelPrefix, ' '); //skip "/load "
            getline(chatSS, levelPrefix);
            std::transform(levelPrefix.begin(), levelPrefix.end(),levelPrefix.begin(), ::toupper);

            for(std::string set : levelSets) {
                nlohmann::json ledis = LoadLevelListFromJSON(set);
                for (auto &ld : ledis.items()) {
                    std::string level = ld.value()["Name"].get<std::string>();
                    std::string levelUpper = ld.value()["Name"].get<std::string>();
                    std::transform(levelUpper.begin(), levelUpper.end(),levelUpper.begin(), ::toupper);

                    if(levelUpper.rfind(levelPrefix, 0) == 0) {
                        levelWindow->SelectLevel(set, level);
                        levelWindow->SendLoad();

                        return;
                    }
                }
            }
        }
    }
}

void CAvaraAppImpl::ChatCommandHistory(std::string chatText) {
    chatCommandHistory.push_back(chatText);
    chatCommandHistoryIterator = chatCommandHistory.begin();
    historyUp = true;
}

void CAvaraAppImpl::ChatCommandHistoryDown() {
    if(chatCommandHistoryIterator != chatCommandHistory.begin()) {
        if(historyUp) {
            chatCommandHistoryIterator--;
            historyUp = false;
        }
        chatCommandHistoryIterator--;
        std::string command = *chatCommandHistoryIterator;

        rosterWindow->SendRosterMessage(1, clearChatLine);
        rosterWindow->SendRosterMessage(command.length(), const_cast<char*>(command.c_str()));
    }
}

void CAvaraAppImpl::ChatCommandHistoryUp() {
    if(chatCommandHistoryIterator != chatCommandHistory.end()) {
        historyUp = true;
        std::string command = *chatCommandHistoryIterator;
        
        rosterWindow->SendRosterMessage(1, clearChatLine);
        rosterWindow->SendRosterMessage(command.length(), const_cast<char*>(command.c_str()));
        chatCommandHistoryIterator++;
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
