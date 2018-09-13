#define APPLICATIONMAIN
#include "CApplication.h"

#include "AvaraGL.h"
#include "Preferences.h"
#include "Types.h"

#include <SDL2/SDL.h>
#include <fstream>
#include <nanogui/nanogui.h>
#include <string>
#include <vector>


json prefs = ReadPrefs();


CApplication::CApplication(std::string title, int width, int height) :
nanogui::Screen(nanogui::Vector2i(width, height), title, true, false, 8, 8, 24, 8, prefs[kMultiSamplesTag]) {
    gApplication = this;
    InitContext();
    setResizeCallback([this](nanogui::Vector2i newSize) { this->WindowResized(newSize.x, newSize.y); });
}

CApplication::~CApplication() {}

void CApplication::Done() {
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

void CApplication::drawContents() {
    Idle();
    DrawContents();
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
