#pragma once

#include "CWindow.h"

class CTrackerWindow : public CWindow {
public:
    CTrackerWindow(CApplication *app);

    virtual ~CTrackerWindow();

    // Handles a command broadcasted by CApplication::BroadcastCommand. Returns true if it was actually handled.
    virtual bool DoCommand(int theCommand);

    void Query();
    
    virtual bool editing();

protected:
    nanogui::TextBox *addressBox;
	nanogui::Widget *resultsBox;
    nanogui::Label *resultsLabel;
};
