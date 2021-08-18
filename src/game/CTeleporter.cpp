/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CTeleporter.c
    Created: Monday, July 3, 1995, 01:19
    Modified: Sunday, September 1, 1996, 20:00
*/

#include "CTeleporter.h"

#include "CAbstractPlayer.h"
#include "CPlayerManager.h"
#include "CSmartPart.h"

#define TELEPORTAREA FIX3(3000)
#define ACTIVETELEPORTAREA FIX3(350)
#define FIELDSTRENGTH FIX3(100)
#define TELEPORTERMIDDLE FIX3(1500)
#define RETRANSMITFRAMES 60

void CTeleporter::BeginScript() {
    ProgramLongVar(iGroup, 0);
    ProgramReference(iDestGroup, iGroup);
    ProgramLongVar(iSound, kDefaultTeleportSound);
    ProgramLongVar(iShape, kTeleportBSP);
    ProgramLongVar(iSpeed, -15);
    ProgramLongVar(iVolume, 10);
    ProgramLongVar(iSpinFlag, 1);
    ProgramLongVar(iFragmentFlag, 1);
    ProgramLongVar(iMask, -1);
    ProgramLongVar(iWin, -1);
    ProgramLongVar(iIsAmbient, false);

    ProgramMessage(iStartMsg, iStartMsg);
    ProgramLongVar(iStopMsg, 0);
    ProgramLongVar(iStatus, false);
    ProgramLongVar(iOutVar, 0);
    ProgramLongVar(iInVar, 0);

    ProgramFixedVar(iRange, ACTIVETELEPORTAREA);
    ProgramLongVar(iDeadRange, 0);
    ProgramLongVar(iShowAlways, true);

    hitScore = -10000000;

    CPlacedActors::BeginScript();
}
CAbstractActor *CTeleporter::EndScript() {
    short shapeRes;
    CPlacedActors::EndScript();

    isActive = kIsActive;

    shapeRes = ReadLongVar(iShape);

    if (shapeRes) {
        partCount = 1;
        LoadPartWithColors(0, shapeRes);
        partList[0]->Reset();
        if (ReadLongVar(iIsAmbient) > 0)
            partList[0]->userFlags |= CBSPUserFlags::kIsAmbient;
        InitialRotatePartY(partList[0], heading);
        TranslatePart(partList[0], location[0], location[1], location[2]);
        partList[0]->MoveDone();
    }

    useCount = 0;
    goTimer = 0;
    noPullTimer = 0;
    pullCounter = 0;
    maskBits |= kTeleportBit;

    RegisterReceiver(&startMsg, ReadLongVar(iStartMsg));
    RegisterReceiver(&stopMsg, ReadLongVar(iStopMsg));
    enabled = ReadLongVar(iStatus);

    transportGroup = ReadLongVar(iGroup);
    destGroup = ReadLongVar(iDestGroup);
    rotationSpeed = FpsCoefficient2(FDegToOne(ReadFixedVar(iSpeed)));

    soundId = ReadLongVar(iSound);
    volume = ReadFixedVar(iVolume);
    gHub->PreLoadSample(soundId);

    options = ReadLongVar(iSpinFlag) ? kSpinOption : 0;
    options |= ReadLongVar(iFragmentFlag) ? kFragmentOption : 0;
    options |= ReadLongVar(iShowAlways) ? kShowAlwaysOption : 0;
    watchTeams = ReadLongVar(iMask);
    winScore = ReadLongVar(iWin);

    activeRange = ReadFixedVar(iRange);
    deadRange = ReadFixedVar(iDeadRange);

    didSendMsg = ReadLongVar(iOutVar);
    didReceiveMsg = ReadLongVar(iInVar);

    return this;
}

void CTeleporter::FrameAction() {
    ActorLocator *loc;
    short i;

    CPlacedActors::FrameAction();

    if (isActive & kHasMessage) {
        if (stopMsg.triggerCount > startMsg.triggerCount)
            enabled = false;
        else if (stopMsg.triggerCount < startMsg.triggerCount)
            enabled = true;

        stopMsg.triggerCount = 0;
        startMsg.triggerCount = 0;

        isActive &= ~kHasMessage;
    }

    if (enabled) {
        if (goTimer) {
            goTimer--;
        } else {
            CAbstractPlayer *thePlayer;

            thePlayer = itsGame->playerList;

            if (noPullTimer)
                noPullTimer--;

            while (thePlayer) {
                if ((thePlayer->teamMask & watchTeams) && (thePlayer->searchCount != searchCount) &&
                    !thePlayer->isInLimbo && thePlayer->itsGame->scores[thePlayer->itsManager->Slot()] >= hitScore) {
                    Vector delta;
                    Fixed distance;

                    delta[0] = thePlayer->location[0] - location[0];
                    delta[1] = thePlayer->location[1] - location[1];
                    delta[2] = thePlayer->location[2] - location[2];

                    distance = FDistanceEstimate(delta[0], delta[1], delta[2]);

                    if (distance < activeRange && distance >= deadRange) {
                        if (winScore < 0) {
                            TeleportPlayer(thePlayer);
                        } else {
                            itsGame->FlagMessage(didSendMsg);
                            thePlayer->Win(winScore, this);
                        }
                    } else if (noPullTimer == 0 && distance < TELEPORTAREA) {
                        delta[0] = FMul(delta[0], -FIELDSTRENGTH);
                        delta[1] = FMul(delta[1], -FIELDSTRENGTH);
                        delta[2] = FMul(delta[2], -FIELDSTRENGTH);
                        thePlayer->Accelerate(delta);
                        pullCounter++;
                        if (pullCounter >= 32) {
                            noPullTimer = 16;
                            pullCounter = 0;
                        }
                    }
                }

                thePlayer = thePlayer->nextPlayer;
            }
        }
    }

    if (partList[0]) {
        if (options & kShowAlwaysOption) {
            partList[0]->isTransparent = !enabled;
        }

        if (rotationSpeed && partList[0]->isTransparent == false) {
            heading += rotationSpeed;
            partList[0]->Reset();
            InitialRotatePartY(partList[0], heading);
            TranslatePart(partList[0], location[0], location[1], location[2]);
            partList[0]->MoveDone();
        }
    }
}

void CTeleporter::TeleportPlayer(CAbstractPlayer *thePlayer) {
    CTeleporter *theActor;
    CTeleporter *thePort;
    unsigned long maxUse = 0xffffFFFF;

    theActor = (CTeleporter *)itsGame->actorList;
    thePort = NULL;

    while (theActor) {
        if (theActor->maskBits & kTeleportBit) {
            if (theActor->transportGroup == destGroup && theActor->useCount < maxUse && theActor != this) {
                maxUse = theActor->useCount;
                thePort = theActor;
            }
        }
        theActor = (CTeleporter *)theActor->nextActor;
    }

    if (thePort) {
        if (thePort->ReceivePlayer(thePlayer)) {
            useCount++;
            itsGame->FlagMessage(didSendMsg);
        }
    }
}

Boolean CTeleporter::ReceivePlayer(CAbstractPlayer *thePlayer) {
    Boolean didMove;

    didMove = thePlayer->TryTransport(location, soundId, volume, options);
    if (didMove) {
        goTimer = FpsFramesPerClassic(RETRANSMITFRAMES);
        itsGame->FlagMessage(didReceiveMsg);
    }
    useCount++;

    return didMove;
}
