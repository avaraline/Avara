#pragma once

#include <json.hpp>

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

// Key names are from https://wiki.libsdl.org/SDL_Scancode
static json defaultPrefs = {{kYonPrefTag, 0},
    {kMouseSensitivityTag, 0},
    {kLatencyToleranceTag, 1},
    {kHullTypeTag, 0}, // 0 = light, 1 = medium, 2 = heavy
    {kServerOptionsTag, 129}, // 1 = allow load, 128 = auto latency
    {kDefaultUDPPort, 19567},
    {kUDPResendPrefTag, 2},
    {kUDPConnectionSpeedTag, 3},
    {kKeyboardMappingTag,
        {{"forward", {"W", "Up"}},
            {"backward", {"S", "Down"}},
            {"left", {"A", "Left"}},
            {"right", {"D", "Right"}},
            {"jump", {"Space", "B", "N", "M", ","}},
            {"abort", "Escape"},
            {"loadMissile", "Q"},
            {"loadGrenade", "E"},
            {"fire", "E"},
            {"boost", "Left Shift"},
            {"aimForward", "2"},
            {"scoutView", "Tab"},
            {"scoutControl", {"2", "Up", "Down", "Left", "Right"}},
            {"chatMode", "Return"}}},
    {kPlayerNameTag, "Unnamed Player"}};
