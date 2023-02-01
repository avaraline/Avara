/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CWorldShader.h
    Created: Saturday, December 30, 1995, 08:12
    Modified: Saturday, December 30, 1995, 08:38
*/

#pragma once
#include "AvaraGL.h"
#include "CDirectObject.h"
#include "Types.h"

#define MAXTOTALSHADES 64

class CAvaraGame;
class CViewParameters;

class CWorldShader : public CDirectObject {
public:
    CAvaraGame *itsGame;

    uint32_t lowSkyColor;
    uint32_t highSkyColor;
    uint32_t groundColor;
    short skyShadeCount;
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
