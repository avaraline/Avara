/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CForceField.c
    Created: Saturday, January 20, 1996, 04:36
    Modified: Sunday, August 25, 1996, 08:54
*/

#include "CForceField.h"

#include "CBSPWorld.h"
#include "CSmartPart.h"
#include "CWallActor.h"
#include "RenderManager.h"

extern CWallActor *lastWallActor;

void CForceField::BeginScript() {
    CPlacedActors::BeginScript();

    ProgramMessage(iStartMsg, iStartMsg);
    ProgramLongVar(iStopMsg, 0);
    ProgramLongVar(iShape, 0);
    ProgramVariable(iRoll, 0);
    ProgramLongVar(iVisible, 0);
    ProgramFixedVar(iDeltaX, 0);
    ProgramFixedVar(iDeltaY, 0);
    ProgramFixedVar(iDeltaZ, 0);

    ProgramLongVar(iWatchMask, kCanPushBit);
    ProgramLongVar(iMask, -1);

    ProgramLongVar(iExit, 0);
    ProgramLongVar(iEnter, 0);

    ProgramLongVar(iSpeed, 0);
}

CAbstractActor *CForceField::EndScript() {
    if (CPlacedActors::EndScript()) {
        short shapeId;

        RegisterReceiver(&startMsg, ReadLongVar(iStartMsg));
        RegisterReceiver(&stopMsg, ReadLongVar(iStopMsg));
        shapeId = ReadLongVar(iShape);
        watchBits = ReadLongVar(iWatchMask);
        watchTeams = ReadLongVar(iMask);

        if (shapeId) {
            partCount = 1;
            LoadPartWithColors(0, shapeId);
            partList[0]->RotateZ(ReadFixedVar(iRoll));
            partList[0]->RotateOneY(heading);
            TranslatePart(partList[0], location[0], location[1], location[2]);
            partList[0]->MoveDone();
        } else if (lastWallActor) {
            CSmartPart *thePart;

            thePart = lastWallActor->partList[0];
            thePart->theOwner = this;

            partCount = 1;
            partList[0] = thePart;

            //TranslatePartY(thePart, ReadLongVar(iHeight));
            VECTORCOPY(location, thePart->itsTransform[3]);
            RenderManager::RemovePart(thePart);

            heading = 0;
            lastWallActor->partList[0] = NULL;
            lastWallActor->partCount = 0;
            delete lastWallActor;
            lastWallActor = NULL;
        } else {
            delete this;
            return NULL;
        }

        partList[0]->isTransparent = !ReadLongVar(iVisible);
        if (!ReadLongVar(iVisible)) {
            // Force yon when visible is false, to hide from level previews.
            partList[0]->yon = FIX(0.001);
            partList[0]->usesPrivateYon = true;
        }

        exitMsg = ReadLongVar(iExit);
        enterMsg = ReadLongVar(iEnter);

        isActive = kIsInactive;
        isWorking = false;

        force[0] = ReadFixedVar(iDeltaX);
        force[1] = ReadFixedVar(iDeltaY);
        force[2] = ReadFixedVar(iDeltaZ);

        spinSpeed = FDegToOne(ReadFixedVar(iSpeed));

        return this;
    } else {
        return NULL;
    }
}

void CForceField::FrameAction() {
    CSmartPart *thePart;

    if (isActive & kHasMessage) {
        isActive &= ~kHasMessage;

        if (stopMsg.triggerCount) {
            isActive &= ~kIsActive;
            stopMsg.triggerCount = 0;
        }

        if (startMsg.triggerCount) {
            isActive |= kIsActive;
            startMsg.triggerCount = 0;
            isWorking = false;
        }
    }

    CPlacedActors::FrameAction();

    if (isActive & kIsActive) {
        Boolean oldWorking = isWorking;
        isWorking = false;

        BuildPartProximityList(partList[0]->sphereGlobCenter, partList[0]->enclosureRadius, watchBits);

        searchCount = ++itsGame->searchCount;
        for (thePart = proximityList.p; thePart; thePart = (CSmartPart *)thePart->nextTemp) {
            CAbstractActor *theActor;

            theActor = (CAbstractActor *)thePart->theOwner;
            if ((theActor->searchCount != searchCount) && (theActor->teamMask & watchTeams) &&
                thePart->CollisionTest(partList[0])) {
                theActor->searchCount = searchCount;
                theActor->Accelerate(force);
                isWorking = true;
            }
        }

        if (isWorking != oldWorking) {
            itsGame->FlagMessage(isWorking ? enterMsg : exitMsg);
        }

        if (spinSpeed) {
            heading += spinSpeed;

            partList[0]->Reset();
            InitialRotatePartY(partList[0], heading);
            TranslatePart(partList[0], location[0], location[1], location[2]);
            partList[0]->MoveDone();
        }
    }
}
