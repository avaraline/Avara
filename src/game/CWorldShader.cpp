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

#define GROUNDOFFSET FIX(-2)

void CWorldShader::Apply() {
    /*
    short		i;
    short		tot;
    short		*colors;
    Fixed		*alts;
    Fixed		altRange;
    short		highColor[3];
    short		deltaColor[3];
    short		detailLevel;
    short		shadeCount;

    detailLevel = gApplication->ReadShortPref(kHorizonDetailTag,
                        ((CAvaraApp *)gApplication)->fastMachine ? 2 : 0);

    switch(detailLevel)
    {	case 0:
            if(skyShadeCount > 0)	numShades = 1;
            break;
        case 1:
            numShades = FSqrt(skyShadeCount * 2L) >> 8;
            break;
        case 2:
        default:
            numShades = skyShadeCount;
            break;
    }

    SetPolyWorld(&gCurrentGame->itsPolyWorld);
    if(numShades == 0)
    {	SetPolyWorldBackground(FindPolyColor(groundMaterial.GetColor()));
    }
    else
    if(numShades > 1)
    {	SetPolyWorldBackground(FindPolyColor(lowSkyColor));
    }
    else
    {	SetPolyWorldBackground(FindPolyColor(highSkyColor));
    }

    colors = shadeColors;
    alts = altitudes;

    *colors++ = FindPolyColor(groundMaterial.GetColor());
    *alts++ = GROUNDOFFSET;

    tot = numShades - 1;

    if(tot > 0)
    {	altRange = highSkyAltitude - lowSkyAltitude;

        highColor[0] = (unsigned char)(highSkyColor >> 16);
        highColor[1] = (unsigned char)(highSkyColor >> 8);
        highColor[2] = (unsigned char)(highSkyColor);
        deltaColor[0] = (unsigned char)(lowSkyColor >> 16);
        deltaColor[1] = (unsigned char)(lowSkyColor >> 8);
        deltaColor[2] = (unsigned char)(lowSkyColor);
        deltaColor[0] -= highColor[0];
        deltaColor[1] -= highColor[1];
        deltaColor[2] -= highColor[2];

        for(i = tot-1;i >= 0;i--)
        {	RGBColor	interColor;

            interColor.red = (highColor[0] + deltaColor[0] * i / numShades)<<8;
            interColor.green = (highColor[1] + deltaColor[1] * i / numShades)<<8;
            interColor.blue = (highColor[2] + deltaColor[2] * i / numShades)<<8;

            *alts++ = highSkyAltitude + GROUNDOFFSET - FMulDivNZ(altRange, i+1, numShades);
            *colors++ = FindPolyColor(RGBToLongColor(&interColor));
        }
    }
    */
}
