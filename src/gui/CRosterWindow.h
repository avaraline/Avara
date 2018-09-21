#pragma once

#include "CApplication.h"
#include "CNetManager.h"

#include <nanogui/common.h>
#include <nanogui/text.h>
#include <nanogui/label.h>

class CRosterWindow : public CWindow {
public:
    CRosterWindow(CApplication *app);

    virtual ~CRosterWindow();
    // Handles a command broadcasted by CApplication::BroadcastCommand. Returns true if it was actually handled.
    virtual bool DoCommand(int theCommand) override;
    void UpdateRoster();
    std::string GetStringStatus(short status, Fixed winFrame);
    bool mouseEnterEvent(const nanogui::Vector2i &p, bool enter) override;
    bool handleSDLEvent(SDL_Event &event);
    void SendRosterMessage(int length, char *message);

    nanogui::Text *levelLoaded;
    nanogui::Text *levelDesigner;
    nanogui::Label *levelDescription;

private:
    CNetManager *theNet;
};