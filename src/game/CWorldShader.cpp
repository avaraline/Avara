/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CWorldShader.c
    Created: Saturday, December 30, 1995, 08:15
    Modified: Thursday, September 5, 1996, 00:33
*/

#include "CWorldShader.h"

#include "CAvaraApp.h"
#include "CAvaraGame.h"
#include "CViewParameters.h"
#include "CBSPPart.h"

CWorldShader::CWorldShader() {
    Reset();
}

void CWorldShader::Reset() {
    lowSkyColor = DEFAULT_LOW_SKY_COLOR;
    highSkyColor = DEFAULT_HIGH_SKY_COLOR;
    groundMaterial = Material(DEFAULT_GROUND_COLOR);
    skyShadeCount = DEFAULT_SKY_SHADE_COUNT;

    lowSkyAltitude = FIX(0);
    highSkyAltitude = FIX(1000);
    hazeDensity = 0.0f;
}
