
#include "CommandManager.h"

#include "CAvaraApp.h"
#include "CAvaraGame.h"
#include "CPlayerManager.h"

#include "CommDefs.h"       // kdEveryone
#include "Resource.h"       // LevelDirNameListing
#include "Debug.h"          // Debug::methods
#include <random>           // std::random_device

CommandManager::CommandManager(CAvaraAppImpl *theApp) : itsApp(theApp) {

    // register commands
    TextCommand* cmd;
    cmd = new TextCommand("/help            <- show list of all commands\n"
                          "/help /xyz       <- show help for command /xyz",
                          METHOD_TO_LAMBDA_VARGS(CommandHelp));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/beep            <- ring the bell",
                          [this](VectorOfArgs vargs) -> bool {
        std::string ringBell("\a\b");    // \a = bell, \b = backspace
        itsApp->rosterWindow->SendRosterMessage(ringBell);
        return true;
    });
    TextCommand::Register(cmd);

    cmd = new TextCommand("/clear           <- clear chat text",
                          [this](VectorOfArgs vargs) -> bool {
        itsApp->rosterWindow->SendRosterMessage(clearChat_utf8);
        return true;
    });
    TextCommand::Register(cmd);

    cmd = new TextCommand("/pref name       <- display value of named preference\n"
                          "/pref name value <- set value of named preference",
                          METHOD_TO_LAMBDA_VARGS(GetSetPreference));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/gg              <- good game!\n"
                          "/gg new phrase   <- change the /gg phrase",
                          METHOD_TO_LAMBDA_VARGS(GoodGame));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/kick slot       <- kick player in given slot. slots start at 1",
                          METHOD_TO_LAMBDA_VARGS(KickPlayer));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/away            <- toggle my presence\n"
                          "/away slot       <- toggle presence of given slot, starting from 1",
                          METHOD_TO_LAMBDA_VARGS(ToggleAwayState));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/load chok       <- load level with name containing the letters 'chok'",
                          METHOD_TO_LAMBDA_VARGS(LoadNamedLevel));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/random       <- load random level from all available levels\n"
                          "/random aa ex <- load random level from any set name containing either 'aa' or 'ex'",
                          METHOD_TO_LAMBDA_VARGS(LoadRandomLevel));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/vvvvv        <- clear line then output ready checkmarks \xE2\x88\x9A",
                          [this](std::string cmd) -> bool {
        std::string readyStr(clearChat_utf8);
        for (int i = 1; i < cmd.length(); i++) {
            readyStr += checkMark_utf8;  // one checkmark for each v
        }
        itsApp->rosterWindow->SendRosterMessage(readyStr);
        return true;
    });
    TextCommand::Register(cmd);

    cmd = new TextCommand("/dbg flag     <- toggles named debugging flag on/off\n"
                          "/dbg flag val <- sets debug flag to integer value",
                          METHOD_TO_LAMBDA_VARGS(SetDebugFlag));
    TextCommand::Register(cmd);
}


//  Commands are executed by typing the command in chat and pressing return.
//  See RegisterCommands() below for a list of which commands are supported.
void CommandManager::ExecuteMatchingCommand(std::string chatText, CPlayerManager* player) {
    // only execute commands for the "local" player (me)
    if (player->IsLocalPlayer() && TextCommand::ExecuteMatchingCallbacks(chatText)) {
        // remember this command in case we want to use the arrow keys to run it again
        HistoryAppend(chatText);
        return;
    }
}

//
//  Commands history
//

void CommandManager::HistoryAppend(std::string chatText) {
    // only add it if it's different than the newest history element
    if (chatCommandHistory.empty() || chatCommandHistory.back().compare(chatText) != 0) {
        chatCommandHistory.push_back(chatText);
        if (chatCommandHistory.size() > 20) { // 20 is plenty?
            // remove oldest history element
            chatCommandHistory.pop_front();
        }
    }
    // reset the history iterator after any command is executed
    chatCommandHistoryIterator = chatCommandHistory.end();
}

void CommandManager::HistoryNewer() {
    // end() indicates we aren't displaying history so there's no "newer"
    if (chatCommandHistoryIterator == chatCommandHistory.end()) {
        return;
    }

    chatCommandHistoryIterator++;
    itsApp->rosterWindow->SendRosterMessage(clearChat_utf8);
    if (chatCommandHistoryIterator != chatCommandHistory.end()) {
        std::string command = *chatCommandHistoryIterator;
        itsApp->rosterWindow->SendRosterMessage(command);
    }
}

