#pragma once

#include "Types.h"

#include <SDL2/SDL.h>
#include <nanogui/nanogui.h>
#include <string>
#include <vector>

class CApplication;

class CWindow : public nanogui::Window {
public:
    CWindow(CApplication *app, const std::string &title);

    virtual ~CWindow();

    // Handles a command broadcasted by CApplication::BroadcastCommand. Returns true if it was actually handled.
    virtual bool DoCommand(int theCommand) { return false; }

    // Called when an applcation preference value changes.
    virtual void PrefChanged(std::string name) {}
    
    virtual bool editing() { return false; }

    virtual void restoreState();
    virtual void saveState();

protected:
    CApplication *mApplication;
};
