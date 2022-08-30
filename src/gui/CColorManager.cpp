#include "CColorManager.h"

#include <algorithm>

ColorBlindMode CColorManager::colorBlindMode = Off;
float CColorManager::hudAlpha = 1.f;
uint32_t CColorManager::energyGaugeColor = 0xff009c00;
uint32_t CColorManager::grenadeSightPrimaryColor = 0xfffefb00;
uint32_t CColorManager::grenadeSightSecondaryColor = 0xffff2600;
uint32_t CColorManager::lookForwardColor = 0xff000000;
uint32_t CColorManager::missileLockColor = 0xffff2600;
uint32_t CColorManager::missileSightPrimaryColor = 0xfffefb00;
uint32_t CColorManager::missileSightSecondaryColor = 0xffff2600;
uint32_t CColorManager::netDelay1Color = 0xffffc900;
uint32_t CColorManager::netDelay2Color = 0xfffefb00;
uint32_t CColorManager::pinwheel1Color = 0xff282d79;
uint32_t CColorManager::pinwheel2Color = 0xff2833a9;
uint32_t CColorManager::pinwheel3Color = 0xff283ad5;
uint32_t CColorManager::pinwheel4Color = 0xff2941ff;
uint32_t CColorManager::plasmaGauge1Color = 0xffb44900;
uint32_t CColorManager::plasmaGauge2Color = 0xffff4e00;
uint32_t CColorManager::plasmaSightsOffColor = 0xff008e00;
uint32_t CColorManager::plasmaSightsOnColor = 0xffff2600;
uint32_t CColorManager::shieldGaugeColor = 0xff0053b4;
uint32_t CColorManager::specialBlackColor = 0xff3e3e3e;
uint32_t CColorManager::specialWhiteColor = 0xffe6e6e6;

uint32_t CColorManager::teamColors[kMaxTeamColors + 1] = {
    0xfffeffff,
    0xff007600,
    0xffd5d200,
    0xffd71e00,
    0xffd72ca9,
    0xffab2fd5,
    0xff00a9d5,
    0xffffb300,
    0xff99ced1
};

