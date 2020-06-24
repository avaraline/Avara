#pragma once

#include "CWindow.h"

class CPlayerWindow : public CWindow {
public:
    CPlayerWindow(CApplication *app);

    virtual ~CPlayerWindow();

    // Handles a command broadcasted by CApplication::BroadcastCommand. Returns true if it was actually handled.
    virtual bool DoCommand(int theCommand) override;

    virtual void PrefChanged(std::string name) override;
    
protected:
    nanogui::TextBox *nameBox;
    nanogui::ComboBox *hullBox;
};
