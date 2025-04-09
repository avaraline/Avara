/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CWallDoor.c
    Created: Thursday, December 7, 1995, 09:18
    Modified: Saturday, August 10, 1996, 18:30
*/

#include "CWallDoor.h"

#include "AbstractRenderer.h"
#include "CBSPWorld.h"
#include "CSmartPart.h"
#include "CWallActor.h"

extern CWallActor *lastWallActor;

void CWallDoor::LoadPart(short ind, short resId) {
    if (lastWallActor) {
        CSmartPart *thePart;

        thePart = lastWallActor->partList[0];
        thePart->theOwner = this;
        if (!IsGeometryStatic() && thePart->userFlags & CBSPUserFlags::kIsStatic) {
            thePart->userFlags -= CBSPUserFlags::kIsStatic;
        }

        partList[ind] = thePart;

        //TranslatePartY(thePart, ReadLongVar(iHeight));
        VECTORCOPY(location, thePart->itsTransform[3]);
        gRenderer->RemovePart(thePart);

        heading = 0;
        lastWallActor->partList[0] = NULL;
        lastWallActor->partCount = 0;
        delete lastWallActor;
        lastWallActor = NULL;
    } else {
        CDoor2Actor::LoadPart(ind, resId);
    }
}
