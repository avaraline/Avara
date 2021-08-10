/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CSliverPart.c
    Created: Monday, November 28, 1994, 04:17
    Modified: Wednesday, August 14, 1996, 14:13
*/

#include "CSliverPart.h"

#include "CAvaraGame.h"

extern ColorRecord ***bspColorLookupTable;
Fixed sliverGravity = -FIX3(20);

#if 0
void	CSliverPart::GenerateColorLookupTable()
{
    **bspColorLookupTable = &borrowedColors;
}
#endif

Fixed FSysRandom() {
    return rand() & 0x0000FFFF;
}

void CSliverPart::ISliverPart(short partNum) {
    ISmartPart(partNum, NULL, 0);
    ignoreDirectionalLights = true;
    nextSliver = NULL;
}

void CSliverPart::Activate(Fixed *origin,
    Fixed *direction,
    Fixed scale,
    Fixed speedFactor,
    short spread,
    short age,
    CBSPPart *fromObject) {
    PolyRecord *borrowPoly;
    int offset = FSysRandom() % (fromObject->polyCount);
    borrowPoly = &fromObject->polyTable[offset];

    ColorRecord c = borrowPoly->color;

    ReplaceColor(0x00fefefe, c); // marker color

    Matrix tm;
    Fixed vLen;
    Fixed smallVector[2];

    // TODO: update for new BSP code
    /*
    ColorRecord	*colorP;
    NormalRecord	*borrowNorms;
    short		borrowPoly, borrowColor;

    if(fromObject->colorReplacements)
        colorP = (ColorRecord *) *fromObject->colorReplacements;
    else
        colorP = (ColorRecord *) (fromObject->header.colorOffset + *fromObject->itsBSPResource);

    borrowPoly = (FRandomBeta() * fromObject->header.normalCount)>>16;
    borrowNorms = (NormalRecord *)(fromObject->header.normalOffset + *fromObject->itsBSPResource);
    borrowedColors = colorP[borrowNorms[borrowPoly].colorIndex];
    fakeMaster = &borrowedColors;
    colorReplacements = (Handle)&fakeMaster;
    */
    lifeCount = age;

    Reset();
    TranslatePartZ(this, FMul(scale, FIX3(250) + (FSysRandom() >> 1)));
    RotateX(spread * FSysRandom());
    RotateZ(360 * FSysRandom());

    smallVector[0] = direction[0];
    smallVector[1] = direction[2];
    vLen = VectorLength(2, smallVector);

    if (vLen > 4) //	Some small number to avoid funny division results.
    {
        MRotateY(FDivNZ(direction[0], vLen), FDivNZ(direction[2], vLen), &itsTransform);
    }

    MRotateX(direction[1], vLen, &itsTransform);

    speed[0] = FMul(speedFactor, itsTransform[3][0]);
    speed[1] = FMul(speedFactor, itsTransform[3][1]);
    speed[2] = FMul(speedFactor, itsTransform[3][2]);

    TranslatePart(this, origin[0], origin[1], origin[2]);
    MoveDone();
    extraAmbient = 0;

    gravity = FMul(sliverGravity, gCurrentGame->gravityRatio);
}

Boolean CSliverPart::SliverAction() {
    if (gCurrentGame->isClassicFrame && --lifeCount) {
        OffsetPart(speed);

        if (itsTransform[3][1] < 0) {
            itsTransform[3][1] = -itsTransform[3][1];
            speed[1] = FMul(speed[1], FIX3(-600));
        }
        speed[1] += gravity;
        speed[0] = FMul(speed[0], FIX3(980));
        speed[1] = FMul(speed[1], FIX3(980));
        speed[2] = FMul(speed[2], FIX3(980));

        extraAmbient = FIX3(500) - (FIX3(2000) >> lifeCount);
    }

    return lifeCount == 0;
}

void CSliverPart::Dispose() {
    // colorReplacements = NULL;	//	Destroy fake handle.

    CSmartPart::Dispose();
}
