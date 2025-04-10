/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CTeleporter.c
    Created: Monday, July 3, 1995, 01:19
    Modified: Sunday, September 1, 1996, 20:00
*/
// #define ENABLE_FPS_DEBUG  // uncomment if you want to see FPS_DEBUG output for this file

#include "CTeleporter.h"

#include "CAbstractPlayer.h"
#include "CPlayerManager.h"
#include "CSmartPart.h"

#define TELEPORTAREA FIX3(3000)
#define ACTIVETELEPORTAREA FIX3(350)
#define FIELDSTRENGTH FIX3(100)
#define TELEPORTERMIDDLE FIX3(1500)
#define RETRANSMITFRAMES 60
#define SPEEDLIMIT 2.25    // ratio of speed/activeRange ... a little more than the diameter of the teleport area


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

    // Preload sounds.
    auto _ = AssetManager::GetOgg(soundId);

    options = ReadLongVar(iSpinFlag) ? kSpinOption : 0;
    options |= ReadLongVar(iFragmentFlag) ? kFragmentOption : 0;
    options |= ReadLongVar(iShowAlways) ? kShowAlwaysOption : 0;
    watchTeams = ReadLongVar(iMask);
    winScore = ReadLongVar(iWin);

    activeRange = ReadFixedVar(iRange);
    deadRange = ReadFixedVar(iDeadRange);

    didSendMsg = ReadLongVar(iOutVar);
    didReceiveMsg = ReadLongVar(iInVar);

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

    return this;
}

void CTeleporter::FrameAction() {
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
                    Fixed distance, speed;

                    delta[0] = thePlayer->location[0] - location[0];
                    delta[1] = thePlayer->location[1] - location[1];
                    delta[2] = thePlayer->location[2] - location[2];

                    distance = FDistanceEstimate(delta);
                    speed = FDistanceEstimate(thePlayer->speed);
                    if (distance < TELEPORTAREA) {
                        FPS_DEBUG("\nframeNumber = " << itsGame->frameNumber << "\n");
                        FPS_DEBUG("distance = " << distance << ", activeRange = " << activeRange << ", deadRange = " << deadRange << ", delta = " << FormatVector(delta, 3) << ", speed = " << speed << ", speed / activeRange = " << speed / double(activeRange) << "\n");
                    }

                    if (distance < activeRange && distance >= deadRange && speed < SPEEDLIMIT*activeRange) {
                        if (winScore < 0) {
                            TeleportPlayer(thePlayer);
                        } else {
                            itsGame->FlagMessage(didSendMsg);
                            thePlayer->Win(winScore, this);
                        }
                    } else if (noPullTimer == 0 && distance < TELEPORTAREA) {
                        Fixed attraction = -FpsCoefficient2(FIELDSTRENGTH);
                        if (pullCounter == 0) {
                            FPS_DEBUG("attraction = " << attraction);
                            attraction -= FpsOffset(FIELDSTRENGTH);
                            FPS_DEBUG(", attraction with initial offset = " << attraction << "\n");
                        }
                        delta[0] = FMul(delta[0], attraction);
                        delta[1] = FMul(delta[1], attraction);
                        delta[2] = FMul(delta[2], attraction);
                        thePlayer->Accelerate(delta);
                        FPS_DEBUG("pullCounter = " << pullCounter << ", attraction = " << FormatVector(delta, 3) << ", new player speed = " << FormatVector(thePlayer->speed, 3) << "\n");
                        pullCounter++;
                        if (pullCounter >= FpsFramesPerClassic(32)) {
                            noPullTimer = FpsFramesPerClassic(16);
                            pullCounter = 0;
                        }
                    } else if (distance > TELEPORTAREA) {
                        pullCounter = noPullTimer = 0;
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
    CAbstractActor *theActor;
    CTeleporter *thePort;
    unsigned long maxUse = 0xffffFFFF;

    theActor = (CAbstractActor *)itsGame->actorList;
    thePort = NULL;

    while (theActor) {
        if (theActor->maskBits & kTeleportBit) {
            CTeleporter *teleActor = (CTeleporter *)theActor;
            if (teleActor->transportGroup == destGroup && teleActor->useCount < maxUse && theActor != this) {
                maxUse = teleActor->useCount;
                thePort = teleActor;
            }
        }
        theActor = (CAbstractActor *)theActor->nextActor;
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
