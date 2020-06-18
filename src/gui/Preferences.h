#pragma once

#include <json.hpp>
#include <SDL.h>
#include <fstream>
#include <string>

using json = nlohmann::json;

#define kYonPrefTag "shortYon"
#define kMouseSensitivityTag "mouseSensitivity"
#define kLatencyToleranceTag "latencyTolerance"
#define kHullTypeTag "hull"

// TODO: split this into separate prefs
#define kServerOptionsTag "_serverOptions"

// UDP stuff
#define kDefaultUDPPort "udpDefaultPort"
#define kUDPResendPrefTag "udpResend"
#define kUDPConnectionSpeedTag "udpConnectionSpeed"

// Keyboard mapping
#define kKeyboardMappingTag "keyboard"

#define kPlayerNameTag "playerName"

// GL stuff
#define kWindowWidth "windowWidth"
#define kWindowHeight "windowHeight"
#define kMultiSamplesTag "multiSamples"
#define kFullScreenTag "fullscreen"
#define kFOV "fov"

// Network & Tracker
#define kLastAddress "lastAddress"
#define kTrackerRegister "trackerRegister"
#define kTrackerRegisterAddress "trackerRegisterAddress"
#define kTrackerRegisterFrequency "trackerRegisterFrequency"
#define kTrackerAddress "trackerAddress"

// Levels
#define kSelectedSet "selectedSet"
#define kSelectedLevel "selectedLevel"

// Key names are from https://wiki.libsdl.org/SDL_Scancode
static json defaultPrefs = {
    {kYonPrefTag, 0},
    {kMouseSensitivityTag, 0},
    {kLatencyToleranceTag, 1},
    {kHullTypeTag, 0}, // 0 = light, 1 = medium, 2 = heavy
    {kServerOptionsTag, 129}, // 1 = allow load, 128 = auto latency
    {kDefaultUDPPort, 19567},
    {kUDPResendPrefTag, 2},
    {kUDPConnectionSpeedTag, 3},
    {kKeyboardMappingTag, {
        {"forward", {"W", "Up"}},
        {"backward", {"S", "Down"}},
        {"left", {"A", "Left"}},
        {"right", {"D", "Right"}},
        {"jump", {"Space", "B", "N", "M", ","}},
        {"pause", "Escape"},
        {"abort", "0"},
        {"loadMissile", "Q"},
        {"loadGrenade", "E"},
        {"fire", "E"},
        {"boost", "Left Shift"},
        {"verticalMotion", "Left Ctrl"},
        {"viewRange", "T"},
        {"aimForward", "2"},
        {"lookLeft", "1"},
        {"lookRight", "3"},
        {"zoomIn", "="},
        {"zoomOut", "-"},
        {"scoutView", "Tab"},
        {"scoutControl", {"2", "Up", "Down", "Left", "Right"}},
        {"chatMode", "Return"},
        {"debug1", "5"},
        {"debug2", "6"}}
    },
    {kPlayerNameTag, "Unnamed Player"},
    {kMultiSamplesTag, 0},
    {kWindowWidth, 1024},
    {kWindowHeight, 768},
    {kFullScreenTag, false},
    {kFOV, 50.0},
    {kLastAddress, ""},
    {kTrackerAddress, "avara.io"},
    {kTrackerRegister, 1},
    {kTrackerRegisterAddress, "avara.io"},
    {kTrackerRegisterFrequency, 5},
    {kSelectedSet, "aa-normal"},
    {kSelectedLevel, "Alektra"}

};



static std::string PrefPath() {
    char *prefPath = SDL_GetPrefPath("Avaraline", "Avara");
    std::string jsonPath = std::string(prefPath) + "prefs.json";
    SDL_free(prefPath);
    return jsonPath;
}

static json ReadPrefs() {
    json prefs;
    std::ifstream in(PrefPath());

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
            else if (defaultPrefs[it.key()].is_object()) {
                // Check second-level objects (like keyboard map) for missing keys.
                for (json::iterator it2 = defaultPrefs[it.key()].begin(); it2 != defaultPrefs[it.key()].end(); ++it2) {
                    if (prefs[it.key()].find(it2.key()) == prefs[it.key()].end()) {
                        prefs[it.key()][it2.key()] = defaultPrefs[it.key()][it2.key()];
                    }
                }
            }
        }
    }

    return prefs;
}

static void WritePrefs(json prefs) {
    std::ofstream out(PrefPath());
    out << std::setw(4) << prefs << std::endl;
}
