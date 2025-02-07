/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CYonBox.c
    Created: Thursday, August 22, 1996, 03:11
    Modified: Sunday, September 1, 1996, 20:35
*/

#include "CYonBox.h"

extern Rect gLastBoxRect;
extern Fixed gLastBoxRounding;

void CYonBox::BeginScript() {
    CAbstractYon::BeginScript();

    ProgramLongVar(iDeltaY, 0);
}

CAbstractActor *CYonBox::EndScript() {
    if (CAbstractYon::EndScript()) {
        Fixed deltaY;

        minBounds[0] = gLastBoxRect.left;
        minBounds[2] = gLastBoxRect.top;

        maxBounds[0] = gLastBoxRect.right;
        maxBounds[2] = gLastBoxRect.bottom;

        deltaY = ReadFixedVar(iDeltaY);
        if (deltaY <= 0) {
            deltaY = gLastBoxRounding;// * ReadFixedVar(iPixelToThickness);
        }

        if (deltaY <= 0) {
            minBounds[1] = 0x80000000;
            maxBounds[1] = 0x7FFFffff;
            deltaY = 0;
        } else {
            minBounds[1] = ReadFixedVar(iHeight) + ReadFixedVar(iBaseHeight);
            maxBounds[1] = minBounds[1] + deltaY;
        }

        sphere[0] = (minBounds[0] + maxBounds[0]) >> 1;
        sphere[1] = (minBounds[1] + maxBounds[1]) >> 1;
        sphere[2] = (minBounds[2] + maxBounds[2]) >> 1;
        sphere[3] = FDistanceOverEstimate(maxBounds[0] - minBounds[0], deltaY, maxBounds[2] - minBounds[2]) >> 1;

        SetYon();

        sphere[3] = 0x7FFFffff;

        return this;
    } else {
        return NULL;
    }
}
