/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CWorldShader.h
    Created: Saturday, December 30, 1995, 08:12
    Modified: Saturday, December 30, 1995, 08:38
*/

#pragma once
#include "ARGBColor.h"
#include "CDirectObject.h"
#include "Types.h"

#define MAXTOTALSHADES 64

#ifdef USE_OLD_COLORS
    #define DEFAULT_LOW_SKY_COLOR 0xffc05020
    #define DEFAULT_HIGH_SKY_COLOR 0xff2050c0
    #define DEFAULT_GROUND_COLOR 0xff000000
    #define DEFAULT_SKY_SHADE_COUNT 8
#else
    #define DEFAULT_LOW_SKY_COLOR 0xff0000cc
    #define DEFAULT_HIGH_SKY_COLOR 0xff000033
    #define DEFAULT_GROUND_COLOR 0xff000033
    #define DEFAULT_SKY_SHADE_COUNT 6
#endif

class CAvaraGame;
class CViewParameters;

class CWorldShader : public CDirectObject {
public:
    ARGBColor lowSkyColor = DEFAULT_LOW_SKY_COLOR;
    ARGBColor highSkyColor = DEFAULT_HIGH_SKY_COLOR;
    ARGBColor groundColor = DEFAULT_GROUND_COLOR;
    short skyShadeCount = DEFAULT_SKY_SHADE_COUNT;
    Fixed lowSkyAltitude;
    Fixed highSkyAltitude;
    float hazeDensity;

    short numShades;
    short shadeColors[MAXTOTALSHADES];
    Fixed altitudes[MAXTOTALSHADES];


    CWorldShader();
    virtual void Reset();
    virtual void Apply();
};
