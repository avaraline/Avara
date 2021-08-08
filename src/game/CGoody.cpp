/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CGoody.c
    Created: Friday, February 23, 1996, 13:53
    Modified: Tuesday, August 27, 1996, 04:16
*/

#include "CGoody.h"

#include "CAbstractPlayer.h"
#include "CBSPWorld.h"
#include "CSmartPart.h"
#include "CWallActor.h"
#include "GoodyRecord.h"
#include "Preferences.h"

#define kGoodySound 250


extern CWallActor *lastWallActor;

void CGoody::BeginScript() {
    hitScore = 0;

    CPlacedActors::BeginScript();

    ProgramLongVar(iShape, 240);
    ProgramLongVar(iAltShape, 0);

    ProgramLongVar(iSpeed, 0);

    ProgramMessage(iStartMsg, iStartMsg);
    ProgramLongVar(iStopMsg, 0);
    ProgramVariable(iRoll, 0);

    ProgramLongVar(iGrenades, 0);
    ProgramLongVar(iMissiles, 0);
    ProgramLongVar(iBoosters, 0);
    ProgramLongVar(iLives, 0);

    ProgramLongVar(iBoostTime, 0);

    ProgramLongVar(iOutVar, 0);

    ProgramLongVar(iSound, kGoodySound);
    ProgramFixedVar(iVolume, FIX(1));
    ProgramLongVar(iOpenSound, 0);
    ProgramLongVar(iCloseSound, 0);

    ProgramLongVar(iFrequency, 1);

    ProgramLongVar(iText, true);
}

CAbstractActor *CGoody::EndScript() {
    if (CPlacedActors::EndScript()) {
        short shapeId;
        short altShapeId;

        enabled = false;

        RegisterReceiver(&startMsg, ReadLongVar(iStartMsg));
        RegisterReceiver(&stopMsg, ReadLongVar(iStopMsg));

        shapeId = ReadLongVar(iShape);
        altShapeId = ReadLongVar(iAltShape);

        partCount = 1;
        LoadPartWithColors(0, shapeId);
        partList[0]->RotateZ(partRoll = ReadFixedVar(iRoll));
        partList[0]->RotateOneY(heading);
        TranslatePart(partList[0], location[0], location[1], location[2]);
        partList[0]->MoveDone();

        if (altShapeId) {
            partCount = 2;
            LoadPart(1, shapeId);
            partList[1]->CopyTransform(&partList[0]->itsTransform);
            partList[1]->MoveDone();
        }

        rotationSpeed = FpsCoefficient2(FDegToOne(ReadFixedVar(iSpeed)));
        grenades = ReadLongVar(iGrenades);
        missiles = ReadLongVar(iMissiles);
        boosters = ReadLongVar(iBoosters);
        lives = ReadLongVar(iLives);
        boostTime = ReadLongVar(iBoostTime);
        outMsg = ReadLongVar(iOutVar);

        if(itsGame->itsApp->Boolean(kIgnoreCustomGoodySound)) {
            soundId = kGoodySound;
        }
        else {
            soundId = ReadLongVar(iSound);
        }

        openSoundId = ReadLongVar(iOpenSound);
        closeSoundId = ReadLongVar(iCloseSound);
        volume = ReadFixedVar(iVolume);

        gHub->PreLoadSample(soundId);
        gHub->PreLoadSample(openSoundId);
        gHub->PreLoadSample(closeSoundId);

        isActive = kIsInactive;
        frequency = ReadLongVar(iFrequency);
        sleepTimer = frequency;

        partList[0]->isTransparent = !enabled;
        if (partList[1])
            partList[1]->isTransparent = enabled;

        return this;
    } else {
        return NULL;
    }
}

void CGoody::FrameAction() {
    CSmartPart *thePart;

    if (isActive & kHasMessage) {
        isActive &= ~kHasMessage;

        if (stopMsg.triggerCount) {
            stopMsg.triggerCount = 0;
            DoSound(closeSoundId, location, volume, FIX(1));
            enabled = false;
        }

        if (startMsg.triggerCount) {
            startMsg.triggerCount = 0;
            DoSound(openSoundId, location, volume, FIX(1));
            enabled = true;
        }
    }

    CPlacedActors::FrameAction();

    if (enabled) {
        BuildPartProximityList(partList[0]->sphereGlobCenter, partList[0]->enclosureRadius, kPlayerBit);

        searchCount = ++itsGame->searchCount;
        for (thePart = proximityList.p; thePart; thePart = (CSmartPart *)thePart->nextTemp) {
            if (thePart->CollisionTest(partList[0])) {
                CAbstractPlayer *theActor;
                GoodyRecord gr;

                gr.grenades = grenades;
                gr.missiles = missiles;
                gr.boosters = boosters;
                gr.lives = lives;
                gr.shield = shields;
                gr.power = blastPower;
                gr.boostTime = boostTime;

                theActor = (CAbstractPlayer *)thePart->theOwner;
                theActor->TakeGoody(&gr);
                DoSound(soundId, location, volume, FIX(1));

                itsGame->FlagMessage(outMsg);
                enabled = false;

                itsGame->scoreReason = ksiGoodyBonus;
                itsGame->Score(
                    theActor->teamColor, theActor->GetActorScoringId(), hitScore, 0, teamColor, GetActorScoringId());
                break;
            }
        }
    }

    if (rotationSpeed) {
        heading += rotationSpeed;
        partList[0]->Reset();
        InitialRotatePartZ(partList[0], partRoll);
        partList[0]->RotateOneY(heading);
        TranslatePart(partList[0], location[0], location[1], location[2]);
        partList[0]->MoveDone();

        if (partList[1]) {
            partList[1]->CopyTransform(&partList[0]->itsTransform);
            partList[1]->MoveDone();
        }
    }

    partList[0]->isTransparent = !enabled;
    if (partList[1])
        partList[1]->isTransparent = enabled;

    if (enabled)
        sleepTimer = frequency;
}
