#include "CommandManager.h"

#include "CAvaraApp.h"
#include "CAvaraGame.h"
#include "CNetManager.h"
#include "CPlayerManager.h"
#include "CAbstractActor.h" // TeamColor
#include "CScoreKeeper.h"

#include "CommDefs.h"       // kdEveryone
#include "Resource.h"       // LevelDirNameListing
#include "Debug.h"          // Debug::methods
#include <random>           // std::random_device
#include "Tags.h"

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
        // TODO: Implement
        //itsApp->rosterWindow->SendRosterMessage(ringBell);
        return true;
    });
    TextCommand::Register(cmd);

    cmd = new TextCommand("/clear           <- clear chat text",
                          [this](VectorOfArgs vargs) -> bool {
        // TODO: Implement
        //itsApp->rosterWindow->SendRosterMessage(clearChat_utf8);
        return true;
    });
    TextCommand::Register(cmd);

    cmd = new TextCommand("/pref name       <- display value of named preference\n"
                          "/pref name value <- set value of named preference",
                          METHOD_TO_LAMBDA_VARGS(GetSetPreference));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/elo             <- display ratings of current players\n"
                          "/elo name [name] <- display ratings of named players",
                          METHOD_TO_LAMBDA_VARGS(DisplayRatings));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/gg              <- good game!\n"
                          "/gg new phrase   <- add phrase to list phrases used",
                          METHOD_TO_LAMBDA_VARGS(GoodGame));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/kick 3          <- kick player in slot 3. (numbers start at 1)\n"
                          "/kick 2 4 5      <- kick multiple players\n"
                          "/kick 5-8        <- kick range of players\n"
                          "/kick *          <- /kick 2-8",
                          METHOD_TO_LAMBDA_VARGS(KickPlayers));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/away            <- toggle my presence\n"
                          "/away slot       <- toggle presence of given slot, starting from 1",
                          METHOD_TO_LAMBDA_VARGS(ToggleAway));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/spectate        <- toggle spectator mode\n"
                          "/spectate slot   <- toggle spectator mode of slot, starting from 1",
                          METHOD_TO_LAMBDA_VARGS(ToggleSpectator));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/load chok       <- load level with name containing the letters 'chok'",
                          METHOD_TO_LAMBDA_VARGS(LoadNamedLevel));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/random           <- load random level from all available levels\n"
                          "/random aa ex     <- load random level from any set name containing either 'aa' or 'ex'\n"
                          "/random #fav      <- load random level having the 'fav' tag\n"
                          "/random aa -#koth <- load random level from 'aa' sets excluding those with the 'koth' tag",
                          METHOD_TO_LAMBDA_VARGS(LoadRandomLevel));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/vvvvv        <- clear line then output ready checkmarks \xE2\x88\x9A",
                          [this](std::string cmd) -> bool {
        std::string readyStr(clearChat_utf8);
        for (int i = 1; i < cmd.length(); i++) {
            readyStr += checkMark_utf8;  // one checkmark for each v
        }
        // TODO: Implement
        // itsApp->rosterWindow->SendRosterMessage(readyStr);
        return true;
    });
    TextCommand::Register(cmd);

    cmd = new TextCommand("/tags          <- list tags for loaded level\n"
                          "/tags foo      <- adds tag #foo to loaded level\n"
                          "/tags -foo bar <- removes tag #foo, adds tag #bar",
                          METHOD_TO_LAMBDA_VARGS(HandleTags));
    TextCommand::Register(cmd);

    cmd = new TextCommand("/teams             <- split into 2-5 teams, whichever combination is best\n"
                          "/teams r|y|b|g ... <- split into however many colored teams",
                          METHOD_TO_LAMBDA_VARGS(SplitIntoTeams));
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
    if (chatCommandHistoryIterator != chatCommandHistory.end()) {
        std::string command = *chatCommandHistoryIterator;
        //itsApp->rosterWindow->SendRosterMessage(command);
    }
}

