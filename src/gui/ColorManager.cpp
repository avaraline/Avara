#include "ColorManager.h"

#include <algorithm>

ColorBlindMode ColorManager::colorBlindMode = Off;
float ColorManager::hudAlpha = 1.f;
uint32_t ColorManager::energyGaugeColor = 0xff009c00;
uint32_t ColorManager::grenadeSightPrimaryColor = 0xfffefb00;
uint32_t ColorManager::grenadeSightSecondaryColor = 0xffff2600;
uint32_t ColorManager::lookForwardColor = 0xff000000;
uint32_t ColorManager::missileLockColor = 0xffff2600;
uint32_t ColorManager::missileSightPrimaryColor = 0xfffefb00;
uint32_t ColorManager::missileSightSecondaryColor = 0xffff2600;
uint32_t ColorManager::netDelay1Color = 0xffffc900;
uint32_t ColorManager::netDelay2Color = 0xfffefb00;
uint32_t ColorManager::pinwheel1Color = 0xff282d79;
uint32_t ColorManager::pinwheel2Color = 0xff2833a9;
uint32_t ColorManager::pinwheel3Color = 0xff283ad5;
uint32_t ColorManager::pinwheel4Color = 0xff2941ff;
uint32_t ColorManager::plasmaGauge1Color = 0xffb44900;
uint32_t ColorManager::plasmaGauge2Color = 0xffff4e00;
uint32_t ColorManager::plasmaSightsOffColor = 0xff008e00;
uint32_t ColorManager::plasmaSightsOnColor = 0xffff2600;
uint32_t ColorManager::shieldGaugeColor = 0xff0053b4;
uint32_t ColorManager::specialBlackColor = 0xff3e3e3e;
uint32_t ColorManager::specialWhiteColor = 0xffe6e6e6;

uint32_t ColorManager::teamColors[kMaxTeamColors + 1] = {
    0xffffffff,
    0xff007600,
    0xffd5d200,
    0xffd71e00,
    0xffd72ca9,
    0xffab2fd5,
    0xff00a9d5,
    0xffffb300,
    0xff99ced1
};

uint32_t ColorManager::teamTextColors[kMaxTeamColors + 1] = {
    0xff333333,
    0xffffffff,
    0xff333333,
    0xffffffff,
    0xffffffff,
    0xffffffff,
    0xff333333,
    0xff333333,
    0xff333333
};

std::string ColorManager::teamColorNames[kMaxTeamColors + 1] = {
    "Neutral",
    "Green",
    "Yellow",
    "Red",
    "Pink",
    "Purple",
    "Blue",
    "Orange",
    "Teal"
};