void CommandManager::HistoryOlder() {
    // if already on the oldest element
    if (chatCommandHistoryIterator == chatCommandHistory.begin()) {
        return;
    }

    chatCommandHistoryIterator--;
    std::string command = *chatCommandHistoryIterator;
    itsApp->rosterWindow->SendRosterMessage(clearChat_utf8);
    itsApp->rosterWindow->SendRosterMessage(command);
}

//
// Slash-command callbacks
//

bool CommandManager::CommandHelp(VectorOfArgs vargs) {
    if (vargs.size() == 0) {
        itsApp->AddMessageLine("Type '/help /xyz' to get further info about the command matching '/xyz'");
        itsApp->AddMessageLine("Available commands:  " + TextCommand::ListOfCommands());
    } else {
        TextCommand::FindMatchingCommands(vargs[0],
                                          [&](TextCommand* command, std::string cmd, VectorOfArgs vargs) -> bool {
            itsApp->AddMessageLine(command->GetUsage());
            return true;
        });
    }
    return true;
}

bool CommandManager::GoodGame(VectorOfArgs vargs) {
    static std::string gg("Well played sir or madam! gg!\r");
    if (vargs.size() > 0) {
        gg = join_with(vargs, " ") + "\r";
        itsApp->AddMessageLine("/gg text changed to: " + gg);
    } else {
        itsApp->rosterWindow->SendRosterMessage(gg);
    }
    return true;
}

bool CommandManager::KickPlayer(VectorOfArgs vargs) {
    std::string slotString(vargs[0]);

    if(CPlayerManagerImpl::LocalPlayer()->Slot() != 0) {
        itsApp->AddMessageLine(
            "Only the host can issue kick commands.",
            MsgAlignment::Left,
            MsgCategory::Error
        );
    }
    else if(!slotString.empty() && std::all_of(slotString.begin(), slotString.end(), ::isdigit)) {
        int slot = std::stoi(slotString) - 1;

        if(slot == 0) {
            itsApp->AddMessageLine(
                "Host cannot be kicked.",
                MsgAlignment::Left,
                MsgCategory::Error
            );
        }
        else {
            itsApp->AddMessageLine("Kicking player in slot " + slotString);
            // Tell everyone to kill their connection to the kicked player.
            itsApp->GetNet()->itsCommManager->SendPacket(kdEveryone, kpKillConnection, slot, 0, 0, 0, NULL);
        }
    }
    else {
        itsApp->AddMessageLine(
            "Invalid Kick command.",
            MsgAlignment::Left,
            MsgCategory::Error
        );
    }
    return true;
}

bool CommandManager::ToggleAwayState(VectorOfArgs vargs) {
    short slot = itsApp->GetNet()->itsCommManager->myId;

    if (vargs.size() > 0) {
        std::string slotString(vargs[0]);
        if(slotString.length() > 0 && std::all_of(slotString.begin(), slotString.end(), ::isdigit)) {
            if(CPlayerManagerImpl::LocalPlayer()->Slot() != 0) {
                itsApp->AddMessageLine(
                    "Only the host can issue active commands for other players.",
                    MsgAlignment::Left,
                    MsgCategory::Error
                );
                return false;
            }
            slot = std::stoi(slotString) - 1;
        } else {
            itsApp->AddMessageLine(
                "No player found with slot = " + slotString,
                MsgAlignment::Left,
                MsgCategory::Error
            );
            return false;
        }
    }

    CPlayerManager* playerToChange = itsApp->GetNet()->playerTable[slot];
    if(playerToChange->LoadingStatus() == kLActive || playerToChange->LoadingStatus() == kLPaused) {
        itsApp->AddMessageLine(
            "State can not be changed on players in a game.",
            MsgAlignment::Left,
            MsgCategory::Error
        );
        return false;
    }

    short newStatus = (playerToChange->LoadingStatus() == kLAway) ? kLConnected : kLAway;
    FrameNumber noWinFrame = -1;
    itsApp->GetNet()->itsCommManager->SendPacket(kdEveryone, kpPlayerStatusChange,
                                        playerToChange->Slot(), newStatus, 0, sizeof(noWinFrame), (Ptr)&noWinFrame);
    itsApp->AddMessageLine("Status of " + playerToChange->GetPlayerName() +
                   " changed to " + std::string(newStatus == kLConnected ? "available" : "away"));
    return true;
}

