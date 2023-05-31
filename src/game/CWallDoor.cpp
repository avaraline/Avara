/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CWallDoor.c
    Created: Thursday, December 7, 1995, 09:18
    Modified: Saturday, August 10, 1996, 18:30
*/

#include "CWallDoor.h"

#include "CBSPWorld.h"
#include "CSmartPart.h"
#include "CWallActor.h"

extern CWallActor *lastWallActor;

void CWallDoor::LoadPart(short ind, short resId) {
    if (lastWallActor) {
        partList[ind] = std::move(lastWallActor->partList[0]);
        CSmartPart *thePart = partList[ind].get();
        thePart->theOwner = this;

        //TranslatePartY(thePart, ReadLongVar(iHeight));
        VECTORCOPY(location, thePart->itsTransform[3]);
        itsGame->itsWorld->RemovePart(thePart);

        heading = 0;
        lastWallActor->partList[0] = nullptr;
        lastWallActor->partCount = 0;
        lastWallActor->Dispose();
        lastWallActor = NULL;
    } else {
        CDoor2Actor::LoadPart(ind, resId);
    }
}
