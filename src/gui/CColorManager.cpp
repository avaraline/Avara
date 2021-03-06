#include "CColorManager.h"

#include <algorithm>

ColorBlindMode CColorManager::colorBlindMode = Off;
float CColorManager::hudAlpha = 1.f;
uint32_t CColorManager::energyGaugeColor = 0xff008f00;
uint32_t CColorManager::grenadeSightPrimaryColor = 0xffffff00;
uint32_t CColorManager::grenadeSightSecondaryColor = 0xffff0000;
uint32_t CColorManager::lookForwardColor = 0xff000000;
uint32_t CColorManager::missileLockColor = 0xffff0000;
uint32_t CColorManager::missileSightPrimaryColor = 0xffffff00;
uint32_t CColorManager::missileSightSecondaryColor = 0xffff0000;
uint32_t CColorManager::netDelay1Color = 0xffffc000;
uint32_t CColorManager::netDelay2Color = 0xffffff00;
uint32_t CColorManager::pinwheel1Color = 0xff1e1e66;
uint32_t CColorManager::pinwheel2Color = 0xff1e1e99;
uint32_t CColorManager::pinwheel3Color = 0xff1e1ecc;
uint32_t CColorManager::pinwheel4Color = 0xff1e1eff;
uint32_t CColorManager::plasmaGauge1Color = 0xffa33600;
uint32_t CColorManager::plasmaGauge2Color = 0xffff3600;
uint32_t CColorManager::plasmaSightsOffColor = 0xff008000;
uint32_t CColorManager::plasmaSightsOnColor = 0xffff0000;
uint32_t CColorManager::shieldGaugeColor = 0xff003da5;
uint32_t CColorManager::specialBlackColor = 0xff303030;
uint32_t CColorManager::specialWhiteColor = 0xffe0e0e0;

