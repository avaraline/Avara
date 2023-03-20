#pragma once

#include <json.hpp>
#include <SDL2/SDL.h>
#include <fstream>
#include <string>

using json = nlohmann::json;

#define kYonPrefTag "shortYon"
#define kMouseSensitivityTag "mouseSensitivity"
// #define kJoystickModeTag "joystickMode"
#define kInvertYAxisTag "invertYAxis"
#define kLatencyToleranceTag "latencyTolerance"
#define kHullTypeTag "hull"
#define kFrameTimeTag "frameTime"
#define kThrottle "throttle"

// TODO: split this into separate prefs
#define kServerOptionsTag "_serverOptions"

// UDP stuff
#define kDefaultUDPPort "udpDefaultPort"
#define kDefaultClientUDPPort "udpDefaultClientPort"
#define kUDPResendPrefTag "udpResend"
#define kUDPConnectionSpeedTag "udpConnectionSpeed"

// Keyboard mapping
#define kKeyboardMappingTag "keyboard"

#define kPlayerNameTag "playerName"
#define kPlayerHullColorTag "hullColor"
#define kPlayerHullTrimColorTag "hullTrimColor"
#define kPlayerCockpitColorTag "cockpitColor"
#define kPlayerGunColorTag "gunColor"

// GL stuff
#define kWindowWidth "windowWidth"
#define kWindowHeight "windowHeight"
#define kMultiSamplesTag "multiSamples"
#define kFullScreenTag "fullscreen"
#define kFOV "fov"

// Other graphics settings
#define kColorBlindMode "colorBlindMode"
#define kWeaponSightAlpha "weaponSightAlpha"
#define kShowOldHUD "showOldHUD"
#define kShowNewHUD "showNewHUD"

// Network & Tracker
#define kLastAddress "lastAddress"
#define kServerDescription "serverDescription"
#define kServerPassword "serverPassword"
#define kClientPassword "clientPassword"
#define kTrackerRegister "trackerRegister"
#define kTrackerRegisterAddress "trackerRegisterAddress"
#define kTrackerRegisterFrequency "trackerRegisterFrequency"
#define kTrackerAddress "trackerAddress"
#define kPunchServerAddress "udpPunchServerAddress"
#define kPunchServerPort "udpPunchServerPort"
#define kPunchHoles "udpHolePunch"

// Levels
#define kRecentSets "recentSets"
#define kRecentLevels "recentLevels"

// Sound
#define kIgnoreCustomGoodySound "ignoreCustomGoodySound"
#define kSoundVolume "soundVolume"

// Key names are from https://wiki.libsdl.org/SDL_Scancode
static json defaultPrefs = {
    {kYonPrefTag, 0},
    // {kJoystickModeTag, false},
    {kInvertYAxisTag, false},
    {kMouseSensitivityTag, 0},
    {kLatencyToleranceTag, 1.0},
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
        {"loadGrenade", {"E", "Right Mouse"}},
        {"fire", {"E", "Right Mouse"}},
        {"boost", "Left Shift"},
        {"verticalMotion", "Left Ctrl"},
        {"aimForward", "2"},
        {"lookLeft", "1"},
        {"lookRight", "3"},
        {"zoomIn", "="},
        {"zoomOut", "-"},
        {"scoutView", "Tab"},
        {"scoutControl", {"2", "Up", "Down", "Left", "Right"}},
        {"spectateNext", "]"},
        {"spectatePrevious", "["},
        {"scoreboard", "/"},
        {"chatMode", "Return"},
        {"debug1", "5"},
        {"debug2", "6"}}
    },
    {kPlayerNameTag, "Unnamed Player"},
    {kPlayerHullColorTag, "default"},
    {kPlayerHullTrimColorTag, "#2b2b2b"},
    {kPlayerCockpitColorTag, "#0333ff"},
    {kPlayerGunColorTag, "#929292"},
    {kMultiSamplesTag, 0},
    {kWindowWidth, 1024},
    {kWindowHeight, 768},
    {kFullScreenTag, false},
    {kFOV, 50.0},
    {kColorBlindMode, 0},
    {kWeaponSightAlpha, 1.0},
    {kShowOldHUD, true},
    {kShowNewHUD, true},
    {kFrameTimeTag, 16},
    {kLastAddress, ""},
    {kServerDescription, ""},
    {kServerPassword, ""},
    {kClientPassword, ""},
    {kTrackerAddress, "avara.io"},
    {kTrackerRegister, 1},
    {kTrackerRegisterAddress, "avara.io"},
    {kTrackerRegisterFrequency, 5},
    {kPunchServerAddress, "avara.io"},
    {kPunchServerPort, 19555},
    {kPunchHoles, true},
    {kRecentSets, {}},
    {kRecentLevels, {}},
    {kSoundVolume, 100},
    {kIgnoreCustomGoodySound, false},
    {kThrottle, 0}
};



static std::string PrefPath() {
    char *prefPath = SDL_GetPrefPath("Avaraline", "Avara");
    std::string jsonPath = std::string(prefPath) + "prefs.json";
    SDL_free(prefPath);
    return jsonPath;
}

static inline json ReadDefaultPrefs() {
    return defaultPrefs;
}

static inline json ReadPrefs() {
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

static inline void WritePrefs(json prefs) {
    try {
        std::ostringstream oss;
        oss << std::setw(4) << prefs << std::endl;

        std::ofstream out(PrefPath());
        out << oss.str();
    }
    catch (std::exception& e)
    {
        SDL_Log("ERROR WRITING PREFS FILE=%s", e.what());
    }

}
