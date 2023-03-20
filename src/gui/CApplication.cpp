#define APPLICATIONMAIN
#include "CApplication.h"

#include "AvaraGL.h"
#include "ColorManager.h"
#include "Preferences.h"

#include <SDL2/SDL.h>
#include <fstream>
#include <nanogui/nanogui.h>
#include <string>
#include <vector>

json CApplication::_prefs = ReadPrefs();
json CApplication::_defaultPrefs = ReadDefaultPrefs();

CApplication::CApplication(std::string title) :
nanogui::Screen(nanogui::Vector2i(_prefs[kWindowWidth], _prefs[kWindowHeight]), title, true, _prefs[kFullScreenTag], 8, 8, 24, 8, _prefs[kMultiSamplesTag]) {
    gApplication = this;
    AvaraGLInitContext();
    setResizeCallback([this](nanogui::Vector2i newSize) { this->WindowResized(newSize.x, newSize.y); });

    ColorManager::setColorBlind(CApplication::Get(kColorBlindMode));
    ColorManager::setHudAlpha(CApplication::Get(kWeaponSightAlpha));
}

CApplication::~CApplication() {}

void CApplication::Unregister(CWindow *win) {
    std::vector<CWindow*>::iterator position = std::find(windowList.begin(), windowList.end(), win);
    if (position != windowList.end())
        windowList.erase(position);
}

void CApplication::Done() {
    for (auto win : windowList) {
        win->saveState();
    }

    _prefs[kWindowWidth] = mSize[0];
    _prefs[kWindowHeight] = mSize[1];
    _prefs.erase(kDefaultClientUDPPort);  // don't save client port
    WritePrefs(_prefs);
}

void CApplication::BroadcastCommand(int theCommand) {
    if (!DoCommand(theCommand)) {
        for (auto win : windowList) {
            if (win->DoCommand(theCommand))
                break;
        }
    }
}

void CApplication::PrefChanged(std::string name) {
    for (auto win : windowList) {
        win->PrefChanged(name);
    }
}

bool CApplication::handleSDLEvent(SDL_Event &event) {
    // By default, give nanogui first crack at events.
    bool handled = nanogui::Screen::handleSDLEvent(event);
    if (!handled) {
        handled = HandleEvent(event);
    }
    return handled;
}

long CApplication::Number(const std::string name, const long defaultValue) {
    if (_prefs[name].is_number()) {
        return _prefs[name];
    }
    return defaultValue;
}
