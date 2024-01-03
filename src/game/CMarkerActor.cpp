/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: CMarkerActor.c
    Created: Tuesday, November 22, 1994, 02:17
    Modified: Thursday, November 23, 1995, 15:10
*/

#include "CMarkerActor.h"

#include "CSmartPart.h"

CMarkerActor::CMarkerActor() {
    isActive = kIsActive;
    maskBits = kTargetBit; // + kMarkerBit; // TODO: what is this??
}

CAbstractActor *CMarkerActor::EndScript() {
    CGlowActors::EndScript();

    partCount = 1;
    LoadPart(0, 230 /*kMarkerBSP*/);
    TranslatePart(partList[0], location[0], location[1], location[2]);
    partList[0]->MoveDone();
    //	LinkPartSpheres();

    return this;
}

void CMarkerActor::FrameAction() {
    CGlowActors::FrameAction();

    heading -= FDegToOne(FIX(15));
    //	UnlinkLocation();
    partList[0]->Reset();
    InitialRotatePartY(partList[0], heading);
    TranslatePart(partList[0], location[0], location[1], location[2]);
    partList[0]->MoveDone();
    //	LinkPartSpheres();
}
