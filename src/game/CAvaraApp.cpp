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

#include "TextCommand.h"

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

    // specify which slash-commands will be handled
    RegisterCommands();

    AddMessageLine("Welcome to Avara.");
    AddMessageLine("Type /help and press return for a list of chat commands.");

    // load up a random decent starting level
    ChatCommand("/rand -normal -tre -strict emo", CPlayerManagerImpl::LocalPlayer());
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

void CAvaraAppImpl::AddMessageLine(std::string lines) {
    std::istringstream iss(lines);
    std::string line;
    // split string on newlines
    while(std::getline(iss, line)) {
        SDL_Log("Message: %s", line.c_str());
        messageLines.push_back(line);
        if (messageLines.size() > 5) {
            messageLines.pop_front();
        }
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


//
//  Chat commands
//
//  Commands are executed by typing the command in chat and pressing return.
//  See RegisterCommands() below for a list of which commands are supported.
void CAvaraAppImpl::ChatCommand(std::string chatText, CPlayerManager* player) {
    // only execute commands for the "local" player (me)
    if (player->IsLocalPlayer() && TextCommand::ExecuteMatchingCallbacks(chatText)) {
        // remember this command in case we want to use the keys to do it again
        ChatCommandHistory(chatText);
        return;
    }
}

void CAvaraAppImpl::ChatCommandHistory(std::string chatText) {
    // only add it if it's different than the front
    if (chatCommandHistory.empty() || chatCommandHistory.front().compare(chatText) != 0) {
        chatCommandHistory.push_front(chatText);
    }
    chatCommandHistoryIterator = chatCommandHistory.begin();
    historyUp = true;
}

void CAvaraAppImpl::ChatCommandHistoryDown() {
    if(historyCleared == false) {
        if(chatCommandHistoryIterator == chatCommandHistory.begin()) {
            rosterWindow->SendRosterMessage(1, clearChatLine);
            if(historyCleared == false)
                chatCommandHistoryIterator--;

            historyCleared = true;
        }
        else {
            if(historyUp == true) {
                chatCommandHistoryIterator--;
            }
            historyUp = false;

            if(chatCommandHistoryIterator != chatCommandHistory.begin()) {
                chatCommandHistoryIterator--;
            }
            std::string command = *chatCommandHistoryIterator;

            rosterWindow->SendRosterMessage(1, clearChatLine);
            rosterWindow->SendRosterMessage(command.length(), const_cast<char*>(command.c_str()));
        }
    }
}

void CAvaraAppImpl::ChatCommandHistoryUp() {
    if(chatCommandHistoryIterator != chatCommandHistory.end()) {
        if(historyUp == false || historyCleared == true) {
            chatCommandHistoryIterator++;
        }
        historyUp = true;
        historyCleared = false;
        std::string command = *chatCommandHistoryIterator;

        rosterWindow->SendRosterMessage(1, clearChatLine);
        rosterWindow->SendRosterMessage(command.length(), const_cast<char*>(command.c_str()));

        if(chatCommandHistoryIterator != chatCommandHistory.end()) {
            chatCommandHistoryIterator++;
        }
    }
}

//
// Slash-command callbacks
//

bool CAvaraAppImpl::CommandHelp(VectorOfArgs vargs) {
    if (vargs.size() == 0) {
        AddMessageLine("Type '/help /xyz' to get further info about the command matching '/xyz'");
        AddMessageLine("Available commands:  " + TextCommand::ListOfCommands());
    } else {
        TextCommand::FindMatchingCommands(vargs[0],
                                          [&](TextCommand* command, VectorOfArgs vargs) -> bool {
            AddMessageLine(command->GetUsage());
            return true;
        });
    }
    return true;
}

bool CAvaraAppImpl::GoodGame(VectorOfArgs vargs) {
    static std::string gg("Well played sir or madam! gg!\r");
    if (vargs.size() > 0) {
        gg = join_with(vargs, " ") + "\r";
        AddMessageLine("/gg text changed to: " + gg);
    } else {
        rosterWindow->SendRosterMessage(gg);
    }
    return true;
}

bool CAvaraAppImpl::KickPlayer(VectorOfArgs vargs) {
    std::string slotString;
    std::stringstream chatSS(vargs[0]);
    int slot;
    getline(chatSS, slotString, ' '); //skip "/kick"
    getline(chatSS, slotString, ' ');

    if(CPlayerManagerImpl::LocalPlayer()->Slot() != 0) {
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
    return true;
}

bool CAvaraAppImpl::ToggleActiveState(VectorOfArgs vargs) {
    short status;
    std::string slotString = vargs[0];
    std::string command;
    int slot;
    if(slotString.length() == 0 || std::all_of(slotString.begin(), slotString.end(), ::isdigit)) {
        if(slotString.length() > 0)
            slot = std::stoi(slotString) - 1;
        else
            slot = CPlayerManagerImpl::LocalPlayer()->Slot();
        CPlayerManager* playerToChange = gameNet->playerTable[slot];
        std::string playerName((char *)playerToChange->PlayerName() + 1, playerToChange->PlayerName()[0]);
        if(playerToChange->LoadingStatus() == kLActive || playerToChange->LoadingStatus() == kLPaused) {
            AddMessageLine("Active command can not be used on players in a game.");
        }
        else if (playerName.length() > 0) {
            playerToChange->SetActive(!playerToChange->Active());
            gameNet->ValueChange(slot, "active", playerToChange->Active());
        }
    }
    else {
        AddMessageLine("Invalid active command.");
    }
    return true;
}

bool CAvaraAppImpl::LoadNamedLevel(VectorOfArgs vargs) {
    std::vector<std::string> levelSets = LevelDirNameListing();
    std::string levelPrefix = vargs[0];
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

                return true;
            }
        }
    }
    return false;
}

