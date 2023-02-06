#pragma once

#include "ColorManager.h"
#include "csscolorparser.hpp"
#include "nanovg.h"

#include <regex>
#include <SDL2/SDL.h>
#include <stdint.h>

#ifdef __has_include
#  if __has_include(<optional>)                // Check for a standard library
#    include <optional>
#  elif __has_include(<experimental/optional>) // Check for an experimental version
#    include <experimental/optional>           // Check if __has_include is present
#  else                                        // Not found at all
#     error "Missing <optional>"
#  endif
#endif

static uint32_t RGBAToLong(CSSColorParser::Color rgba) {
    return (
        (static_cast<int>((rgba.a * 255.0) + 0.5) << 24) +
        (rgba.r << 16) +
        (rgba.g << 8) +
        rgba.b
    );
}

static inline uint8_t LongToR(uint32_t in) {
    return ((in >> 16) & 0xFF);
}

static inline uint8_t LongToG(uint32_t in) {
    return ((in >> 8) & 0xFF);
}

static inline uint8_t LongToB(uint32_t in) {
    return (in & 0xFF);
}

static inline uint8_t LongToA(uint32_t in) {
    return (in >> 24);
}

static void LongToRGBA(uint32_t in, float *out, int n = 4) {
    if (n < 3 || n > 4) {
        SDL_Log("n must be 3 (for RGB) or 4 (for RGBA)");
        exit(69);
    }

    out[0] = LongToR(in) / 255.0;
    out[1] = LongToG(in) / 255.0;
    out[2] = LongToB(in) / 255.0;

    if (n == 4) {
        out[3] = LongToA(in) / 255.0;
    }
}

static inline NVGcolor LongToNVG(uint32_t in) {
    return nvgRGBA(
        LongToR(in),
        LongToG(in),
        LongToB(in),
        LongToA(in)
    );
}

static std::optional<uint32_t> ParseColor(const std::string& str) {
    std::optional<CSSColorParser::Color> color = CSSColorParser::parse(str);

    if (color) {
        return RGBAToLong(*color);
    }

    std::string tmp = str;

    // Remove all whitespace.
    tmp.erase(std::remove(tmp.begin(), tmp.end(), ' '), tmp.end());

    // Convert to lowercase.
    std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);

    {
        std::regex search("^marker\\((\\d+)\\)$");
        std::smatch match;
        if (std::regex_search(tmp, match, search)) {
            return ColorManager::getMarkerColor(
                strtoll(match[1].str().c_str(), nullptr, 10)
            );
        }
    }

    {
        std::regex search("^team\\((\\d+)\\)$");
        std::smatch match;
        if (std::regex_search(tmp, match, search)) {
            return ColorManager::getTeamColor(
                strtoll(match[1].str().c_str(), nullptr, 10)
            );
        }
    }

    return {};
}
