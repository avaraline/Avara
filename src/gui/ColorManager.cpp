#include "ColorManager.h"

#include <algorithm>

ColorBlindMode ColorManager::colorBlindMode = Off;
float ColorManager::hudAlpha = 1.f;
ARGBColor ColorManager::energyGaugeColor = 0xff009c00;
ARGBColor ColorManager::grenadeSightPrimaryColor = 0xfffefb00;
ARGBColor ColorManager::grenadeSightSecondaryColor = 0xffff2600;
ARGBColor ColorManager::lookForwardColor = 0xff000000;
ARGBColor ColorManager::missileLockColor = 0xffff2600;
ARGBColor ColorManager::missileSightPrimaryColor = 0xfffefb00;
ARGBColor ColorManager::missileSightSecondaryColor = 0xffff2600;
ARGBColor ColorManager::missileArmedColor = 0xff400000;
ARGBColor ColorManager::missileLaunchedColor = 0xfffe0000;
ARGBColor ColorManager::netDelay1Color = 0xffffc900;
ARGBColor ColorManager::netDelay2Color = 0xfffefb00;
ARGBColor ColorManager::pinwheel1Color = 0xff282d79;
ARGBColor ColorManager::pinwheel2Color = 0xff2833a9;
ARGBColor ColorManager::pinwheel3Color = 0xff283ad5;
ARGBColor ColorManager::pinwheel4Color = 0xff2941ff;
ARGBColor ColorManager::plasmaGauge1Color = 0xffb44900;
ARGBColor ColorManager::plasmaGauge2Color = 0xffff4e00;
ARGBColor ColorManager::plasmaSightsOffColor = 0xff008e00;
ARGBColor ColorManager::plasmaSightsOnColor = 0xffff2600;
ARGBColor ColorManager::shieldGaugeColor = 0xff0053b4;

ARGBColor ColorManager::teamColors[kMaxTeamColors + 1] = {
    0xffffffff,
    0xff007600,
    0xffd5d200,
    0xffd71e00,
    0xffd72ca9,
    0xffab2fd5,
    0xff00a9d5,
    0xff3e3e3e,
    0xffe6e6e6
};

ARGBColor ColorManager::teamTextColors[kMaxTeamColors + 1] = {
    0xff333333,
    0xffffffff,
    0xff333333,
    0xffffffff,
    0xffffffff,
    0xffffffff,
    0xff333333,
    0xffffffff,
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
    "Black",
    "White"
};

ARGBColor ColorManager::messageColors[3] = {
    0xffffffff,
    0xff92ebe9,
    0xffff8185
};

