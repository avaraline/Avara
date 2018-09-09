/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CLevelDescriptor.c
    Created: Wednesday, November 30, 1994, 08:15
    Modified: Thursday, August 29, 1996, 16:31
*/

#include "CLevelDescriptor.h"

#include "CharWordLongPointer.h"
#include "Memory.h"
#include "Resource.h"

CLevelDescriptor *LoadLevelListFromResource(OSType *currentDir) {
    Handle theRes;
    CLevelDescriptor *firstLevel = NULL;
    charWordLongP uniP;
    long directoryTag;

    theRes = GetResource(LEVELLISTRESTYPE, LEVELLISTRESID);

    if (theRes) {
        short levelCount;

        HLock(theRes);
        uniP.c = *theRes;
        *currentDir = ntohl(*uniP.l++);
        levelCount = ntohs(*uniP.w++);

        firstLevel = new CLevelDescriptor;
        firstLevel->ILevelDescriptor(uniP.c, levelCount);

        HUnlock(theRes);
        ReleaseResource(theRes);
    }

    return firstLevel;
}

void CLevelDescriptor::ILevelDescriptor(Ptr levelInfo, short levelsLeft) {
    charWordLongP uniP;
    short i;

    orderNumber = levelsLeft;
    uniP.c = levelInfo;

    tag = ntohl(*(uniP.l++));
    CopyEvenPascalString(&uniP, name, sizeof(name));
    CopyEvenPascalString(&uniP, intro, sizeof(intro));
    CopyEvenPascalString(&uniP, access, sizeof(access));
    enablesNeeded = ntohs(*(uniP.w++));
    fromFile = ntohs(*(uniP.uw++)) >> 8;

    uniP.l++; //	Skip reserved data

    countEnables = ntohs(*(uniP.w++));
    if (countEnables > MAXENABLES)
        countEnables = MAXENABLES;

    for (i = 0; i < countEnables; i++) {
        winEnables[i] = ntohl(*(uniP.l++));
    }

    if (--levelsLeft) {
        levelInfo = uniP.c;
        nextLevel = new CLevelDescriptor;
        nextLevel->ILevelDescriptor(levelInfo, levelsLeft);
    } else {
        nextLevel = NULL;
    }
}

void CLevelDescriptor::Dispose() {
    if (nextLevel) {
        nextLevel->Dispose();
    }

    CDirectObject::Dispose();
}