void ColorManager::setColorBlind(ColorBlindMode mode) {
    switch (mode) {
        case Off:
            ColorManager::energyGaugeColor = 0xff009c00;
            ColorManager::grenadeSightPrimaryColor = 0xfffefb00;
            ColorManager::grenadeSightSecondaryColor = 0xffff2600;
            ColorManager::lookForwardColor = 0xff000000;
            ColorManager::missileLockColor = 0xffff2600;
            ColorManager::missileSightPrimaryColor = 0xfffefb00;
            ColorManager::missileSightSecondaryColor = 0xffff2600;
            ColorManager::netDelay1Color = 0xffffc900;
            ColorManager::netDelay2Color = 0xfffefb00;
            ColorManager::pinwheel1Color = 0xff282d79;
            ColorManager::pinwheel2Color = 0xff2833a9;
            ColorManager::pinwheel3Color = 0xff283ad5;
            ColorManager::pinwheel4Color = 0xff2941ff;
            ColorManager::plasmaGauge1Color = 0xffb44900;
            ColorManager::plasmaGauge2Color = 0xffff4e00;
            ColorManager::plasmaSightsOffColor = 0xff008e00;
            ColorManager::plasmaSightsOnColor = 0xffff2600;
            ColorManager::shieldGaugeColor = 0xff0053b4;
            ColorManager::specialBlackColor = 0xff3e3e3e;
            ColorManager::specialWhiteColor = 0xffe6e6e6;
            ColorManager::teamColors[0] = 0xffffffff;
            ColorManager::teamColors[1] = 0xff007600;
            ColorManager::teamColors[2] = 0xffd5d200;
            ColorManager::teamColors[3] = 0xffd71e00;
            ColorManager::teamColors[4] = 0xffd72ca9;
            ColorManager::teamColors[5] = 0xffab2fd5;
            ColorManager::teamColors[6] = 0xff00a9d5;
            ColorManager::teamColors[7] = 0xffffb300;
            ColorManager::teamColors[8] = 0xff99ced1;
            ColorManager::teamTextColors[0] = 0xff333333;
            ColorManager::teamTextColors[1] = 0xffffffff;
            ColorManager::teamTextColors[2] = 0xff333333;
            ColorManager::teamTextColors[3] = 0xffffffff;
            ColorManager::teamTextColors[4] = 0xffffffff;
            ColorManager::teamTextColors[5] = 0xffffffff;
            ColorManager::teamTextColors[6] = 0xff333333;
            ColorManager::teamTextColors[7] = 0xff333333;
            ColorManager::teamTextColors[8] = 0xff333333;
            break;
        case Deuteranopia:
            ColorManager::energyGaugeColor = 0xff009ea0;
            ColorManager::grenadeSightPrimaryColor = 0xfffefb00;
            ColorManager::grenadeSightSecondaryColor = 0xffffb700;
            ColorManager::lookForwardColor = 0xff000000;
            ColorManager::missileLockColor = 0xffffb700;
            ColorManager::missileSightPrimaryColor = 0xfffefb00;
            ColorManager::missileSightSecondaryColor = 0xffffb700;
            ColorManager::netDelay1Color = 0xffffe491;
            ColorManager::netDelay2Color = 0xfffefb00;
            ColorManager::pinwheel1Color = 0xff282d79;
            ColorManager::pinwheel2Color = 0xff2833a9;
            ColorManager::pinwheel3Color = 0xff283ad5;
            ColorManager::pinwheel4Color = 0xff2941ff;
            ColorManager::plasmaGauge1Color = 0xffb44900;
            ColorManager::plasmaGauge2Color = 0xffff4e00;
            ColorManager::plasmaSightsOffColor = 0xff008f68;
            ColorManager::plasmaSightsOnColor = 0xffff6c00;
            ColorManager::shieldGaugeColor = 0xff0020b4;
            ColorManager::specialBlackColor = 0xff3e3e3e;
            ColorManager::specialWhiteColor = 0xffe6e6e6;
            ColorManager::teamColors[0] = 0xffffffff;
            ColorManager::teamColors[1] = 0xff007768;
            ColorManager::teamColors[2] = 0xffd5d200;
            ColorManager::teamColors[3] = 0xffd76a00;
            ColorManager::teamColors[4] = 0xffd72ca9;
            ColorManager::teamColors[5] = 0xffab2fd5;
            ColorManager::teamColors[6] = 0xff00a9d5;
            ColorManager::teamColors[7] = 0xfff7e0b2;
            ColorManager::teamColors[8] = 0xff99ced1;
            ColorManager::teamTextColors[0] = 0xff333333;
            ColorManager::teamTextColors[1] = 0xffffffff;
            ColorManager::teamTextColors[2] = 0xff333333;
            ColorManager::teamTextColors[3] = 0xffffffff;
            ColorManager::teamTextColors[4] = 0xffffffff;
            ColorManager::teamTextColors[5] = 0xffffffff;
            ColorManager::teamTextColors[6] = 0xffffffff;
            ColorManager::teamTextColors[7] = 0xff333333;
            ColorManager::teamTextColors[8] = 0xff333333;
            break;
        case Protanopia:
            ColorManager::energyGaugeColor = 0xff009ea0;
            ColorManager::grenadeSightPrimaryColor = 0xfffefb00;
            ColorManager::grenadeSightSecondaryColor = 0xffffb700;
            ColorManager::lookForwardColor = 0xff000000;
            ColorManager::missileLockColor = 0xffffb700;
            ColorManager::missileSightPrimaryColor = 0xfffefb00;
            ColorManager::missileSightSecondaryColor = 0xffffb700;
            ColorManager::netDelay1Color = 0xffffe491;
            ColorManager::netDelay2Color = 0xfffefb00;
            ColorManager::pinwheel1Color = 0xff282d79;
            ColorManager::pinwheel2Color = 0xff2833a9;
            ColorManager::pinwheel3Color = 0xff283ad5;
            ColorManager::pinwheel4Color = 0xff2941ff;
            ColorManager::plasmaGauge1Color = 0xffb44900;
            ColorManager::plasmaGauge2Color = 0xffff4e00;
            ColorManager::plasmaSightsOffColor = 0xff008f68;
            ColorManager::plasmaSightsOnColor = 0xffff6c00;
            ColorManager::shieldGaugeColor = 0xff0020b4;
            ColorManager::specialBlackColor = 0xff3e3e3e;
            ColorManager::specialWhiteColor = 0xffe6e6e6;
            ColorManager::teamColors[0] = 0xffffffff;
            ColorManager::teamColors[1] = 0xff007768;
            ColorManager::teamColors[2] = 0xffd5d200;
            ColorManager::teamColors[3] = 0xffd76a00;
            ColorManager::teamColors[4] = 0xffd72ca9;
            ColorManager::teamColors[5] = 0xffab2fd5;
            ColorManager::teamColors[6] = 0xff00a9d5;
            ColorManager::teamColors[7] = 0xfff7e0b2;
            ColorManager::teamColors[8] = 0xff99ced1;
            ColorManager::teamTextColors[0] = 0xff333333;
            ColorManager::teamTextColors[1] = 0xffffffff;
            ColorManager::teamTextColors[2] = 0xff333333;
            ColorManager::teamTextColors[3] = 0xffffffff;
            ColorManager::teamTextColors[4] = 0xffffffff;
            ColorManager::teamTextColors[5] = 0xffffffff;
            ColorManager::teamTextColors[6] = 0xffffffff;
            ColorManager::teamTextColors[7] = 0xff333333;
            ColorManager::teamTextColors[8] = 0xff333333;
            break;
        case Tritanopia:
            ColorManager::energyGaugeColor = 0xff009c00;
            ColorManager::grenadeSightPrimaryColor = 0xfffefb00;
            ColorManager::grenadeSightSecondaryColor = 0xffff2600;
            ColorManager::lookForwardColor = 0xff000000;
            ColorManager::missileLockColor = 0xffff2600;
            ColorManager::missileSightPrimaryColor = 0xfffefb00;
            ColorManager::missileSightSecondaryColor = 0xffff2600;
            ColorManager::netDelay1Color = 0xffffc900;
            ColorManager::netDelay2Color = 0xfffefb00;
            ColorManager::pinwheel1Color = 0xff282d79;
            ColorManager::pinwheel2Color = 0xff2833a9;
            ColorManager::pinwheel3Color = 0xff283ad5;
            ColorManager::pinwheel4Color = 0xff2941ff;
            ColorManager::plasmaGauge1Color = 0xffb44900;
            ColorManager::plasmaGauge2Color = 0xffff4e00;
            ColorManager::plasmaSightsOffColor = 0xff008e00;
            ColorManager::plasmaSightsOnColor = 0xffff2600;
            ColorManager::shieldGaugeColor = 0xff0053b4;
            ColorManager::specialBlackColor = 0xff3e3e3e;
            ColorManager::specialWhiteColor = 0xffe6e6e6;
            ColorManager::teamColors[0] = 0xffffffff;
            ColorManager::teamColors[1] = 0xff007600;
            ColorManager::teamColors[2] = 0xffd5d200;
            ColorManager::teamColors[3] = 0xffd71e00;
            ColorManager::teamColors[4] = 0xffd72ca9;
            ColorManager::teamColors[5] = 0xffab2fd5;
            ColorManager::teamColors[6] = 0xff00a9d5;
            ColorManager::teamColors[7] = 0xffffb300;
            ColorManager::teamColors[8] = 0xff99ced1;
            ColorManager::teamTextColors[0] = 0xff333333;
            ColorManager::teamTextColors[1] = 0xffffffff;
            ColorManager::teamTextColors[2] = 0xff333333;
            ColorManager::teamTextColors[3] = 0xffffffff;
            ColorManager::teamTextColors[4] = 0xffffffff;
            ColorManager::teamTextColors[5] = 0xffffffff;
            ColorManager::teamTextColors[6] = 0xffffffff;
            ColorManager::teamTextColors[7] = 0xff333333;
            ColorManager::teamTextColors[8] = 0xff333333;
            break;
    }
    if (ColorManager::hudAlpha != 1.f) {
        ColorManager::setHudAlpha(ColorManager::hudAlpha);
    }
    ColorManager::colorBlindMode = mode;
}