void CommandManager::HistoryOlder() {
    // if already on the oldest element
    if (chatCommandHistoryIterator == chatCommandHistory.begin()) {
        return;
    }

    chatCommandHistoryIterator--;
    std::string command = *chatCommandHistoryIterator;
    //itsApp->rosterWindow->SendRosterMessage(command);
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
    json ggs = itsApp->Get(kGoodGamePhrases);
    if (vargs.size() > 0) {
        std::string gg = join_with(vargs, " ") + "\r";
        std::vector<std::string> ggVec;
        if (ggs.is_array()) {
            ggVec = ggs.get<std::vector<std::string>>();
        }
        ggVec.push_back(gg);
        itsApp->Set(kGoodGamePhrases, ggVec);
        itsApp->SavePrefs();
        itsApp->AddMessageLine("/gg text added to prefs");
    } else {
        // TODO: implement
        //itsApp->rosterWindow->SendRosterMessage(gg);
    }
    return true;
}

bool CommandManager::ForEachSlot(VectorOfArgs vargs, std::function<bool(int)> SlotCallback) {
    bool success = true;
    for (auto arg: vargs) {
        // handle string with various combinations of commas and dashes like "2,4-6"
        std::stringstream commaStream(arg);
        std::string commaStr;
        // split on commas first
        while (std::getline(commaStream, commaStr, ',')) {
            if (commaStr == "*") {
                // will match all slots below
                commaStr = "-";
            }
            std::stringstream dashStream(commaStr);
            std::string dashStr;
            // split on dashes
            int firstSlot = 0, lastSlot = 0;
            while (std::getline(dashStream, dashStr, '-')) {
                try {
                    if (firstSlot == 0) {
                        if (dashStr == commaStr) {
                            // example: "4"
                            firstSlot = std::stoi(dashStr);
                            lastSlot = firstSlot;
                        } else if (dashStr != "") {
                            // example: "4-" or "4-6"
                            firstSlot = std::stoi(dashStr);
                            lastSlot = kMaxAvaraPlayers;
                        } else {
                            // example: "-4" or "-"
                            firstSlot = 1;
                            lastSlot = kMaxAvaraPlayers;
                        }
                    } else {
                        lastSlot = std::stoi(dashStr);
                    }
                }
                catch (std::invalid_argument& e) {
                    // user probably entered non-numeric value
                    itsApp->AddMessageLine(
                        "Invalid slot number: " + dashStr,
                        MsgAlignment::Left,
                        MsgCategory::Error
                    );

                }
            }
            // loop over range (usually just a single value unless there's a dash)
            for (int slot = firstSlot; slot > 0 && slot <= lastSlot; slot++) {
                if (itsApp->GetNet()->playerTable[slot-1]->LoadingStatus() != kLNotConnected) {
                    success = success && SlotCallback(slot);
                } else {
                    // quietly do nothing for empty slots
                }
            }
        }
    }
    // if no slot(s) specified, send -1 as the slot and let the callback decide what to do
    if (vargs.size() == 0) {
        success = SlotCallback(-1);
    }
    return success;
}

bool CommandManager::KickPlayers(VectorOfArgs vargs) {
    return ForEachSlot(vargs, [&](int slot) -> bool {
        return KickPlayer(slot);
    });
}

bool CommandManager::KickPlayer(int slot) {

    std::string slotString(std::to_string(slot));

    if(CPlayerManagerImpl::LocalPlayer()->Slot() != 0) {
        itsApp->AddMessageLine(
            "Only the host can issue kick commands.",
            MsgAlignment::Left,
            MsgCategory::Error
        );
        return false;
    }
    else if (slot > 0 && slot <= kMaxAvaraPlayers) {
        if(slot == 1) {
            itsApp->AddMessageLine(
                "Host cannot be kicked.",
                MsgAlignment::Left,
                MsgCategory::Error
            );
        }
        else {
            itsApp->AddMessageLine("Kicking player in slot " + slotString);
            // Tell everyone to kill their connection to the kicked player.
            itsApp->GetNet()->itsCommManager->SendPacket(kdEveryone, kpKillConnection, slot-1, 0, 0, 0, NULL);
        }
    }
    else {
        itsApp->AddMessageLine(
            "Invalid Kick command.",
            MsgAlignment::Left,
            MsgCategory::Error
        );
        return false;
    }
    return true;
}

