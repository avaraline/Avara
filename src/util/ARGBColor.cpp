#include <assert.h>

#include "ARGBColor.h"
#include "ColorManager.h"

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

void ARGBColor::ExportGLFloats(float *out, int n)
{
    assert(n == 3 || n == 4);

    out[0] = GetR() / 255.0;
    out[1] = GetG() / 255.0;
    out[2] = GetB() / 255.0;

    if (n == 4) {
        out[3] = GetA() / 255.0;
    }
}
