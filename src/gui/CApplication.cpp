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

CApplication::CApplication(std::string title, int width, int height) :
  nanogui::Screen(nanogui::Vector2i(width, height), title) {
    char *prefPath = SDL_GetPrefPath("Avaraline", "Avara");
    jsonPath = std::string(prefPath) + "prefs.json";
    SDL_free(prefPath);

    std::ifstream in(jsonPath);
    if (in.fail()) {
        // No prefs file, use defaults.
        prefs = defaultPrefs;
    } else {
        in >> prefs;
        for (json::iterator it = defaultPrefs.begin(); it != defaultPrefs.end(); ++it) {
            if (prefs.find(it.key()) == prefs.end()) {
                // A new key was added to defaultPrefs, add it to prefs.
                prefs[it.key()] = it.value();
            }
        }
    }

    gApplication = this;
    InitContext();
    setResizeCallback([this](nanogui::Vector2i newSize) { this->WindowResized(newSize.x, newSize.y); });
}

CApplication::~CApplication() {}

void CApplication::Done() {
    std::ofstream out(jsonPath);
    out << std::setw(4) << prefs << std::endl;
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
