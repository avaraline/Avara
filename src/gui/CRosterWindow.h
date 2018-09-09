#pragma once

#include "CApplication.h"

class CRosterWindow : public CWindow {
public:
    CRosterWindow(CApplication *app);

    virtual ~CRosterWindow();
    // Handles a command broadcasted by CApplication::BroadcastCommand. Returns true if it was actually handled.
    virtual bool DoCommand(int theCommand);
    void UpdateRoster();
    std::string GetStringStatus(short status);
};