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
#define kSpawnOrder "spawnOrder"

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
#define kIgnoreCustomColorsTag "ignoreCustomColors"

// GL stuff
#define kWindowWidth "windowWidth"
#define kWindowHeight "windowHeight"
#define kFullScreenTag "fullscreen"
#define kFOV "fov"
#define kFXAA "fxaa"
#define kUseLegacyRenderer "useLegacyRenderer"

// Other graphics settings
#define kColorBlindMode "colorBlindMode"
#define kHUDColor "hudColor"
#define kHUDPositiveColor "hudPositiveColor"
#define kHUDWarningColor "hudWarningColor"
#define kHUDCriticalColor "hudCriticalColor"
#define kHUDAlpha "hudAlpha"
#define kHUDPreset "hudPreset"
#define kHUDInertia "hudInertia"
#define kHUDArrowStyle "hudArrowStyle"
#define kHUDArrowScale "hudArrowScale"
#define kHUDArrowDistance "hudArrowDistance"
#define kHUDShowMissileLock "hudShowMissileLock"
#define kHUDShowShieldGauge "hudShowShieldGauge"
#define kHUDShowEnergyGauge "hudShowEnergyGauge"
#define kHUDShowBoosterCount "hudShowBoosterCount"
#define kHUDShowGrenadeCount "hudShowGrenadeCount"
#define kHUDShowMissileCount "hudShowMissileCount"
#define kHUDShowLivesCount "hudShowLivesCount"
#define kHUDShowLevelMessages "hudShowLevelMessages"
#define kHUDShowSystemMessages "hudShowSystemMessages"
#define kHUDShowPlayerList "hudShowPlayerList"
#define kHUDShowDirArrow "hudShowDirArrow"
#define kHUDShowScore "hudShowScore"
#define kHUDShowTime "hudShowTime"
#define kHUDShowKillFeed "hudShowKillFeed"
#define kShowNewHUD "showNewHUD"
#define kIgnoreLevelCustomColorsTag "ignoreLevelCustomColors"

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

// other
#define kGoodGamePhrases "ggs"
#define kShowElo "showElo"
#define kDefaultArgs "defaultArgs"

// editing tools
#define kEditingToolsDispatcherPort 19569

// Key names are from https://wiki.libsdl.org/SDL_Scancode
static json defaultPrefs = {
    {kYonPrefTag, 0},
    // {kJoystickModeTag, false},
    {kInvertYAxisTag, false},
    {kMouseSensitivityTag, 0},
    {kLatencyToleranceTag, 2.5},  // 2.5 = max for auto latency
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
        {"toggleFreeCam", "Space"},
        {"scoutView", "Tab"},
        {"scoutControl", {"2", "Up", "Down", "Left", "Right"}},
        {"spectateNext", "]"},
        {"spectatePrevious", "["},
        {"scoreboard", "/"},
        {"chatMode", "Return"},
        {"freeCamUp", "R"},
        {"freeCamDown", "F"},
        {"debug1", "5"},
        {"debug2", "6"}}
    },
    {kPlayerNameTag, "Unnamed Player"},
    {kPlayerHullColorTag, "default"},
    {kPlayerHullTrimColorTag, "#2b2b2b"},
    {kPlayerCockpitColorTag, "#0333ff"},
    {kPlayerGunColorTag, "#929292"},
    {kWindowWidth, 1024},
    {kWindowHeight, 768},
    {kFullScreenTag, false},
    {kFOV, 50.0},
    {kFXAA, true},
    {kColorBlindMode, 0},
    {kHUDColor, "#03f5f5"},
    {kHUDPositiveColor, "#51e87e"},
    {kHUDWarningColor, "#edd62d"},
    {kHUDCriticalColor, "#fa1313"},
    {kHUDAlpha, 1.0},
    {kHUDPreset, 2},
    {kHUDInertia, 1.0},
    {kHUDArrowScale, 1.0},
    {kHUDArrowDistance, 8.0},
    {kHUDArrowStyle, 2},
    {kHUDShowMissileLock, true},
    {kHUDShowShieldGauge, true},
    {kHUDShowEnergyGauge, true},
    {kHUDShowBoosterCount, true},
    {kHUDShowGrenadeCount, true},
    {kHUDShowMissileCount, true},
    {kHUDShowLivesCount, true},
    {kHUDShowSystemMessages, true},
    {kHUDShowLevelMessages, true},
    {kHUDShowPlayerList, true},
    {kHUDShowScore, true},
    {kHUDShowTime, true},
    {kHUDShowKillFeed, true},
    {kSpawnOrder, 3},
    {kShowNewHUD, true},
    {kFrameTimeTag, 16},
    {kLastAddress, ""},
    {kServerDescription, ""},
    {kServerPassword, ""},
    {kClientPassword, ""},
    {kTrackerAddress, "tracker.avara.gg"},
    {kTrackerRegister, 1},
    {kTrackerRegisterAddress, "tracker.avara.gg"},
    {kTrackerRegisterFrequency, 5},
    {kPunchServerAddress, "tracker.avara.gg"},
    {kPunchServerPort, 19555},
    {kPunchHoles, true},
    {kRecentSets, {}},
    {kRecentLevels, {}},
    {kSoundVolume, 100},
    {kIgnoreCustomColorsTag, false},
    {kIgnoreLevelCustomColorsTag, false},
    {kIgnoreCustomGoodySound, false},
    {kThrottle, 0},
    {kGoodGamePhrases, {}},
    {kShowElo, false},
    {kUseLegacyRenderer, false},
    {kDefaultArgs, "-/ '/rand avara aa emo ex #fav -#bad'"}
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