void ColorManager::setHudAlpha(float alpha) {
    alpha = std::clamp(alpha, 0.f, 1.f);
    uint32_t alphaSet = static_cast<uint32_t>((alpha * 255) + 0.5) << 24;

    ColorManager::grenadeSightPrimaryColor = (ColorManager::grenadeSightPrimaryColor & 0x00ffffff) | alphaSet;
    ColorManager::grenadeSightSecondaryColor = (ColorManager::grenadeSightSecondaryColor & 0x00ffffff) | alphaSet;
    ColorManager::lookForwardColor = (ColorManager::lookForwardColor & 0x00ffffff) | alphaSet;
    ColorManager::missileLockColor = (ColorManager::missileLockColor & 0x00ffffff) | alphaSet;
    ColorManager::missileSightPrimaryColor = (ColorManager::missileSightPrimaryColor & 0x00ffffff) | alphaSet;
    ColorManager::missileSightSecondaryColor = (ColorManager::missileSightSecondaryColor & 0x00ffffff) | alphaSet;
    ColorManager::plasmaSightsOffColor = (ColorManager::plasmaSightsOffColor & 0x00ffffff) | alphaSet;
    ColorManager::plasmaSightsOnColor = (ColorManager::plasmaSightsOnColor & 0x00ffffff) | alphaSet;

    ColorManager::hudAlpha = alpha;
}
