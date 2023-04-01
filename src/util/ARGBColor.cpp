#include <assert.h>
#include <regex>

#include "ARGBColor.h"
#include "ColorManager.h"
#include "csscolorparser.hpp"

ARGBColor::ARGBColor(const HSLAColor& hsla)
{
    uint8_t a = round(hsla.GetA() * 255);
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    if (hsla.GetS() == 0.0) {
        uint8_t val = round(hsla.GetL() * 255);
        r = val;
        g = val;
        b = val;
    } else {
        float hue = 0.0;
        float tmp1 = 0.0;
        float tmp2 = 0.0;
        float tmpChannels[3] = {0.0, 0.0, 0.0};

        if (hsla.GetL() < 0.5) {
            tmp1 = hsla.GetL() * (1.0 + hsla.GetS());
        } else {
            tmp1 = hsla.GetL() + hsla.GetS() - hsla.GetL() * hsla.GetS();
        }
        tmp2 = 2 * hsla.GetL() - tmp1;
        hue = hsla.GetH() / 360.0;
        tmpChannels[0] = hue + 0.333;
        tmpChannels[1] = hue;
        tmpChannels[2] = hue - 0.333;
        for (uint8_t i = 0; i < 3; i++) {
            if (tmpChannels[i] < 0.0) {
                tmpChannels[i] += 1.0;
            } else if (tmpChannels[i] > 1.0) {
                tmpChannels[i] -= 1.0;
            }

            if (6 * tmpChannels[i] < 1.0) {
                tmpChannels[i] = tmp2 + (tmp1 - tmp2) * 6 * tmpChannels[i];
            } else if (2 * tmpChannels[i] < 1.0) {
                tmpChannels[i] = tmp1;
            } else if (3 * tmpChannels[i] < 2.0) {
                tmpChannels[i] = tmp2 + (tmp1 - tmp2) * (0.666 - tmpChannels[i]) * 6;
            } else {
                tmpChannels[i] = tmp2;
            }
        }
        r = round(tmpChannels[0] * 255);
        g = round(tmpChannels[1] * 255);
        b = round(tmpChannels[2] * 255);
    }

    color =
        (static_cast<uint32_t>(a) << 24) +
        (static_cast<uint32_t>(r) << 16) +
        (static_cast<uint32_t>(g) << 8) +
        static_cast<uint32_t>(b);
}

std::optional<ARGBColor> ARGBColor::Parse(const std::string& str)
{
    std::optional<CSSColorParser::Color> c = CSSColorParser::parse(str);

    if (c) {
        uint32_t value = (static_cast<uint32_t>(((*c).a * 255.0) + 0.5) << 24) +
            ((*c).r << 16) +
            ((*c).g << 8) +
            (*c).b;
        return ARGBColor(value);
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

void ARGBColor::ExportGLFloats(float *out, int n) const
{
    assert(n == 3 || n == 4);

    out[0] = GetR() / 255.0;
    out[1] = GetG() / 255.0;
    out[2] = GetB() / 255.0;

    if (n == 4) {
        out[3] = GetA() / 255.0;
    }
}

ARGBColor ARGBColor::GetContrastingShade() const
{
    HSLAColor hsla = HSLAColor(*this);
    if (hsla.GetL() > 0.4) {
        hsla = hsla.WithL(0.1);
    } else {
        hsla = hsla.WithL(0.7);
    }
    if (hsla.GetS() > 0.5) {
        hsla = hsla.WithS(0.2);
    } else {
        hsla = hsla.WithS(0.8);
    }
    return ARGBColor(hsla);
}

HSLAColor::HSLAColor(float h, float s, float l, float a)
{
    SetH(h);
    SetS(s);
    SetL(l);
    SetA(a);
}

HSLAColor::HSLAColor(const ARGBColor& argb)
{
    float h = 0.0;
    float s = 0.0;
    float l = 0.0;
    float a = argb.GetA() / 255.0;

    float r = argb.GetR() / 255.0;
    float g = argb.GetG() / 255.0;
    float b = argb.GetB() / 255.0;

    float min = fmin(r, fmin(g, b));
    float max = fmax(r, fmax(g, b));

    l = (min + max) / 2;

    if (min != max) {
        if (l <= 0.5) {
            s = (max - min) / (max + min);
        } else {
            s = (max - min) / (2.0 - max - min);
        }
    }

    if (r == max) {
        h = ((g - b) / (max - min)) * 60.0;
    } else if (g == max) {
        h = (2.0 + (b - r) / (max - min)) * 60.0;
    } else {
        h = (4.0 + (r - g) / (max - min)) * 60.0;
    }

    SetH(h);
    SetS(s);
    SetL(l);
    SetA(a);
}

void HSLAColor::SetH(float h)
{
    while (h < 0) {
        h += 360.0;
    }

    if (h > 360) {
        h = fmod(h, 360.0);
    }

    hue = h;
}
