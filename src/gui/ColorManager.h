#pragma once

#include "AvaraDefines.h"
#include "Messages.h"

#include <stdint.h>
#include <string>

#ifdef __has_include
#  if __has_include(<optional>)                // Check for a standard library
#    include <optional>
#  elif __has_include(<experimental/optional>) // Check for an experimental version
#    include <experimental/optional>           // Check if __has_include is present
#  else                                        // Not found at all
#     error "Missing <optional>"
#  endif
#endif

enum ColorBlindMode { Off, Deuteranopia, Protanopia, Tritanopia };

class ColorManager {
public:
    static inline uint32_t getDefaultTeamColor() {
        return teamColors[0];
    }

    static inline uint32_t getEnergyGaugeColor() {
        return energyGaugeColor;
    }

    static inline uint32_t getGrenadeSightPrimaryColor() {
        return grenadeSightPrimaryColor;
    }

    static inline uint32_t getGrenadeSightSecondaryColor() {
        return grenadeSightSecondaryColor;
    }

    static inline uint32_t getLookForwardColor() {
        return lookForwardColor;
    }

    static inline uint32_t getMissileLockColor() {
        return missileLockColor;
    }

    static inline uint32_t getMissileSightPrimaryColor() {
        return missileSightPrimaryColor;
    }

    static inline uint32_t getMissileSightSecondaryColor() {
        return missileSightSecondaryColor;
    }

    static inline uint32_t getMissileArmedColor() {
        return missileArmedColor;
    }

    static inline uint32_t getMissileLaunchedColor() {
        return missileLaunchedColor;
    }

    static inline uint32_t getNetDelay1Color() {
        return netDelay1Color;
    }

    static inline uint32_t getNetDelay2Color() {
        return netDelay2Color;
    }

    static inline uint32_t getPinwheel1Color() {
        return pinwheel1Color;
    }

    static inline uint32_t getPinwheel2Color() {
        return pinwheel2Color;
    }

    static inline uint32_t getPinwheel3Color() {
        return pinwheel3Color;
    }

    static inline uint32_t getPinwheel4Color() {
        return pinwheel4Color;
    }

    static inline uint32_t getPlasmaGauge1Color() {
        return plasmaGauge1Color;
    }

    static inline uint32_t getPlasmaGauge2Color() {
        return plasmaGauge2Color;
    }

    static inline uint32_t getPlasmaSightsOffColor() {
        return plasmaSightsOffColor;
    }

    static inline uint32_t getPlasmaSightsOnColor() {
        return plasmaSightsOnColor;
    }

    static inline uint32_t getShieldGaugeColor() {
        return shieldGaugeColor;
    }

    static inline uint32_t getSpecialBlackColor() {
        return specialBlackColor;
    }

    static inline uint32_t getSpecialWhiteColor() {
        return specialWhiteColor;
    }

    static inline std::optional<uint32_t> getMarkerColor(uint8_t num) {
        switch (num) {
            case 0:
                return 0x00fefefe;
            case 1:
                return 0x00fe0000;
            default:
                return std::optional<uint32_t>{};
        }
    }

    static inline std::optional<uint32_t> getTeamColor(uint8_t num) {
        return (num <= kMaxTeamColors)
            ? teamColors[num]
            : std::optional<uint32_t>{};
    }

    static inline std::optional<uint32_t> getTeamTextColor(uint8_t num) {
        return (num <= kMaxTeamColors)
            ? teamTextColors[num]
            : std::optional<uint32_t>{};
    }

    static inline std::optional<std::string> getTeamColorName(uint8_t num) {
        return (num <= kMaxTeamColors)
            ? teamColorNames[num]
            : std::optional<std::string>{};
    }

    static inline uint32_t getMessageColor(MsgCategory category) {
        uint32_t color = 0xffffffff;
        int idx = static_cast<int>(category);
        if (idx < sizeof(messageColors) / sizeof(uint32_t)) {
            color = messageColors[idx];
        }
        return color;
    }

    static inline float getHudAlpha() {
        return hudAlpha;
    }

    static void setColorBlind(ColorBlindMode mode);
    static void setHudAlpha(float alpha);
    static void setMissileArmedColor(uint32_t color);
    static void setMissileLaunchedColor(uint32_t color);
private:
    ColorManager() {}

    static ColorBlindMode colorBlindMode;
    static float hudAlpha;

    static uint32_t energyGaugeColor;
    static uint32_t grenadeSightPrimaryColor;
    static uint32_t grenadeSightSecondaryColor;
    static uint32_t lookForwardColor;
    static uint32_t missileLockColor;
    static uint32_t missileSightPrimaryColor;
    static uint32_t missileSightSecondaryColor;
    static uint32_t missileArmedColor;
    static uint32_t missileLaunchedColor;
    static uint32_t netDelay1Color;
    static uint32_t netDelay2Color;
    static uint32_t pinwheel1Color;
    static uint32_t pinwheel2Color;
    static uint32_t pinwheel3Color;
    static uint32_t pinwheel4Color;
    static uint32_t plasmaGauge1Color;
    static uint32_t plasmaGauge2Color;
    static uint32_t plasmaSightsOffColor;
    static uint32_t plasmaSightsOnColor;
    static uint32_t shieldGaugeColor;
    static uint32_t specialBlackColor;
    static uint32_t specialWhiteColor;
    static uint32_t teamColors[kMaxTeamColors + 1];
    static uint32_t teamTextColors[kMaxTeamColors + 1];
    static std::string teamColorNames[kMaxTeamColors + 1];
    static uint32_t messageColors[3];
};
