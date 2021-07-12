#pragma once

#include "AvaraDefines.h"

#include <optional>
#include <stdint.h>

enum ColorBlindMode { Off, Deuteranopia, Protanopia, Tritanopia };

class CColorManager {
public:
    static inline std::optional<uint32_t> getTeamColor(uint8_t num) {
        if (num <= kMaxTeamColors) {
            return teamColors[num];
        }
        return {};
    };

    static inline std::optional<uint32_t> getTeamTextColor(uint8_t num) {
        if (num <= kMaxTeamColors) {
            return teamTextColors[num];
        }
        return {};
    };

    static void setColorBlind(ColorBlindMode mode);
private:
    CColorManager() {}

    static uint32_t teamColors[kMaxTeamColors + 1];
    static uint32_t teamTextColors[kMaxTeamColors + 1];
};
