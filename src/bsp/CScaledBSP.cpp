/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CScaledBSP.c
    Created: Thursday, March 28, 1996, 18:19
    Modified: Thursday, March 28, 1996, 18:21
*/

#include "CScaledBSP.h"

#include "Memory.h"
#include "AvaraGL.h"

CScaledBSP::CScaledBSP(Fixed scale, short resId, CAbstractActor *anActor, short aPartCode) {
    isMorphable = false;

    CSmartPart::ISmartPart(resId, anActor, aPartCode);

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
    
    SetScale(scale, scale, scale);
}

void CScaledBSP::Scale(Fixed scale) {
    if (!isMorphable) return;

    
    SetScale(scale, scale, scale);
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
}

void CScaledBSP::ScaleXYZ(Fixed scaleX, Fixed scaleY, Fixed scaleZ) {
    if (!isMorphable) return;
   SetScale(scaleX, scaleY, scaleZ);
}

void CScaledBSP::Reset() {
    CBSPPart::Reset();

    if (isMorphable) {
       ResetScale();
    }
}

void CScaledBSP::Dispose() {
    CSmartPart::Dispose();
}
