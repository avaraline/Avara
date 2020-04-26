#define APPLICATIONMAIN
#include "CApplication.h"

#include "AvaraGL.h"
#include "Preferences.h"
#include "Types.h"

#include <SDL.h>
#include <fstream>
#include <nanogui/nanogui.h>
#include <string>
#include <vector>


json prefs = ReadPrefs();


CApplication::CApplication(std::string title) :
nanogui::Screen(nanogui::Vector2i(prefs[kWindowWidth], prefs[kWindowHeight]), title, true, prefs[kFullScreenTag], 8, 8, 24, 8, prefs[kMultiSamplesTag]) {
    gApplication = this;
    AvaraGLInitContext();
    setResizeCallback([this](nanogui::Vector2i newSize) { this->WindowResized(newSize.x, newSize.y); });
}

CApplication::~CApplication() {}

void CApplication::Done() {
    prefs[kWindowWidth] = mSize[0];
    prefs[kWindowHeight] = mSize[1];
    WritePrefs(prefs);
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

std::string CApplication::String(const std::string name) {
    return prefs[name];
}

long CApplication::Number(const std::string name) {
    return prefs[name];
}

json CApplication::Get(const std::string name) {
    return prefs[name];
}

void CApplication::Set(const std::string name, const std::string value) {
    prefs[name] = value;
    PrefChanged(name);
}

void CApplication::Set(const std::string name, long value) {
    prefs[name] = value;
    PrefChanged(name);
}

void CApplication::Set(const std::string name, json value) {
    prefs[name] = value;
    PrefChanged(name);
}
