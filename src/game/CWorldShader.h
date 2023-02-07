/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CWorldShader.h
    Created: Saturday, December 30, 1995, 08:12
    Modified: Saturday, December 30, 1995, 08:38
*/

#pragma once
#include "ARGBColor.h"
#include "AvaraGL.h"
#include "CDirectObject.h"
#include "Types.h"

#define MAXTOTALSHADES 64

#ifdef USE_OLD_COLORS
    #define DEFAULT_LOW_SKY_COLOR 0xffc05020
    #define DEFAULT_HIGH_SKY_COLOR 0xff2050c0
    #define DEFAULT_GROUND_COLOR 0xff000000
    #define DEFAULT_SKY_SHADE_COUNT 8
#else
    #define DEFAULT_LOW_SKY_COLOR 0xffcc0000
    #define DEFAULT_HIGH_SKY_COLOR 0xff330000
    #define DEFAULT_GROUND_COLOR 0xff330000
    #define DEFAULT_SKY_SHADE_COUNT 6
#endif

class CAvaraGame;
class CViewParameters;

class CWorldShader : public CDirectObject {
public:
    CAvaraGame *itsGame;

    ARGBColor lowSkyColor = DEFAULT_LOW_SKY_COLOR;
    ARGBColor highSkyColor = DEFAULT_HIGH_SKY_COLOR;
    ARGBColor groundColor = DEFAULT_GROUND_COLOR;
    short skyShadeCount = DEFAULT_SKY_SHADE_COUNT;
    Fixed lowSkyAltitude;
    Fixed highSkyAltitude;

    short numShades;
    short shadeColors[MAXTOTALSHADES];
    Fixed altitudes[MAXTOTALSHADES];


    virtual void IWorldShader(CAvaraGame *theGame);
    virtual void IWorldShader();
    virtual void Reset();
    virtual void Apply();
    virtual void ShadeWorld(CViewParameters *theView);
};