uint32_t CColorManager::teamTextColors[kMaxTeamColors + 1] = {
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

std::string CColorManager::teamColorNames[kMaxTeamColors + 1] = {
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

void CColorManager::setColorBlind(ColorBlindMode mode) {
    switch (mode) {
        case Off:
            CColorManager::energyGaugeColor = 0xff009c00;
            CColorManager::grenadeSightPrimaryColor = 0xfffefb00;
            CColorManager::grenadeSightSecondaryColor = 0xffff2600;
            CColorManager::lookForwardColor = 0xff000000;
            CColorManager::missileLockColor = 0xffff2600;
            CColorManager::missileSightPrimaryColor = 0xfffefb00;
            CColorManager::missileSightSecondaryColor = 0xffff2600;
            CColorManager::netDelay1Color = 0xffffc900;
            CColorManager::netDelay2Color = 0xfffefb00;
            CColorManager::pinwheel1Color = 0xff282d79;
            CColorManager::pinwheel2Color = 0xff2833a9;
            CColorManager::pinwheel3Color = 0xff283ad5;
            CColorManager::pinwheel4Color = 0xff2941ff;
            CColorManager::plasmaGauge1Color = 0xffb44900;
            CColorManager::plasmaGauge2Color = 0xffff4e00;
            CColorManager::plasmaSightsOffColor = 0xff008e00;
            CColorManager::plasmaSightsOnColor = 0xffff2600;
            CColorManager::shieldGaugeColor = 0xff0053b4;
            CColorManager::specialBlackColor = 0xff3e3e3e;
            CColorManager::specialWhiteColor = 0xffe6e6e6;
            CColorManager::teamColors[0] = 0xfffeffff;
            CColorManager::teamColors[1] = 0xff007600;
            CColorManager::teamColors[2] = 0xffd5d200;
            CColorManager::teamColors[3] = 0xffd71e00;
            CColorManager::teamColors[4] = 0xffd72ca9;
            CColorManager::teamColors[5] = 0xffab2fd5;
            CColorManager::teamColors[6] = 0xff00a9d5;
            CColorManager::teamColors[7] = 0xffffb300;
            CColorManager::teamColors[8] = 0xff99ced1;
            CColorManager::teamTextColors[0] = 0xff333333;
            CColorManager::teamTextColors[1] = 0xffffffff;
            CColorManager::teamTextColors[2] = 0xff333333;
            CColorManager::teamTextColors[3] = 0xffffffff;
            CColorManager::teamTextColors[4] = 0xffffffff;
            CColorManager::teamTextColors[5] = 0xffffffff;
            CColorManager::teamTextColors[6] = 0xff333333;
            CColorManager::teamTextColors[7] = 0xff333333;
            CColorManager::teamTextColors[8] = 0xff333333;
            break;
        case Deuteranopia:
            CColorManager::energyGaugeColor = 0xff009ea0;
            CColorManager::grenadeSightPrimaryColor = 0xfffefb00;
            CColorManager::grenadeSightSecondaryColor = 0xffffb700;
            CColorManager::lookForwardColor = 0xff000000;
            CColorManager::missileLockColor = 0xffffb700;
            CColorManager::missileSightPrimaryColor = 0xfffefb00;
            CColorManager::missileSightSecondaryColor = 0xffffb700;
            CColorManager::netDelay1Color = 0xffffe491;
            CColorManager::netDelay2Color = 0xfffefb00;
            CColorManager::pinwheel1Color = 0xff282d79;
            CColorManager::pinwheel2Color = 0xff2833a9;
            CColorManager::pinwheel3Color = 0xff283ad5;
            CColorManager::pinwheel4Color = 0xff2941ff;
            CColorManager::plasmaGauge1Color = 0xffb44900;
            CColorManager::plasmaGauge2Color = 0xffff4e00;
            CColorManager::plasmaSightsOffColor = 0xff008f68;
            CColorManager::plasmaSightsOnColor = 0xffff6c00;
            CColorManager::shieldGaugeColor = 0xff0020b4;
            CColorManager::specialBlackColor = 0xff3e3e3e;
            CColorManager::specialWhiteColor = 0xffe6e6e6;
            CColorManager::teamColors[0] = 0xfffeffff;
            CColorManager::teamColors[1] = 0xff007768;
            CColorManager::teamColors[2] = 0xffd5d200;
            CColorManager::teamColors[3] = 0xffd76a00;
            CColorManager::teamColors[4] = 0xffd72ca9;
            CColorManager::teamColors[5] = 0xffab2fd5;
            CColorManager::teamColors[6] = 0xff00a9d5;
            CColorManager::teamColors[7] = 0xfff7e0b2;
            CColorManager::teamColors[8] = 0xff99ced1;
            CColorManager::teamTextColors[0] = 0xff333333;
            CColorManager::teamTextColors[1] = 0xffffffff;
            CColorManager::teamTextColors[2] = 0xff333333;
            CColorManager::teamTextColors[3] = 0xffffffff;
            CColorManager::teamTextColors[4] = 0xffffffff;
            CColorManager::teamTextColors[5] = 0xffffffff;
            CColorManager::teamTextColors[6] = 0xffffffff;
            CColorManager::teamTextColors[7] = 0xff333333;
            CColorManager::teamTextColors[8] = 0xff333333;
            break;
        case Protanopia:
            CColorManager::energyGaugeColor = 0xff009ea0;
            CColorManager::grenadeSightPrimaryColor = 0xfffefb00;
            CColorManager::grenadeSightSecondaryColor = 0xffffb700;
            CColorManager::lookForwardColor = 0xff000000;
            CColorManager::missileLockColor = 0xffffb700;
            CColorManager::missileSightPrimaryColor = 0xfffefb00;
            CColorManager::missileSightSecondaryColor = 0xffffb700;
            CColorManager::netDelay1Color = 0xffffe491;
            CColorManager::netDelay2Color = 0xfffefb00;
            CColorManager::pinwheel1Color = 0xff282d79;
            CColorManager::pinwheel2Color = 0xff2833a9;
            CColorManager::pinwheel3Color = 0xff283ad5;
            CColorManager::pinwheel4Color = 0xff2941ff;
            CColorManager::plasmaGauge1Color = 0xffb44900;
            CColorManager::plasmaGauge2Color = 0xffff4e00;
            CColorManager::plasmaSightsOffColor = 0xff008f68;
            CColorManager::plasmaSightsOnColor = 0xffff6c00;
            CColorManager::shieldGaugeColor = 0xff0020b4;
            CColorManager::specialBlackColor = 0xff3e3e3e;
            CColorManager::specialWhiteColor = 0xffe6e6e6;
            CColorManager::teamColors[0] = 0xfffeffff;
            CColorManager::teamColors[1] = 0xff007768;
            CColorManager::teamColors[2] = 0xffd5d200;
            CColorManager::teamColors[3] = 0xffd76a00;
            CColorManager::teamColors[4] = 0xffd72ca9;
            CColorManager::teamColors[5] = 0xffab2fd5;
            CColorManager::teamColors[6] = 0xff00a9d5;
            CColorManager::teamColors[7] = 0xfff7e0b2;
            CColorManager::teamColors[8] = 0xff99ced1;
            CColorManager::teamTextColors[0] = 0xff333333;
            CColorManager::teamTextColors[1] = 0xffffffff;
            CColorManager::teamTextColors[2] = 0xff333333;
            CColorManager::teamTextColors[3] = 0xffffffff;
            CColorManager::teamTextColors[4] = 0xffffffff;
            CColorManager::teamTextColors[5] = 0xffffffff;
            CColorManager::teamTextColors[6] = 0xffffffff;
            CColorManager::teamTextColors[7] = 0xff333333;
            CColorManager::teamTextColors[8] = 0xff333333;
            break;
        case Tritanopia:
            CColorManager::energyGaugeColor = 0xff009c00;
            CColorManager::grenadeSightPrimaryColor = 0xfffefb00;
            CColorManager::grenadeSightSecondaryColor = 0xffff2600;
            CColorManager::lookForwardColor = 0xff000000;
            CColorManager::missileLockColor = 0xffff2600;
            CColorManager::missileSightPrimaryColor = 0xfffefb00;
            CColorManager::missileSightSecondaryColor = 0xffff2600;
            CColorManager::netDelay1Color = 0xffffc900;
            CColorManager::netDelay2Color = 0xfffefb00;
            CColorManager::pinwheel1Color = 0xff282d79;
            CColorManager::pinwheel2Color = 0xff2833a9;
            CColorManager::pinwheel3Color = 0xff283ad5;
            CColorManager::pinwheel4Color = 0xff2941ff;
            CColorManager::plasmaGauge1Color = 0xffb44900;
            CColorManager::plasmaGauge2Color = 0xffff4e00;
            CColorManager::plasmaSightsOffColor = 0xff008e00;
            CColorManager::plasmaSightsOnColor = 0xffff2600;
            CColorManager::shieldGaugeColor = 0xff0053b4;
            CColorManager::specialBlackColor = 0xff3e3e3e;
            CColorManager::specialWhiteColor = 0xffe6e6e6;
            CColorManager::teamColors[0] = 0xfffeffff;
            CColorManager::teamColors[1] = 0xff007600;
            CColorManager::teamColors[2] = 0xffd5d200;
            CColorManager::teamColors[3] = 0xffd71e00;
            CColorManager::teamColors[4] = 0xffd72ca9;
            CColorManager::teamColors[5] = 0xffab2fd5;
            CColorManager::teamColors[6] = 0xff00a9d5;
            CColorManager::teamColors[7] = 0xffffb300;
            CColorManager::teamColors[8] = 0xff99ced1;
            CColorManager::teamTextColors[0] = 0xff333333;
            CColorManager::teamTextColors[1] = 0xffffffff;
            CColorManager::teamTextColors[2] = 0xff333333;
            CColorManager::teamTextColors[3] = 0xffffffff;
            CColorManager::teamTextColors[4] = 0xffffffff;
            CColorManager::teamTextColors[5] = 0xffffffff;
            CColorManager::teamTextColors[6] = 0xffffffff;
            CColorManager::teamTextColors[7] = 0xff333333;
            CColorManager::teamTextColors[8] = 0xff333333;
            break;
    }
    if (CColorManager::hudAlpha != 1.f) {
        CColorManager::setHudAlpha(CColorManager::hudAlpha);
    }
    CColorManager::colorBlindMode = mode;
}

void CColorManager::setHudAlpha(float alpha) {
    alpha = std::clamp(alpha, 0.f, 1.f);
    uint32_t alphaSet = static_cast<uint32_t>((alpha * 255) + 0.5) << 24;

    CColorManager::grenadeSightPrimaryColor = (CColorManager::grenadeSightPrimaryColor & 0x00ffffff) | alphaSet;
    CColorManager::grenadeSightSecondaryColor = (CColorManager::grenadeSightSecondaryColor & 0x00ffffff) | alphaSet;
    CColorManager::lookForwardColor = (CColorManager::lookForwardColor & 0x00ffffff) | alphaSet;
    CColorManager::missileLockColor = (CColorManager::missileLockColor & 0x00ffffff) | alphaSet;
    CColorManager::missileSightPrimaryColor = (CColorManager::missileSightPrimaryColor & 0x00ffffff) | alphaSet;
    CColorManager::missileSightSecondaryColor = (CColorManager::missileSightSecondaryColor & 0x00ffffff) | alphaSet;
    CColorManager::plasmaSightsOffColor = (CColorManager::plasmaSightsOffColor & 0x00ffffff) | alphaSet;
    CColorManager::plasmaSightsOnColor = (CColorManager::plasmaSightsOnColor & 0x00ffffff) | alphaSet;

    CColorManager::hudAlpha = alpha;
}
