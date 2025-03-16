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
    std::string GetStringStatus(CPlayerManager *player);
    bool handleSDLEvent(SDL_Event &event);
    void SendRosterMessage(std::string& message);
    void SendRosterMessage(const char* message); // arg must be null-terminated
    void SendRosterMessage(size_t length, char *message);
    void NewChatLine(Str255 playerName, std::string message);
    void ChatLineDelete();
    void ResetChatPrompt();
    std::string ChatPromptFor(std::string name);
    virtual void PrefChanged(std::string name) override;
    void UpdateTags(std::string& tags);

    nanogui::Text *levelLoaded;
    nanogui::Text *levelDesigner;
    nanogui::Label *levelDescription;
    nanogui::Text *levelTags;
    nanogui::Label *chatInput;

private:
    CNetManager *theNet;
};
