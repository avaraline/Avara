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
#include "Resource.h"
#include "CBSPPart.h"
#include "AvaraGL.h"

void CWorldShader::IWorldShader(CAvaraGame *theGame) {
    itsGame = theGame;
    Reset();
}

void CWorldShader::Reset() {
#ifdef USE_OLD_COLORS
    lowSkyColor = 0xC05020;
    highSkyColor = 0x2050C0;
    groundColor = 0;
    skyShadeCount = 8;
#endif
    lowSkyColor = 0xCC;
    highSkyColor = 0x33;
    groundColor = 0x33;
    skyShadeCount = 6;

    lowSkyAltitude = FIX(0);
    highSkyAltitude = FIX(500);
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

    SetPolyWorld(&itsGame->itsPolyWorld);
    if(numShades == 0)
    {	SetPolyWorldBackground(FindPolyColor(groundColor));
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

    *colors++ = FindPolyColor(groundColor);
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

void CWorldShader::ShadeWorld(CViewParameters *theView) {
    AvaraGLShadeWorld(this, theView);
    /*
    if(numShades)
        theView->RenderPlanes(numShades, shadeColors, altitudes);
    */
    
}