bool CAvaraAppImpl::LoadRandomLevel(VectorOfArgs matchArgs) {
    if (matchArgs.size() == 0) {
        matchArgs.push_back(""); // to match all sets
    }
    // std::cout << "LoadRandomLevel matchArgs[0] = " << matchArgs[0] << std::endl;

    std::vector<std::string> levelSets = LevelDirNameListing();
    std::vector<std::string> matchingSets;
    for (auto matchStr : matchArgs) {
        for (auto setName : levelSets) {
            if (setName.find(matchStr, 0) != std::string::npos) {
                matchingSets.push_back(setName);
            }
        }
    }

    AddMessageLine((std::ostringstream() << "Choosing random level from " << matchingSets.size() << " sets").str());

    if (matchingSets.size() > 0) {
        //count levels
        int levelCount = 0;
        for (auto setName : matchingSets) {
            nlohmann::json levels = LoadLevelListFromJSON(setName);
            levelCount += levels.size();
        }

        std::random_device rd; // obtain a random number from hardware
        std::mt19937 gen(rd()); // seed the generator
        std::uniform_int_distribution<> distr(0, levelCount - 1); // define the range
        int randomlevelIndex = distr(gen);

        //load level
        int currentCount = 0;
        int previousCount = 0;
        for (auto setName : matchingSets) {
            nlohmann::json levels = LoadLevelListFromJSON(setName);
            currentCount += levels.size();

            if(randomlevelIndex >= previousCount && randomlevelIndex < currentCount) {
                levelWindow->SelectSet(setName);
                nlohmann::json randomLevel = levels[randomlevelIndex - previousCount];
                //AddMessageLine((std::ostringstream() << "LoadRandomLevel i=" << randomlevelIndex << " cur=" << currentCount
                //                << " prev=" << previousCount << " index=" << randomlevelIndex - previousCount).str());

                std::string levelName = randomLevel.at("Name");
                levelWindow->SelectLevel(setName, levelName);
                levelWindow->SendLoad();
                return true;
            }

            previousCount = currentCount;
        }
    }
    return false;
}

bool CAvaraAppImpl::GetSetPreference(VectorOfArgs vargs) {
    std::string pref(vargs.size() > 0 ? vargs[0] : "");
    std::string value(vargs.size() > 1 ? vargs[1] : "");
    std::string currentValue;
    try {
        currentValue = this->Get(pref).dump();
    } catch (json::out_of_range &e) {
        AddMessageLine(pref + " is not a valid preference");
        return false;
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
    return true;
}

void CAvaraAppImpl::RegisterCommands() {
    TextCommand* cmd;
    cmd = new TextCommand("/help            <- show list of all commands\n"
                          "/help /xyz       <- show help for command /xyz",
                          METHOD_TO_LAMBDA(CAvaraAppImpl::CommandHelp));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/beep            <- ring the bell",
                          [this](VectorOfArgs vargs) -> bool {
        NotifyUser();
        return true;
    });
    TextCommand::Register(cmd);

    cmd = new TextCommand("/clear           <- clear chat text",
                          [this](VectorOfArgs vargs) -> bool {
        rosterWindow->SendRosterMessage(1, clearChatLine);
        return true;
    });
    TextCommand::Register(cmd);

    cmd = new TextCommand("/pref name       <- display value of named preference\n"
                          "/pref name value <- set value of named preference",
                          METHOD_TO_LAMBDA(CAvaraAppImpl::GetSetPreference));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/gg              <- good game!\n"
                          "/gg new phrase   <- change the /gg phrase",
                          METHOD_TO_LAMBDA(CAvaraAppImpl::GoodGame));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/kick slot       <- kick player in given slot. slots start at 1",
                          METHOD_TO_LAMBDA(CAvaraAppImpl::KickPlayer));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/active           <- toggle my active/playing state\n"
                          "/active slot      <- toggle active state for given player. slots start at 1",
                          METHOD_TO_LAMBDA(CAvaraAppImpl::ToggleActiveState));
    // TextCommand::Register(cmd); // needs more debugging

    cmd = new TextCommand("/load chok        <- load level with name containing the letters 'chok'",
                          METHOD_TO_LAMBDA(CAvaraAppImpl::LoadNamedLevel));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/random       <- load random level from all available levels\n"
                          "/random aa ex <- load random level from any set name containing either 'aa' or 'ex'",
                          METHOD_TO_LAMBDA(CAvaraAppImpl::LoadRandomLevel));
    TextCommand::Register(cmd);
}