bool CommandManager::ToggleAway(VectorOfArgs vargs) {
    return ForEachSlot(vargs, [&](int slot) -> bool {
        return TogglePresence(slot, kzAway, "away");
    });
}

bool CommandManager::ToggleSpectator(VectorOfArgs vargs) {
    return ForEachSlot(vargs, [&](int slot) -> bool {
        return TogglePresence(slot, kzSpectating, "spectating");
    });
}

bool CommandManager::TogglePresence(int slot, PresenceType togglePresence, std::string stateName) {

    if (slot < 0) {
        slot = itsApp->GetNet()->itsCommManager->myId + 1;
    } else {
        if(CPlayerManagerImpl::LocalPlayer()->Slot() != 0) {
            itsApp->AddMessageLine(
                "Only the host can issue this command for other players.",
                MsgAlignment::Left,
                MsgCategory::Error
            );
            return false;
        }
    }

    CPlayerManager* playerToChange = itsApp->GetNet()->playerTable[slot-1].get();
    if(playerToChange->LoadingStatus() == kLActive || playerToChange->LoadingStatus() == kLPaused) {
        itsApp->AddMessageLine(
            "State can not be changed on players in a game.",
            MsgAlignment::Left,
            MsgCategory::Error
        );
        return false;
    }

    PresenceType newPresence = togglePresence;
    if (playerToChange->Presence() == togglePresence) {
        newPresence = kzAvailable;
    }

    FrameNumber noWinFrame = -1;
    itsApp->GetNet()->itsCommManager->SendPacket(kdEveryone, kpPlayerStatusChange,
            playerToChange->Slot(), playerToChange->LoadingStatus(), newPresence, sizeof(FrameNumber), (Ptr)&noWinFrame);
    itsApp->AddMessageLine("Status of " + playerToChange->GetPlayerName() +
                   " changed to " + std::string(newPresence == kzAvailable ? "available" : stateName));

    int newColor = (newPresence == kzAvailable) ? kNeutralTeam : kBlackTeam;
    itsApp->GetNet()->SetTeamColor(slot-1, newColor);

    return true;
}

bool CommandManager::LoadNamedLevel(VectorOfArgs vargs) {
    if (vargs.size() == 0) {
        std::string cmd = "/load";
        std::string usage = TextCommand::UsageForCommand(cmd);
        itsApp->AddMessageLine(usage);
        return false;
    }

    static int loadNumber = 0;
    std::vector<std::string> levelSets = LevelDirNameListing();
    std::string levelSubstr = join_with(vargs, " ");
    std::transform(levelSubstr.begin(), levelSubstr.end(),levelSubstr.begin(), ::toupper);

    std::vector<std::pair<std::string, std::string>> bestLevels = {};
    for(std::string set : levelSets) {
        nlohmann::json ledis = LoadLevelListFromJSON(set);
        for (auto &ld : ledis.items()) {
            std::string level = ld.value()["Name"].get<std::string>();
            std::string levelUpper = ld.value()["Name"].get<std::string>();
            std::transform(levelUpper.begin(), levelUpper.end(),levelUpper.begin(), ::toupper);
            std::string alf = ld.value()["Alf"].get<std::string>();

            // find levelSubstr anywhere within the level name
            if(levelUpper.find(levelSubstr) != std::string::npos) {
                // the shorter the match string, the better (e.g. match string "bwadi" picks "Bwadi" over "Bwadi Remix")
                if (bestLevels.empty() || level.size() < bestLevels[0].second.size()) {
                    bestLevels.clear();
                    bestLevels.push_back({set, alf});
                    SDL_Log("/load leading candidate: %s/%s\n", set.c_str(), level.c_str());
                } else if (!bestLevels.empty() && level.size() == bestLevels[0].second.size()) {
                    bestLevels.push_back({set, alf});
                    SDL_Log("/load   equal candidate: %s/%s\n", set.c_str(), level.c_str());
                } else {
                    SDL_Log("/load         not using: %s/%s\n", set.c_str(), level.c_str());
                }
            }
        }
    }
    if (!bestLevels.empty()) {
        int loadIndex = loadNumber++ % bestLevels.size();  // alternates between candidates
        //itsApp->levelWindow->SelectLevel(bestLevels[loadIndex].first, bestLevels[loadIndex].second);
        //itsApp->levelWindow->SendLoad();
        itsApp->GetNet()->SendLoadLevel(bestLevels[loadIndex].first, bestLevels[loadIndex].second);
    }
    return true;
}

