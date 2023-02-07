#pragma once

#include "nanovg.h"

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

class ARGBColor final {
public:
    void* operator new(std::size_t) = delete;
    bool operator==(const ARGBColor& other) { return color == other.color; }
    constexpr ARGBColor(uint32_t value): color(value) {}
    static std::optional<ARGBColor> Parse(const std::string& str);
    inline uint32_t GetRaw() { return color; }
    inline uint8_t GetA() { return color >> 24; }
    inline uint8_t GetR() { return (color >> 16) & 0xFF; }
    inline uint8_t GetG() { return (color >> 8) & 0xFF; }
    inline uint8_t GetB() { return color & 0xFF; }
    inline ARGBColor WithA(uint8_t a) { return ARGBColor((color & 0x00ffffff) | (static_cast<uint32_t>(a) << 24)); }
    inline ARGBColor WithR(uint8_t r) { return ARGBColor((color & 0xff00ffff) | (static_cast<uint32_t>(r) << 16)); }
    inline ARGBColor WithG(uint8_t g) { return ARGBColor((color & 0xffff00ff) | (static_cast<uint32_t>(g) << 8)); }
    inline ARGBColor WithB(uint8_t b) { return ARGBColor((color & 0xffffff00) | static_cast<uint32_t>(b)); }
    inline NVGcolor IntoNVG() { return nvgRGBA(GetR(), GetG(), GetB(), GetA()); }
    void ExportGLFloats(float *out, int n = 4);
private:
    uint32_t color = 0x00000000;
};
