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
#include "AvaraGL.h"

void CScaledBSP::IScaledBSP(Fixed scale, short resId, CAbstractActor *anActor, short aPartCode) {
    //OSErr iErr;
    Vector *p;
    size_t i;

    isMorphable = false;

    CSmartPart::ISmartPart(resId, anActor, aPartCode);

    p = pointTable.get();
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
    AvaraGLUpdateData(this);
}

void CScaledBSP::Scale(Fixed scale) {
    if (!isMorphable) return;
    
    Vector *p;
    size_t i;

    p = pointTable.get();
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

    // Scale the bounding sphere
    enclosureRadius = FMul(enclosureRadius, scale);
    enclosurePoint.w = FIX1;

    rSquare[0] = 0;
    rSquare[1] = 0;
    FSquareAccumulate(enclosureRadius, rSquare);
    AvaraGLUpdateData(this);
}

void CScaledBSP::ScaleXYZ(Fixed scaleX, Fixed scaleY, Fixed scaleZ) {
    if (!isMorphable) return;

    Vector *p;
    size_t i;
    Fixed avgScale = (scaleX + scaleY + scaleZ) / 3;

    p = pointTable.get();
    for (i = 0; i < pointCount; i++) {
        (*p)[0] = FMul((*p)[0], scaleX);
        (*p)[1] = FMul((*p)[1], scaleY);
        (*p)[2] = FMul((*p)[2], scaleZ);
        p++;
    }

    AvaraGLUpdateData(this);
}

void CScaledBSP::Reset() {
    CBSPPart::Reset();

    if (isMorphable) {
        Vector *p, *orig;
        size_t i;

        orig = origPointTable.get();
        p = pointTable.get();
        for (i = 0; i < pointCount; i++) {
            (*p)[0] = (*orig)[0];
            (*p)[1] = (*orig)[1];
            (*p)[2] = (*orig)[2];
            p++;
            orig++;
        }
    }
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