bool CommandManager::LoadRandomLevel(VectorOfArgs matchArgs) {
    if (matchArgs.size() == 0) {
        matchArgs.push_back(""); // to match all sets
    } else if (matchArgs[0][0] == '-') {
        // if first arg has a - in front, assume they want to load all levels first...
        // example: /rand -#bad -#koth
        matchArgs.insert(matchArgs.begin(), "");  // matchArgs.push_front("")
    }

    std::set<Tags::LevelURL> allLevels;

    for (auto matchStr : matchArgs) {
        bool addLevels = true;
        if (matchStr[0] == '-') {
            addLevels = false;
            matchStr = matchStr.substr(1);
        } else if (matchStr[0] == '+') {
            matchStr = matchStr.substr(1);
        }
        if (matchStr[0] == '#') {
            for (auto levelUrl: Tags::GetLevelsMatchingTag(matchStr)) {
                if (addLevels) {
                    allLevels.insert(levelUrl);
                } else {
                    allLevels.erase(levelUrl);
                }
            }
        } else {
            for (auto setName : LevelDirNameListing()) {
                if (setName.find(matchStr, 0) != std::string::npos) {
                    nlohmann::json levels = LoadLevelListFromJSON(setName);
                    for (auto level : levels) {
                        if (addLevels) {
                            allLevels.insert(Tags::LevelURL(setName, level.at("Alf")));
                        } else {
                            allLevels.erase(Tags::LevelURL(setName, level.at("Alf")));
                        }
                    }
                }
            }
        }
    }

    if (allLevels.size() > 0) {
        itsApp->AddMessageLine("Choosing random level from " + std::to_string(allLevels.size()) + " levels");

        std::random_device rd; // obtain a random number from hardware
        std::mt19937 gen(rd()); // seed the generator
        std::uniform_int_distribution<> distr(0, int(allLevels.size() - 1)); // define the range
        int randomLevelIndex = distr(gen);

        // advance allLevels iterator random amount then get that element
        auto it = allLevels.cbegin();
        std::advance(it, randomLevelIndex);
        Tags::LevelURL randomLevel = *it;
        //itsApp->levelWindow->SelectLevel(randomLevel.first, randomLevel.second);
        //itsApp->levelWindow->SendLoad();
        itsApp->GetNet()->SendLoadLevel(randomLevel.first, randomLevel.second);
        return true;
    } else {
        itsApp->AddMessageLine("No matching levels found.");
    }
    return false;
}

bool CommandManager::GetSetPreference(VectorOfArgs vargs) {
    if (vargs.size() == 0) {
        itsApp->AddMessageLine("usage: /pref matchString [setValue]");
        return false;
    }

    // find all matching prefs
    std::vector<std::string> prefs = itsApp->Matches(vargs[0]);
    if (prefs.size() == 0) {
        itsApp->AddMessageLine("no prefs matching '" + vargs[0] + "'");
        return true;
    }

    std::string prefName;
    for (auto pref: prefs) {
        if (vargs.size() == 1) {
            itsApp->AddMessageLine(pref + " = " + itsApp->Get(pref).dump());
        }
        prefName = pref;
    }

    if (vargs.size() > 1) {
        if (prefs.size() > 1) {
            itsApp->AddMessageLine("only 1 preference can be updated at a time");
            return true;
        } else {
            std::string oldValue = itsApp->Get(prefName).dump();
            std::string newValue = vargs[1];
            for (int i = 2; i < vargs.size(); i++) {
                newValue += " " + vargs[i];
            }
            // update pref
            bool success = itsApp->Update(prefName, newValue);

            if (success) {
                //write prefs
                itsApp->AddMessageLine(prefName + " changed from " + oldValue + " to " + newValue);
                itsApp->CApplication::PrefChanged(prefName);
            }
        }
    }
    return true;
}

