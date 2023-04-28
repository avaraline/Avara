
#pragma once

class CAvaraAppImpl;

#include "TextCommand.h"
#include "CPlayerManager.h"

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
    bool ForEachSlot(VectorOfArgs vargs, std::function<bool(int)> slotCallback);
    bool KickPlayers(VectorOfArgs vargs);
    bool KickPlayer(int slot);
    bool ToggleAway(VectorOfArgs);
    bool ToggleSpectator(VectorOfArgs);
    bool TogglePresence(int slot, PresenceType, std::string);

    // LevelCommands
    bool LoadNamedLevel(VectorOfArgs);
    bool LoadRandomLevel(VectorOfArgs);

    // Coding helpers
    bool SetDebugFlag(VectorOfArgs);
};
