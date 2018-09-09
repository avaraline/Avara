/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CSwitchActor.c
    Created: Tuesday, January 16, 1996, 00:12
    Modified: Saturday, August 10, 1996, 19:35
*/

#include "CSwitchActor.h"

#include "CSmartPart.h"

void CSwitchActor::BeginScript() {
    CGlowActors::BeginScript();

    ProgramLongVar(iOutVar, 0);
    ProgramReference(iFirstOut, iOutVar);
    ProgramReference(iFirstOut + 1, iOutVar);

    ProgramLongVar(iInVar, 0);
    ProgramReference(iFirstIn, iInVar);
    ProgramReference(iFirstIn + 1, iInVar);

    ProgramLongVar(iShape, 562);
    ProgramOffsetAdd(iAltShape, iShape, 1);

    ProgramLongVar(iRestartFlag, true);
    ProgramLongVar(iTogglePower, 0);
    ProgramLongVar(iBlastToggle, 1000);

    ProgramFixedVar(iHeight, FIX3(1500));
    ProgramLongVar(iStatus, 0);
    ProgramLongVar(iIsTarget, true);

    ProgramLongVar(iHitVolume, 2);
    ProgramLongVar(iHitSound, 240);
}

CAbstractActor *CSwitchActor::EndScript() {
    if (CGlowActors::EndScript()) {
        short onShape, offShape;

        maskBits |= kSolidBit + kDoorIgnoreBit + kCollisionDamageBit;
        heading += 0x8000;
        offMessage = ReadLongVar(iFirstOut);
        onMessage = ReadLongVar(iFirstOut + 1);

        RegisterReceiver(&offActivator, ReadLongVar(iFirstIn));
        RegisterReceiver(&onActivator, ReadLongVar(iFirstIn + 1));
        msgToggle = offActivator.messageId == onActivator.messageId;
        togglePower = ReadFixedVar(iTogglePower);
        blastToggle = ReadFixedVar(iBlastToggle);
        theState = ReadLongVar(iStatus);
        shownState = theState;

        isActive = false;

        toggleCount = 0;
        restart = ReadLongVar(iRestartFlag);
        enabled = true;

        onShape = ReadLongVar(iAltShape);
        offShape = ReadLongVar(iShape);
        LoadPartWithColors(0, offShape);
        partList[0]->Reset();
        InitialRotatePartY(partList[0], heading);
        TranslatePart(partList[0], location[0], location[1], location[2]);
        partList[0]->MoveDone();

        if (onShape == offShape) {
            partCount = 1;
        } else {
            partCount = 2;
            LoadPartWithColors(1, onShape);
            partList[1]->Reset();
            InitialRotatePartY(partList[1], heading);
            TranslatePart(partList[1], location[0], location[1], location[2]);
            partList[1]->MoveDone();
        }

        PlaceParts();
        isActive = kIsInactive;
        return this;
    } else {
        return NULL;
    }
}

void CSwitchActor::PlaceParts() {
    if (shownState && partCount > 1) {
        partList[0]->isTransparent = true;
        partList[1]->isTransparent = false;
    } else {
        partList[0]->isTransparent = false;
        partList[1]->isTransparent = true;
    }

    LinkPartSpheres();
}

CSmartPart *CSwitchActor::CollisionTest() {
    CAbstractActor *theActor;
    CSmartPart *thePart;
    CSmartPart **thePartList;
    CSmartPart *myPart;

    myPart = partList[shownState];

    theActor = itsGame->actorList;

    for (thePart = proximityList.p; thePart; thePart = (CSmartPart *)thePart->nextTemp) {
        theActor = thePart->theOwner;
        if (!(theActor->maskBits & kDoorIgnoreBit) && myPart->CollisionTest(thePart)) {
            return thePart;
        }
    }

    return NULL;
}

void CSwitchActor::FrameAction() {
    if (isActive & kHasMessage) {
        isActive &= ~kHasMessage;

        if (msgToggle) {
            toggleCount += onActivator.triggerCount;
        } else {
            while (onActivator.triggerCount && offActivator.triggerCount) {
                onActivator.triggerCount--;
                offActivator.triggerCount--;
                toggleCount += 2;
            }

            if (theState && offActivator.triggerCount) {
                toggleCount++;
            }
            if (!theState && onActivator.triggerCount) {
                toggleCount++;
            }
        }
        onActivator.triggerCount = 0;
        offActivator.triggerCount = 0;
        DoSound(hitSoundId, location, hitSoundVolume << 15, FIX(1));
    }

    if (enabled) {
        while (enabled && toggleCount--) {
            theState = !theState;
            if (theState)
                itsGame->FlagMessage(onMessage);
            else
                itsGame->FlagMessage(offMessage);

            enabled = restart;
        }

        toggleCount = 0;
    } else {
        toggleCount = 0;
    }

    if (shownState != theState) {
        shownState = theState;
        isActive |= kIsActive;

        PlaceParts();
        BuildPartSpheresProximityList(kSolidBit);
        if (CollisionTest()) {
            shownState = !theState;
            PlaceParts();
        } else {
            isActive &= ~kIsActive;
        }
    }
}

void CSwitchActor::WasHit(RayHitRecord *theHit, Fixed hitEnergy) {
    CGlowActors::WasHit(theHit, hitEnergy);

    if (hitEnergy >= togglePower) {
        isActive |= kIsActive;
        toggleCount++;
    }
}

void CSwitchActor::BlastHit(BlastHitRecord *theHit) {
    Fixed savedToggle;

    savedToggle = togglePower;
    togglePower = blastToggle;

    CGlowActors::BlastHit(theHit);

    togglePower = savedToggle;
}

void CSwitchActor::Dispose() {
    itsGame->RemoveReceiver(&onActivator);
    itsGame->RemoveReceiver(&offActivator);

    CGlowActors::Dispose();
}
