/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CTextActor.c
    Created: Saturday, January 20, 1996, 03:34
    Modified: Sunday, September 8, 1996, 22:57
*/

#include "CTextActor.h"

#include "CSmartPart.h"
//#include "CInfoPanel.h"
#include "CAbstractPlayer.h"
#include "Messages.h"

void CTextActor::BeginScript() {
    CAbstractActor::BeginScript();

    ProgramLongVar(iShowEveryone, false);
    ProgramLongVar(iInVar, 0);
    ProgramReference(iText, iText);

    ProgramMessage(iStartMsg, iStartMsg);
    ProgramLongVar(iStopMsg, 0);
    ProgramLongVar(iStatus, false);

    ProgramLongVar(iRestartFlag, 75);
    ProgramLongVar(iFrequency, 5);
    ProgramLongVar(iTimer, 0);

    ProgramLongVar(iAlignment, static_cast<short>(MsgAlignment::Center));

    ProgramLongVar(iSound, 150);
    ProgramFixedVar(iVolume, FIX3(250));
}

CAbstractActor *CTextActor::EndScript() {
    if (CAbstractActor::EndScript()) {
        MsgType inMsg;

        radius = GetLastOval(location);
        location[1] = ReadFixedVar(iHeight) + ReadFixedVar(iBaseHeight);

        RegisterReceiver(&in, ReadLongVar(iInVar));
        showEveryone = ReadLongVar(iShowEveryone);
        theMessage = ReadStringVar(iText);

        RegisterReceiver(&startMsg, ReadLongVar(iStartMsg));
        RegisterReceiver(&stopMsg, ReadLongVar(iStopMsg));
        enabled = ReadLongVar(iStatus);
        alignment = ReadLongVar(iAlignment);

        nextShowTime = 0;
        restartDelay = ReadLongVar(iRestartFlag);
        frequency = ReadLongVar(iFrequency);

        textTimer = ReadLongVar(iTimer);
        showTime = -1;

        soundId = ReadLongVar(iSound);
        soundVol = ReadFixedVar(iVolume);
        gHub->PreLoadSample(soundId);

        return this;
    } else {
        return NULL;
    }
}

void CTextActor::Dispose() {
    itsGame->RemoveReceiver(&in);

    CAbstractActor::Dispose();
}

void CTextActor::FrameAction() {
    Boolean wasTriggered = false;
    Boolean newFlag = false;
    CSmartPart *thePart;
    CAbstractPlayer *player;

    CAbstractActor::FrameAction();

    if (isActive & kHasMessage) {
        if (stopMsg.triggerCount) {
            stopMsg.triggerCount = 0;
            enabled = false;
        }

        if (startMsg.triggerCount) {
            startMsg.triggerCount = 0;
            enabled = true;
        }

        if (in.triggerCount) {
            in.triggerCount = 0;
            wasTriggered = enabled;
        }

        isActive &= ~kHasMessage;
    }

    sleepTimer = frequency;

    if ((wasTriggered || in.messageId == 0) && nextShowTime <= itsGame->frameNumber) {
        if (in.messageId) {
            areaFlag = false;
        }

        if (showEveryone) {
            if (in.messageId == 0) {
                BuildPartProximityList(location, radius, kPlayerBit);
            } else {
                newFlag = true;
            }
        } else {
            CSmartPart **partP;

            player = itsGame->GetLocalPlayer();
            proximityList.p = NULL;

            for (partP = player->partList; *partP; partP++) {
                if (!(*partP)->isTransparent) {
                    (*partP)->nextTemp = proximityList.p;
                    proximityList.p = *partP;
                }
            }
        }

        if (!newFlag) {
            for (thePart = proximityList.p; thePart; thePart = (CSmartPart *)thePart->nextTemp) {
                if (radius + thePart->enclosureRadius > FDistanceEstimate(location[0] - thePart->sphereGlobCenter[0],
                                                            location[1] - thePart->sphereGlobCenter[1],
                                                            location[2] - thePart->sphereGlobCenter[2])) {
                    newFlag = true;
                    break;
                }
            }
        }

        if (newFlag != areaFlag) {
            areaFlag = newFlag;
            if (newFlag) {
                isActive |= kIsActive;
                showTime = itsGame->FramesFromNow(textTimer);
            }
        }
    }

    if (showTime == itsGame->frameNumber) {
        CBasicSound *dullSound;

        dullSound = gHub->GetSoundSampler(hubRate, soundId);
        dullSound->SetVolume(soundVol);
        dullSound->Start();

        itsGame->itsApp->StringLine(theMessage, static_cast<MsgAlignment>(alignment));
        isActive &= ~kIsActive;
    }
}
