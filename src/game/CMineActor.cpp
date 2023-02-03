/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CMineActor.c
    Created: Tuesday, September 5, 1995, 08:25
    Modified: Sunday, September 8, 1996, 22:35
*/

#include "CMineActor.h"

#include "CDepot.h"
#include "CSmartPart.h"

#define MINEACTIVESOUND 300
#define MINEBLOWSOUND 301

#define PRIMEMINEBSP 310
#define ALTMINEBSP 311

#define HITSOUNDV1 221

void CMineActor::BeginScript() {
    maskBits |= kTargetBit + kSolidBit;
    shields = FIX(1);
    hitScore = 5;
    destructScore = 20;

    CGlowActors::BeginScript();

    ProgramLongVar(iWatchMask, kPlayerBit);

    ProgramLongVar(iShape, PRIMEMINEBSP);
    ProgramLongVar(iAltShape, ALTMINEBSP);

    ProgramLongVar(iPower, 4);
    ProgramLongVar(iRange, 2);

    ProgramLongVar(iPhase, 0);
    ProgramLongVar(iFrequency, 4);

    ProgramLongVar(iActiveTimer, 0);

    ProgramLongVar(iIdleShapeTimer, 20);
    ProgramLongVar(iIdleAltShapeTimer, 0);
    ProgramLongVar(iActiveShapeTimer, 2);
    ProgramLongVar(iActiveAltShapeTimer, 2);

    ProgramLongVar(iActivateSound, MINEACTIVESOUND);
    ProgramLongVar(iActivateVolume, 1);
    ProgramLongVar(iBlastSound, MINEBLOWSOUND);
    ProgramLongVar(iHitSound, HITSOUNDV1);
    ProgramFixedVar(iActivateEnergy, FIX3(200));

    ProgramMessage(iStartMsg, iStartMsg);
    ProgramLongVar(iStopMsg, 0);
    ProgramLongVar(iStatus, false);

    ProgramLongVar(iBoom, 0);
}

CAbstractActor *CMineActor::EndScript() {
    short altShapeId;
    short primeShapeId;

    CGlowActors::EndScript();

    watchBits = ReadLongVar(iWatchMask);

    altShapeId = ReadLongVar(iAltShape);
    primeShapeId = ReadLongVar(iShape);

    RegisterReceiver(&startMsg, ReadLongVar(iStartMsg));
    RegisterReceiver(&stopMsg, ReadLongVar(iStopMsg));
    enabled = ReadLongVar(iStatus);

    activateSound = ReadLongVar(iActivateSound);
    activateVolume = ReadFixedVar(iActivateVolume);
    gHub->PreLoadSample(activateSound);
    RegisterReceiver(&activator, ReadLongVar(iBoom));

    radius = ReadFixedVar(iRange);

    shapeTimes[0][0] = ReadLongVar(iIdleShapeTimer);
    shapeTimes[0][1] = ReadLongVar(iIdleAltShapeTimer);
    shapeTimes[1][0] = ReadLongVar(iActiveShapeTimer);
    shapeTimes[1][1] = ReadLongVar(iActiveAltShapeTimer);

    phase = ReadLongVar(iPhase);
    lookTime = ReadLongVar(iFrequency);
    fuseTime = ReadLongVar(iActiveTimer);
    activateEnergy = ReadFixedVar(iActivateEnergy);

    partCount = 2;

    LoadPartWithColors(0, primeShapeId);
    LoadPartWithColors(1, altShapeId);

    InitialRotatePartY(partList[0], heading);
    TranslatePart(partList[0], location[0], location[1], location[2]);
    partList[0]->MoveDone();

    InitialRotatePartY(partList[1], heading);
    TranslatePart(partList[1], location[0], location[1], location[2]);
    partList[1]->MoveDone();
    partList[1]->isTransparent = true;

    LinkPartSpheres();

    isActive = kIsActive;
    maskBits |= kSolidBit;
    activated = false;

    lookNextTime = 0;
    shapeNextTime = shapeTimes[0][0];
    curShape = 0;

    sliverCounts[kSmallSliver] = 8;
    sliverCounts[kMediumSliver] = 4;

    return this;
}

void CMineActor::Activate() {
    if (enabled) {
        activated = true;
        shapeNextTime = phase;

        lookNextTime = phase + fuseTime;
        if (fuseTime) {
            itsSoundLink = gHub->GetSoundLink();
            if (itsSoundLink) {
                CBasicSound *theSound;

                PlaceSoundLink(itsSoundLink, location);
                theSound = gHub->GetSoundSampler(hubRate, activateSound);
                theSound->SetVolume(activateVolume);
                theSound->SetSoundLink(itsSoundLink);
                theSound->Start();
            }
        }

        if (shapeTimes[activated][1]) {
            curShape = 0;
        }
    }
}

void CMineActor::Dispose() {
    itsGame->RemoveReceiver(&activator);

    if (itsSoundLink) {
        gHub->ReleaseLinkAndKillSounds(itsSoundLink);
        itsSoundLink = NULL;
    }

    CGlowActors::Dispose();
}

void CMineActor::FrameAction() {
    if (isActive & kHasMessage) {
        if (stopMsg.triggerCount > startMsg.triggerCount)
            enabled = false;
        else if (stopMsg.triggerCount < startMsg.triggerCount)
            enabled = true;

        stopMsg.triggerCount = 0;
        startMsg.triggerCount = 0;

        if (activator.triggerCount) {
            activator.triggerCount = 0;
            Activate();
        }

        isActive &= ~kHasMessage;
    }

    phase++;

    if (phase >= lookNextTime) {
        if (activated) {
            WasDestroyed();
            itsGame->scoreReason = ksiMineBlast;
            SecondaryDamage(teamColor, -1);
            return;
        } else {
            lookNextTime = phase + lookTime;

            if (lookTime) {
                CAbstractActor *theActor;
                CSmartPart *thePart;

                BuildPartProximityList(location, radius, watchBits);
                theActor = itsGame->actorList;

                for (thePart = proximityList.p; thePart; thePart = (CSmartPart *)thePart->nextTemp) {
                    theActor = thePart->theOwner;
                    if (radius + thePart->enclosureRadius >
                        FDistanceEstimate(location[0] - thePart->sphereGlobCenter[0],
                            location[1] - thePart->sphereGlobCenter[1],
                            location[2] - thePart->sphereGlobCenter[2])) {
                        Activate();
                        break;
                    }
                }
            }
        }
    }

    if (phase >= shapeNextTime) {
        long newTime;

        newTime = shapeTimes[activated][1 - curShape];
        if (newTime) {
            partList[curShape]->isTransparent = true;
            curShape = 1 - curShape;
            shapeNextTime = phase + newTime;
            partList[curShape]->isTransparent = false;
        }
    }

    CGlowActors::FrameAction();
}

void CMineActor::WasHit(RayHitRecord *theHit, Fixed hitEnergy) {
    CGlowActors::WasHit(theHit, hitEnergy);

    if (!activated && shields > 0 && hitEnergy > activateEnergy) {
        Activate();
    }
}