uint32_t CColorManager::teamColors[kMaxTeamColors + 1] = {
    0xffffffff,
    0xff006600,
    0xffcccc00,
    0xffcc0000,
    0xffcc0099,
    0xff9900cc,
    0xff0099cc,
    0xffffa500,
    0xff89c4c7
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

void CColorManager::setColorBlind(ColorBlindMode mode) {
    switch (mode) {
        case Off:
            CColorManager::energyGaugeColor = 0xff008f00;
            CColorManager::grenadeSightPrimaryColor = 0xffffff00;
            CColorManager::grenadeSightSecondaryColor = 0xffff0000;
            CColorManager::lookForwardColor = 0xff000000;
            CColorManager::missileLockColor = 0xffff0000;
            CColorManager::missileSightPrimaryColor = 0xffffff00;
            CColorManager::missileSightSecondaryColor = 0xffff0000;
            CColorManager::netDelay1Color = 0xffffc000;
            CColorManager::netDelay2Color = 0xffffff00;
            CColorManager::pinwheel1Color = 0xff1e1e66;
            CColorManager::pinwheel2Color = 0xff1e1e99;
            CColorManager::pinwheel3Color = 0xff1e1ecc;
            CColorManager::pinwheel4Color = 0xff1e1eff;
            CColorManager::plasmaGauge1Color = 0xffa33600;
            CColorManager::plasmaGauge2Color = 0xffff3600;
            CColorManager::plasmaSightsOffColor = 0xff008000;
            CColorManager::plasmaSightsOnColor = 0xffff0000;
            CColorManager::shieldGaugeColor = 0xff003da5;
            CColorManager::specialBlackColor = 0xff303030;
            CColorManager::specialWhiteColor = 0xffe0e0e0;
            CColorManager::teamColors[0] = 0xffffffff;
            CColorManager::teamColors[1] = 0xff006600;
            CColorManager::teamColors[2] = 0xffcccc00;
            CColorManager::teamColors[3] = 0xffcc0000;
            CColorManager::teamColors[4] = 0xffcc0099;
            CColorManager::teamColors[5] = 0xff9900cc;
            CColorManager::teamColors[6] = 0xff0099cc;
            CColorManager::teamColors[7] = 0xffffa500;
            CColorManager::teamColors[8] = 0xff89c4c7;
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
            CColorManager::energyGaugeColor = 0xff008f8f;
            CColorManager::grenadeSightPrimaryColor = 0xffffff00;
            CColorManager::grenadeSightSecondaryColor = 0xffffaa00;
            CColorManager::lookForwardColor = 0xff000000;
            CColorManager::missileLockColor = 0xffffaa00;
            CColorManager::missileSightPrimaryColor = 0xffffff00;
            CColorManager::missileSightSecondaryColor = 0xffffaa00;
            CColorManager::netDelay1Color = 0xffffe080;
            CColorManager::netDelay2Color = 0xffffff00;
            CColorManager::pinwheel1Color = 0xff1e1e66;
            CColorManager::pinwheel2Color = 0xff1e1e99;
            CColorManager::pinwheel3Color = 0xff1e1ecc;
            CColorManager::pinwheel4Color = 0xff1e1eff;
            CColorManager::plasmaGauge1Color = 0xffa33600;
            CColorManager::plasmaGauge2Color = 0xffff3600;
            CColorManager::plasmaSightsOffColor = 0xff008055;
            CColorManager::plasmaSightsOnColor = 0xffff5500;
            CColorManager::shieldGaugeColor = 0xff0000a5;
            CColorManager::specialBlackColor = 0xff303030;
            CColorManager::specialWhiteColor = 0xffe0e0e0;
            CColorManager::teamColors[0] = 0xffffffff;
            CColorManager::teamColors[1] = 0xff006655;
            CColorManager::teamColors[2] = 0xffcccc00;
            CColorManager::teamColors[3] = 0xffcc5500;
            CColorManager::teamColors[4] = 0xffcc0099;
            CColorManager::teamColors[5] = 0xff9900cc;
            CColorManager::teamColors[6] = 0xff0099cc;
            CColorManager::teamColors[7] = 0xfff5daa3;
            CColorManager::teamColors[8] = 0xff89c4c7;
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
            CColorManager::energyGaugeColor = 0xff008f8f;
            CColorManager::grenadeSightPrimaryColor = 0xffffff00;
            CColorManager::grenadeSightSecondaryColor = 0xffffaa00;
            CColorManager::lookForwardColor = 0xff000000;
            CColorManager::missileLockColor = 0xffffaa00;
            CColorManager::missileSightPrimaryColor = 0xffffff00;
            CColorManager::missileSightSecondaryColor = 0xffffaa00;
            CColorManager::netDelay1Color = 0xffffe080;
            CColorManager::netDelay2Color = 0xffffff00;
            CColorManager::pinwheel1Color = 0xff1e1e66;
            CColorManager::pinwheel2Color = 0xff1e1e99;
            CColorManager::pinwheel3Color = 0xff1e1ecc;
            CColorManager::pinwheel4Color = 0xff1e1eff;
            CColorManager::plasmaGauge1Color = 0xffa33600;
            CColorManager::plasmaGauge2Color = 0xffff3600;
            CColorManager::plasmaSightsOffColor = 0xff008055;
            CColorManager::plasmaSightsOnColor = 0xffff5500;
            CColorManager::shieldGaugeColor = 0xff0000a5;
            CColorManager::specialBlackColor = 0xff303030;
            CColorManager::specialWhiteColor = 0xffe0e0e0;
            CColorManager::teamColors[0] = 0xffffffff;
            CColorManager::teamColors[1] = 0xff006655;
            CColorManager::teamColors[2] = 0xffcccc00;
            CColorManager::teamColors[3] = 0xffcc5500;
            CColorManager::teamColors[4] = 0xffcc0099;
            CColorManager::teamColors[5] = 0xff9900cc;
            CColorManager::teamColors[6] = 0xff0099cc;
            CColorManager::teamColors[7] = 0xfff5daa3;
            CColorManager::teamColors[8] = 0xff89c4c7;
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
            CColorManager::energyGaugeColor = 0xff008f00;
            CColorManager::grenadeSightPrimaryColor = 0xffffff00;
            CColorManager::grenadeSightSecondaryColor = 0xffff0000;
            CColorManager::lookForwardColor = 0xff000000;
            CColorManager::missileLockColor = 0xffff0000;
            CColorManager::missileSightPrimaryColor = 0xffffff00;
            CColorManager::missileSightSecondaryColor = 0xffff0000;
            CColorManager::netDelay1Color = 0xffffc000;
            CColorManager::netDelay2Color = 0xffffff00;
            CColorManager::pinwheel1Color = 0xff1e1e66;
            CColorManager::pinwheel2Color = 0xff1e1e99;
            CColorManager::pinwheel3Color = 0xff1e1ecc;
            CColorManager::pinwheel4Color = 0xff1e1eff;
            CColorManager::plasmaGauge1Color = 0xffa33600;
            CColorManager::plasmaGauge2Color = 0xffff3600;
            CColorManager::plasmaSightsOffColor = 0xff008000;
            CColorManager::plasmaSightsOnColor = 0xffff0000;
            CColorManager::shieldGaugeColor = 0xff003da5;
            CColorManager::specialBlackColor = 0xff303030;
            CColorManager::specialWhiteColor = 0xffe0e0e0;
            CColorManager::teamColors[0] = 0xffffffff;
            CColorManager::teamColors[1] = 0xff006600;
            CColorManager::teamColors[2] = 0xffcccc00;
            CColorManager::teamColors[3] = 0xffcc0000;
            CColorManager::teamColors[4] = 0xffcc0099;
            CColorManager::teamColors[5] = 0xff9900cc;
            CColorManager::teamColors[6] = 0xff0099cc;
            CColorManager::teamColors[7] = 0xffffa500;
            CColorManager::teamColors[8] = 0xff89c4c7;
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
