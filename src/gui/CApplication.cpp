#define APPLICATIONMAIN
#include "CApplication.h"

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
nanogui::Screen(nanogui::Vector2i(_prefs[kWindowWidth], _prefs[kWindowHeight]), title, true, _prefs[kFullScreenTag], 8, 8, 24, 8, 0) {
    gApplication = this;
    setResizeCallback([this](nanogui::Vector2i newSize) {
        this->WindowResized(newSize.x, newSize.y);
    });

    ColorManager::refresh(this); // Init ColorManager from prefs.
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
    ColorManager::refresh(this);
    
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

std::string ToLower(const std::string source) {
    std::string result = source;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::vector<std::string> CApplication::Matches(const std::string matchStr) {
    std::vector<std::string> results;
    std::string matchLower = ToLower(matchStr);
    for (auto& el : _prefs.items()) {
        std::string keyLower = ToLower(el.key());
        if (!el.value().is_object() && !el.value().is_array() &&
            keyLower.find(matchLower) != std::string::npos) {
            if (keyLower.length() == matchLower.length()) {
                // if it matches completely, ignore all other substring matches
                // example: "hull" matches "hull" and "hullColor" (if you want to see both, pass "hul")
                results.clear();
                results.push_back(el.key());
                break;
            } else {
                results.push_back(el.key());
            }
        }
    }
    return results;
}

bool CApplication::Update(const std::string name, std::string &value) {
    // construct json from the inputs and update the internal JSON object
    if (_prefs.at(name).is_string()) {
        // wrap string values in quotes
        value = '"' + value + '"';
    }
    try {
        json updatePref = json::parse("{ \"" + name + "\": " + value + "}");
        
        // If the type of the new value is different than the old one
        // this will easily cause a crash when reading the json
        if (_prefs[name].type_name() == updatePref[name].type_name()) {
            _prefs.update(updatePref);
            WritePrefs(_prefs);
        } else {
            SDL_Log("Type mismatch. User added type '%s' did not match existing type '%s'. Prefs were not updated.", _prefs[name].type_name(), updatePref[name].type_name());
            return false;
        }
    }
    catch (json::parse_error &ex) {
        // User typed in the command to change a pref. The value type did not match for the given pref
        SDL_Log("User input value for '%s' did not parse to the correct type.", name.c_str());
        return false;  // Did not update pref
    }
    return true;  // Successfully updated pref
}

void CApplication::SavePrefs() {
    WritePrefs(_prefs);
}
