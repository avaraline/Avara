/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: CGroundColorAdjuster.c
    Created: Tuesday, November 29, 1994, 08:24
    Modified: Saturday, December 30, 1995, 08:39
*/

#include "CGroundColorAdjuster.h"

#include "AbstractRenderer.h"
#include "CWorldShader.h"

CAbstractActor *CGroundColorAdjuster::EndScript() {
    CAbstractActor::EndScript();

    auto theShader = gRenderer->skyParams;
    theShader->groundMaterial = GetPixelMaterial();

    delete this;
    return NULL;
}