bool CommandManager::DisplayRatings(VectorOfArgs vargs) {
    if (vargs.size() == 0) {
        // there must be an easier way to get the player names...
        for (int i = 0; i < kMaxAvaraPlayers; i++) {
            CPlayerManager *mgr = itsApp->GetNet()->playerTable[i].get();
            auto name = mgr->GetPlayerName();
            if (!name.empty()) {
                vargs.push_back(name);
            }
        }
    }

    std::ostringstream os;
    int endline = 0;
    for (auto [name, rating]: itsApp->GetGame()->scoreKeeper->playerRatings->GetRatings(vargs)) {
        os << std::right << std::setw(16) << name << " = " << std::setw(4) << int(rating.rating+0.5) << " ";
        if (++endline % 2 == 0) {
            os << std::endl;
        }
    }
    itsApp->AddMessageLine(os.str());
    return true;
}

std::vector<int> TeamColorsFromArgs(VectorOfArgs vargs) {
    std::vector<int> colors;
    for (auto arg: vargs) {
        switch (std::tolower(arg[0])) {
            case 'r':
                colors.push_back(kRedTeam);
                break;
            case 'y':
                colors.push_back(kYellowTeam);
                break;
            case 'b':
                colors.push_back(kBlueTeam);
                break;
            case 'g':
                colors.push_back(kGreenTeam);
                break;
            case 'p':
                colors.push_back(kPinkTeam);
                break;
            case 'w':
                colors.push_back(kWhiteTeam);
                break;
            case '*':
            case '-':
                // reset all the colors, brightest colors go to the best players
                return {kRedTeam, kYellowTeam, kPinkTeam, kBlueTeam, kGreenTeam, kPurpleTeam, kWhiteTeam, kBlackTeam};
                break;
            default:
                colors.push_back(kBlackTeam);
                break;
        }
    }
    return colors;
}

bool CommandManager::SplitIntoTeams(VectorOfArgs vargs) {
    std::map<int, std::vector<std::string>> colorTeamMap;
    std::vector<std::string> playerNames;
    for (auto player: itsApp->GetNet()->AvailablePlayers()) {
        playerNames.push_back(player->GetPlayerName());
    }

    if (vargs.size() == 0) {
        // with no arguments try to find fairest split of 2, 3, 4 or 5 teams
        colorTeamMap = itsApp->GetGame()->scoreKeeper->playerRatings->SplitIntoBestTeams({kRedTeam, kYellowTeam, kBlueTeam, kGreenTeam, kPurpleTeam}, playerNames);

    } else {
        std::vector<int> colors = TeamColorsFromArgs(vargs);
        colorTeamMap = itsApp->GetGame()->scoreKeeper->playerRatings->SplitIntoTeams(colors, playerNames);
    }

    itsApp->GetNet()->ChangeTeamColors(colorTeamMap);
    return true;
}


bool CommandManager::HandleTags(VectorOfArgs vargs) {
    Tags::LevelURL curLevel(itsApp->GetGame()->loadedSet,
                            itsApp->GetGame()->loadedLevel);

    if (vargs.size() > 0) {
        for (auto tag: vargs) {
            if (tag[0] == '-') {
                tag = tag.substr(1);
                Tags::DeleteTagFromLevel(curLevel, tag);
            } else {
                if (tag[0] == '+') {
                    tag = tag.substr(1);
                }
                Tags::AddTagToLevel(curLevel, tag);
            }
        }
    }

    std::string msg = "tags for \"" + curLevel.first + "/" + curLevel.second + "\":";
    for (auto tag: Tags::GetTagsForLevel(curLevel)) {
        msg +=  " " + tag;
    }
    itsApp->AddMessageLine(msg);

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
        os << "Debugging flag '" << key << "' is " << (dbg ? "ON" : "OFF");
    } else {
        int val = Debug::SetValue(key, std::stoi(vargs[1]));
        os << "Debugging flag '" << key << "' = " << val;
    }
    itsApp->AddMessageLine(os.str());
    return true;
}