bool CommandManager::LoadNamedLevel(VectorOfArgs vargs) {
    std::vector<std::string> levelSets = LevelDirNameListing();
    std::string levelSubstr = join_with(vargs, " ");
    std::transform(levelSubstr.begin(), levelSubstr.end(),levelSubstr.begin(), ::toupper);

    for(std::string set : levelSets) {
        nlohmann::json ledis = LoadLevelListFromJSON(set);
        for (auto &ld : ledis.items()) {
            std::string level = ld.value()["Name"].get<std::string>();
            std::string levelUpper = ld.value()["Name"].get<std::string>();
            std::transform(levelUpper.begin(), levelUpper.end(),levelUpper.begin(), ::toupper);

            // find levelSubstr anywhere within the level name
            if(levelUpper.find(levelSubstr) != std::string::npos) {
                itsApp->levelWindow->SelectLevel(set, level);
                itsApp->levelWindow->SendLoad();

                return true;
            }
        }
    }
    return false;
}

bool CommandManager::LoadRandomLevel(VectorOfArgs matchArgs) {
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

    itsApp->AddMessageLine("Choosing random level from " + std::to_string(matchingSets.size()) + " sets");

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
                itsApp->levelWindow->SelectSet(setName);
                nlohmann::json randomLevel = levels[randomlevelIndex - previousCount];
                //itsApp->AddMessageLine((std::ostringstream() << "LoadRandomLevel i=" << randomlevelIndex << " cur=" << currentCount
                //                << " prev=" << previousCount << " index=" << randomlevelIndex - previousCount).str());

                std::string levelName = randomLevel.at("Name");
                itsApp->levelWindow->SelectLevel(setName, levelName);
                itsApp->levelWindow->SendLoad();
                return true;
            }

            previousCount = currentCount;
        }
    }
    return false;
}

bool CommandManager::GetSetPreference(VectorOfArgs vargs) {
    std::string pref(vargs.size() > 0 ? vargs[0] : "");
    std::string value(vargs.size() > 1 ? vargs[1] : "");
    std::string currentValue;
    try {
        currentValue = itsApp->Get(pref).dump();
    } catch (json::out_of_range &e) {
        itsApp->AddMessageLine(pref + " is not a valid preference");
        return false;
    }
    if(value.length() == 0) {
        //read prefs
        itsApp->AddMessageLine(pref + " = " + currentValue);
    }
    else {
        //write prefs
        nlohmann::json currentValueJSON = itsApp->Get(pref); //get current type
        if (currentValueJSON.is_string()) {
            itsApp->Set(pref, value);
        }
        else if (currentValueJSON.is_number_float() || value.find('.') != std::string::npos ) {
            nlohmann::json jsonFloat = nlohmann::json(stof(value));
            itsApp->Set(pref, jsonFloat);
        }
        else if (currentValueJSON.is_number_integer()) {
            itsApp->Set(pref, stoi(value));
        }
        else if (currentValueJSON.is_boolean()) {
            nlohmann::json bvalue = nlohmann::json(value == "true" ? true : false);
            itsApp->Set(pref, bvalue);
        }
        itsApp->AddMessageLine(pref + " changed from " + currentValue + " to " + value);
        itsApp->CApplication::PrefChanged(pref);
    }
    return true;
}

bool CommandManager::SetDebugFlag(VectorOfArgs vargs) {
    if (vargs.size() == 0) {
        itsApp->AddMessageLine("Need to specify a key");
        return false;
    }
    auto key = vargs[0];
    std::ostringstream os;
    if (vargs.size() == 1) {
        bool dbg = Debug::Toggle(key);
        os << "Debugging flag " << key << " is " << (dbg ? "ON" : "OFF");
    } else {
        int val = Debug::SetValue(key, std::stoi(vargs[1]));
        os << "Debugging flag " << key << " = " << val;
    }
    itsApp->AddMessageLine(os.str());
    return true;
}
