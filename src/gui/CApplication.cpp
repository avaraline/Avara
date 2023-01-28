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

// for now it's just a list of keys that you can turn on/off
bool CApplication::ToggleDebug(const std::string& key) {
    bool hasKey = Debug(key);
    if (hasKey) {
        debugMap.erase(key);
    } else {
        debugMap[key] = -1;  // value doesn't matter for on/off debug
    }
    return !hasKey;
}

bool CApplication::Debug(const std::string& key) {
    return (debugMap.find(key) != debugMap.end());
}

int CApplication::SetDebugValue(const std::string& key, int value) {
    return debugMap[key] = value;
}

int CApplication::DebugValue(const std::string& key) {
    if (debugMap.find(key) != debugMap.end()) {
        return debugMap[key];
    }
    return -1;
}