ARGBColor ColorManager::pingColors[3] = {
    0xff2eff2e,
    0xffffee2e,
    0xffff382e
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
            ColorManager::teamColors[0] = 0xffffffff;
            ColorManager::teamColors[1] = 0xff007600;
            ColorManager::teamColors[2] = 0xffd5d200;
            ColorManager::teamColors[3] = 0xffd71e00;
            ColorManager::teamColors[4] = 0xffd72ca9;
            ColorManager::teamColors[5] = 0xffab2fd5;
            ColorManager::teamColors[6] = 0xff00a9d5;
            ColorManager::teamColors[7] = 0xff3e3e3e;
            ColorManager::teamColors[8] = 0xffe6e6e6;
            ColorManager::teamTextColors[0] = 0xff333333;
            ColorManager::teamTextColors[1] = 0xffffffff;
            ColorManager::teamTextColors[2] = 0xff333333;
            ColorManager::teamTextColors[3] = 0xffffffff;
            ColorManager::teamTextColors[4] = 0xffffffff;
            ColorManager::teamTextColors[5] = 0xffffffff;
            ColorManager::teamTextColors[6] = 0xff333333;
            ColorManager::teamTextColors[7] = 0xffffffff;
            ColorManager::teamTextColors[8] = 0xff333333;
            ColorManager::messageColors[0] = 0xffffffff;
            ColorManager::messageColors[1] = 0xff92ebe9;
            ColorManager::messageColors[2] = 0xffff8185;
            ColorManager::pingColors[0] = 0xff2eff2e;
            ColorManager::pingColors[1] = 0xffffee2e;
            ColorManager::pingColors[2] = 0xffff382e;
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
            ColorManager::teamColors[0] = 0xffffffff;
            ColorManager::teamColors[1] = 0xff007768;
            ColorManager::teamColors[2] = 0xffd5d200;
            ColorManager::teamColors[3] = 0xffd76a00;
            ColorManager::teamColors[4] = 0xffd72ca9;
            ColorManager::teamColors[5] = 0xffab2fd5;
            ColorManager::teamColors[6] = 0xff00a9d5;
            ColorManager::teamColors[7] = 0xff3e3e3e;
            ColorManager::teamColors[8] = 0xffe6e6e6;
            ColorManager::teamTextColors[0] = 0xff333333;
            ColorManager::teamTextColors[1] = 0xffffffff;
            ColorManager::teamTextColors[2] = 0xff333333;
            ColorManager::teamTextColors[3] = 0xffffffff;
            ColorManager::teamTextColors[4] = 0xffffffff;
            ColorManager::teamTextColors[5] = 0xffffffff;
            ColorManager::teamTextColors[6] = 0xffffffff;
            ColorManager::teamTextColors[7] = 0xffffffff;
            ColorManager::teamTextColors[8] = 0xff333333;
            ColorManager::messageColors[0] = 0xffffffff;
            ColorManager::messageColors[1] = 0xff8ee0ef;
            ColorManager::messageColors[2] = 0xffffbf80;
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
            ColorManager::teamColors[0] = 0xffffffff;
            ColorManager::teamColors[1] = 0xff007768;
            ColorManager::teamColors[2] = 0xffd5d200;
            ColorManager::teamColors[3] = 0xffd76a00;
            ColorManager::teamColors[4] = 0xffd72ca9;
            ColorManager::teamColors[5] = 0xffab2fd5;
            ColorManager::teamColors[6] = 0xff00a9d5;
            ColorManager::teamColors[7] = 0xff3e3e3e;
            ColorManager::teamColors[8] = 0xffe6e6e6;
            ColorManager::teamTextColors[0] = 0xff333333;
            ColorManager::teamTextColors[1] = 0xffffffff;
            ColorManager::teamTextColors[2] = 0xff333333;
            ColorManager::teamTextColors[3] = 0xffffffff;
            ColorManager::teamTextColors[4] = 0xffffffff;
            ColorManager::teamTextColors[5] = 0xffffffff;
            ColorManager::teamTextColors[6] = 0xffffffff;
            ColorManager::teamTextColors[7] = 0xffffffff;
            ColorManager::teamTextColors[8] = 0xff333333;
            ColorManager::messageColors[0] = 0xffffffff;
            ColorManager::messageColors[1] = 0xff8ee0ef;
            ColorManager::messageColors[2] = 0xffffbf80;
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
            ColorManager::teamColors[0] = 0xffffffff;
            ColorManager::teamColors[1] = 0xff007600;
            ColorManager::teamColors[2] = 0xffd5d200;
            ColorManager::teamColors[3] = 0xffd71e00;
            ColorManager::teamColors[4] = 0xffd72ca9;
            ColorManager::teamColors[5] = 0xffab2fd5;
            ColorManager::teamColors[6] = 0xff00a9d5;
            ColorManager::teamColors[7] = 0xff3e3e3e;
            ColorManager::teamColors[8] = 0xffe6e6e6;
            ColorManager::teamTextColors[0] = 0xff333333;
            ColorManager::teamTextColors[1] = 0xffffffff;
            ColorManager::teamTextColors[2] = 0xff333333;
            ColorManager::teamTextColors[3] = 0xffffffff;
            ColorManager::teamTextColors[4] = 0xffffffff;
            ColorManager::teamTextColors[5] = 0xffffffff;
            ColorManager::teamTextColors[6] = 0xffffffff;
            ColorManager::teamTextColors[7] = 0xffffffff;
            ColorManager::teamTextColors[8] = 0xff333333;
            ColorManager::messageColors[0] = 0xffffffff;
            ColorManager::messageColors[1] = 0xff92ebe9;
            ColorManager::messageColors[2] = 0xffff8185;
            break;
    }
    if (ColorManager::hudAlpha != 1.f) {
        ColorManager::setHudAlpha(ColorManager::hudAlpha);
    }
    ColorManager::colorBlindMode = mode;
}

void ColorManager::setHudAlpha(float alpha) {
    alpha = std::clamp(alpha, 0.f, 1.f);
    uint8_t a = static_cast<uint8_t>((alpha * 255) + 0.5);

    ColorManager::grenadeSightPrimaryColor = ColorManager::grenadeSightPrimaryColor.WithA(a);
    ColorManager::grenadeSightSecondaryColor = ColorManager::grenadeSightSecondaryColor.WithA(a);
    ColorManager::lookForwardColor = ColorManager::lookForwardColor.WithA(a);
    ColorManager::missileLockColor = ColorManager::missileLockColor.WithA(a);
    ColorManager::missileSightPrimaryColor = ColorManager::missileSightPrimaryColor.WithA(a);
    ColorManager::missileSightSecondaryColor = ColorManager::missileSightSecondaryColor.WithA(a);
    ColorManager::plasmaSightsOffColor = ColorManager::plasmaSightsOffColor.WithA(a);
    ColorManager::plasmaSightsOnColor = ColorManager::plasmaSightsOnColor.WithA(a);

    ColorManager::hudAlpha = alpha;
}

void ColorManager::setMissileArmedColor(ARGBColor color) {
    ColorManager::missileArmedColor = color;
}

void ColorManager::setMissileLaunchedColor(ARGBColor color) {
    ColorManager::missileLaunchedColor = color;
}
