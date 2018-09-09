/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CScaledBSP.c
    Created: Thursday, March 28, 1996, 18:19
    Modified: Thursday, March 28, 1996, 18:21
*/

#include "CScaledBSP.h"

#include "Memory.h"
#include "Resource.h"

void CScaledBSP::IScaledBSP(Fixed scale, short resId, CAbstractActor *anActor, short aPartCode) {
    OSErr iErr;
    Vector *p;
    short i;

    CSmartPart::ISmartPart(resId, anActor, aPartCode);

    p = (Vector *)pointTable;
    for (i = 0; i < pointCount; i++) {
        (*p)[0] = FMul((*p)[0], scale);
        (*p)[1] = FMul((*p)[1], scale);
        (*p)[2] = FMul((*p)[2], scale);
        p++;
    }

    minBounds.x = FMul(minBounds.x, scale);
    minBounds.y = FMul(minBounds.y, scale);
    minBounds.z = FMul(minBounds.z, scale);

    maxBounds.x = FMul(maxBounds.x, scale);
    maxBounds.y = FMul(maxBounds.y, scale);
    maxBounds.z = FMul(maxBounds.z, scale);

    enclosureRadius = FMul(enclosureRadius, scale);
    enclosurePoint.w = FIX1;

    MoveDone();

    rSquare[0] = 0;
    rSquare[1] = 0;
    FSquareAccumulate(enclosureRadius, rSquare);
}

void CScaledBSP::Dispose() {
    /*
    Handle				handCopy;
    BSPResourceHeader	*bp;

    handCopy = itsBSPResource;
    bp = (BSPResourceHeader *)*handCopy;
    bp->refCount = 99;	//	Prevent ReleaseResource call!
    */
    CSmartPart::Dispose();

    // DisposeHandle(handCopy);
}
