
#pragma once

class CAvaraAppImpl;
class CPlayerManager;

#include "TextCommand.h"

#include <deque>

class CommandManager {
private:
    CAvaraAppImpl *itsApp;

    std::deque<std::string> chatCommandHistory;
    std::deque<std::string>::iterator chatCommandHistoryIterator;

public:
    CommandManager(CAvaraAppImpl *theApp);

    void ExecuteMatchingCommand(std::string cmdText, CPlayerManager* player);
    void HistoryAppend(std::string chatText);
    void HistoryOlder();
    void HistoryNewer();

    //
    // eventually move some/all of the below command handlers into subclasses?
    //
    bool CommandHelp(VectorOfArgs);

    // PlayerCommands
    bool GetSetPreference(VectorOfArgs);

    // RosterCommands
    bool GoodGame(VectorOfArgs);

    // ServerCommands
    bool KickPlayer(VectorOfArgs);
    bool ToggleAwayState(VectorOfArgs);

    // LevelCommands
    bool LoadNamedLevel(VectorOfArgs);
    bool LoadRandomLevel(VectorOfArgs);

    void SendRosterMessage(std::string& message);
    void SendRosterMessage(const char* message); // arg must be null-terminated
    void SendRosterMessage(int length, char *message);
};
