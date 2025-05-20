#pragma once
#include "ARGBColor.h"
#include "AvaraDefines.h"
#include "CApplication.h"
#include "Messages.h"
#include "Preferences.h"

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
    static inline ARGBColor getDefaultTeamColor() {
        return teamColors[0];
    }

    static inline ARGBColor getEnergyGaugeColor() {
        return energyGaugeColor;
    }

    static inline ARGBColor getGrenadeSightPrimaryColor() {
        return grenadeSightPrimaryColor;
    }

    static inline ARGBColor getGrenadeSightSecondaryColor() {
        return grenadeSightSecondaryColor;
    }

    static inline ARGBColor getLookForwardColor() {
        return lookForwardColor;
    }

    static inline ARGBColor getMissileLockColor() {
        return missileLockColor;
    }

    static inline ARGBColor getMissileSightPrimaryColor() {
        return missileSightPrimaryColor;
    }

    static inline ARGBColor getMissileSightSecondaryColor() {
        return missileSightSecondaryColor;
    }

    static inline ARGBColor getMissileArmedColor() {
        return missileArmedColor;
    }

    static inline ARGBColor getMissileLaunchedColor() {
        return missileLaunchedColor;
    }

    static inline ARGBColor getNetDelay1Color() {
        return netDelay1Color;
    }

    static inline ARGBColor getNetDelay2Color() {
        return netDelay2Color;
    }

    static inline ARGBColor getPinwheel1Color() {
        return pinwheel1Color;
    }

    static inline ARGBColor getPinwheel2Color() {
        return pinwheel2Color;
    }

    static inline ARGBColor getPinwheel3Color() {
        return pinwheel3Color;
    }

    static inline ARGBColor getPinwheel4Color() {
        return pinwheel4Color;
    }

    static inline ARGBColor getPlasmaGauge1Color() {
        return plasmaGauge1Color;
    }

    static inline ARGBColor getPlasmaGauge2Color() {
        return plasmaGauge2Color;
    }

    static inline ARGBColor getPlasmaSightsOffColor() {
        return plasmaSightsOffColor;
    }

    static inline ARGBColor getPlasmaSightsOnColor() {
        return plasmaSightsOnColor;
    }

    static inline ARGBColor getShieldGaugeColor() {
        return shieldGaugeColor;
    }

    static inline ARGBColor getHUDColor() {
        return hudColor;
    }

    static inline ARGBColor getHUDAltColor() {
        return hudAltColor;
    }
    
    static inline ARGBColor getHUDPositiveColor() {
        return hudPositiveColor;
    }

    static inline ARGBColor getHUDWarningColor() {
        return hudWarningColor;
    }

    static inline ARGBColor getHUDCriticalColor() {
        return hudCriticalColor;
    }


    static inline std::optional<ARGBColor> getMarkerColor(uint8_t num) {
        switch (num) {
            case 0:
                return 0x00fefefe;
            case 1:
                return 0x00fe0000;
            case 2:
                return 0x000333ff;
            case 3:
                return 0x00929292;
            default:
                return std::optional<ARGBColor>{};
        }
    }

    static inline std::optional<ARGBColor> getTeamColor(uint8_t num) {
        return (num <= kMaxTeamColors)
            ? (teamColorOverrides[num])
                ? teamColorOverrides[num]
                : teamColors[num]
            : std::optional<ARGBColor>{};
    }

    static inline std::optional<ARGBColor> getTeamTextColor(uint8_t num) {
        return (num <= kMaxTeamColors)
            ? (teamTextColorOverrides[num])
                ? teamTextColorOverrides[num]
                : teamTextColors[num]
            : std::optional<ARGBColor>{};
    }

    static inline std::optional<ARGBColor> getTeamBaseColor(uint8_t num) {
        return (num <= kMaxTeamColors)
            ? teamColors[num]
            : std::optional<ARGBColor>{};
    }

    static inline std::optional<ARGBColor> getTeamTextBaseColor(uint8_t num) {
        return (num <= kMaxTeamColors)
            ? teamTextColors[num]
            : std::optional<ARGBColor>{};
    }

    static inline std::optional<std::string> getTeamColorName(uint8_t num) {
        return (num <= kMaxTeamColors)
            ? teamColorNames[num]
            : std::optional<std::string>{};
    }

    static inline ARGBColor getMessageColor(MsgCategory category) {
        ARGBColor color = 0xffffffff;
        size_t idx = static_cast<size_t>(category);
        if (idx < sizeof(messageColors) / sizeof(ARGBColor)) {
            color = messageColors[idx];
        }
        return color;
    }

    static inline ARGBColor getPingColor(uint8_t num) {
        return pingColors[num];
    }

    static inline float getHUDAlpha() {
        return hudAlpha;
    }

    static void setColorBlind(ColorBlindMode mode);
    static void setHUDColor(ARGBColor color);
    static void setHUDPositiveColor(ARGBColor color);
    static void setHUDWarningColor(ARGBColor color);
    static void setHUDCriticalColor(ARGBColor color);
    static void setHUDAlpha(float alpha);
    static void setMissileArmedColor(ARGBColor color);
    static void setMissileLaunchedColor(ARGBColor color);

    static void overrideTeamColor(uint8_t num, ARGBColor color);

    static void refresh(CApplication *app);
    static void resetOverrides();
private:
    ColorManager() {}

    static ColorBlindMode colorBlindMode;
    static ARGBColor hudColor;
    static ARGBColor hudAltColor;
    static ARGBColor hudPositiveColor;
    static ARGBColor hudWarningColor;
    static ARGBColor hudCriticalColor;

    static float hudAlpha;

    static ARGBColor energyGaugeColor;
    static ARGBColor grenadeSightPrimaryColor;
    static ARGBColor grenadeSightSecondaryColor;
    static ARGBColor lookForwardColor;
    static ARGBColor missileLockColor;
    static ARGBColor missileSightPrimaryColor;
    static ARGBColor missileSightSecondaryColor;
    static ARGBColor missileArmedColor;
    static ARGBColor missileLaunchedColor;
    static ARGBColor netDelay1Color;
    static ARGBColor netDelay2Color;
    static ARGBColor pinwheel1Color;
    static ARGBColor pinwheel2Color;
    static ARGBColor pinwheel3Color;
    static ARGBColor pinwheel4Color;
    static ARGBColor plasmaGauge1Color;
    static ARGBColor plasmaGauge2Color;
    static ARGBColor plasmaSightsOffColor;
    static ARGBColor plasmaSightsOnColor;
    static ARGBColor shieldGaugeColor;
    static ARGBColor teamColors[kMaxTeamColors + 1];
    static ARGBColor teamTextColors[kMaxTeamColors + 1];
    static std::string teamColorNames[kMaxTeamColors + 1];
    static std::optional<ARGBColor> teamColorOverrides[kMaxTeamColors + 1];
    static std::optional<ARGBColor> teamTextColorOverrides[kMaxTeamColors + 1];
    static ARGBColor messageColors[3];
    static ARGBColor pingColors[3];
};
