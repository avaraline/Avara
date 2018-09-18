/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CSoundActor.c
    Created: Thursday, November 30, 1995, 23:29
    Modified: Sunday, September 1, 1996, 18:56
*/

#include "CSoundActor.h"

#include "CRateSound.h"
#include "CSoundMixer.h"

void CSoundActor::BeginScript() {
    CPlacedActors::BeginScript();

    ProgramLongVar(iSound, 0);
    ProgramLongVar(iVolume, 10);
    ProgramLongVar(iHeight, 10);
    ProgramLongVar(iRate, 1);
    ProgramLongVar(iLoopCount, 0);

    ProgramLongVar(iPhase, 0);
    ProgramReference(iVolume0, iVolume);
    ProgramReference(iVolume1, iVolume);

    ProgramLongVar(iRestartFlag, true);
    ProgramLongVar(iIsAmbient, true);
    ProgramLongVar(iIsMusic, false);
    ProgramLongVar(iIsPlaced, true);

    ProgramMessage(iStartMsg, iStartMsg);
    ProgramLongVar(iStopMsg, 0);
    ProgramLongVar(iKillMsg, 0);
}

CAbstractActor *CSoundActor::EndScript() {
    if (CPlacedActors::EndScript()) {
        RegisterReceiver(&startMsg, ReadLongVar(iStartMsg));
        RegisterReceiver(&stopMsg, ReadLongVar(iStopMsg));
        RegisterReceiver(&killMsg, ReadLongVar(iKillMsg));

        soundId = ReadLongVar(iSound);
        isPlaced = ReadLongVar(iIsPlaced);
        isMusic = ReadLongVar(iIsMusic);
        isAmbient = ReadLongVar(iIsAmbient);
        restart = ReadLongVar(iRestartFlag);
        loopCount = ReadLongVar(iLoopCount);
        rate = ReadFixedVar(iRate);

        if (!isAmbient || (itsGame->soundSwitches & kAmbientSoundToggle))
            gHub->PreLoadSample(soundId);

        if (isPlaced) {
            itsSoundLink = gHub->GetSoundLink();
            PlaceSoundLink(itsSoundLink, location);
            volumes[0] = ReadFixedVar(iVolume);
        } else {
            volumes[0] = ReadFixedVar(iVolume0);
            volumes[1] = ReadFixedVar(iVolume1);
            phase = ReadLongVar(iPhase);
        }

        controlLink = gHub->GetSoundLink();

        needsResume = false;
        isActive = false;
        theSampler = NULL;

        return this;
    } else {
        return NULL;
    }
}

void CSoundActor::GetSound() {
    theSampler = (CRateSound *)gHub->GetSoundSampler(hubRate, soundId);
    theSampler->SetRate(rate);

    if (loopCount)
        theSampler->SetLoopCount(loopCount);

    if (isPlaced) {
        theSampler->SetSoundLink(itsSoundLink);
        theSampler->SetVolume(volumes[0]);
    } else {
        if (gHub->Stereo()) {
            theSampler->SetDirectVolumes(volumes[0], volumes[1]);
            theSampler->currentCount[0].i -= phase;
            theSampler->currentCount[1].i += phase;
        } else {
            theSampler->SetVolume((volumes[0] + volumes[1]) >> 1);
        }
    }

    theSampler->SetControlLink(controlLink);
}

void CSoundActor::ResumeLevel() {
    CPlacedActors::ResumeLevel();

    if (needsResume && ShouldPlay()) {
        needsResume = false;
        GetSound();
        theSampler->currentCount[0] = savedPosition[0];
        theSampler->currentCount[1] = savedPosition[1];
        theSampler->phaseBalance = savedBalance;
        theSampler->loopCount[0] = savedLoopCount[0];
        theSampler->loopCount[1] = savedLoopCount[1];
        theSampler->distanceDelay = false;
        theSampler->Start();
    }
}

void CSoundActor::PauseLevel() {
    CPlacedActors::PauseLevel();

    controlLink->meta = metaSuspend;

    if (theSampler && controlLink->refCount > 1) {
        needsResume = true;
        savedPosition[0] = theSampler->currentCount[0];
        savedPosition[1] = theSampler->currentCount[1];
        savedBalance = theSampler->phaseBalance;
        savedLoopCount[0] = theSampler->loopCount[0];
        savedLoopCount[1] = theSampler->loopCount[1];

        theSampler = NULL;
    }

    controlLink->meta = metaNoData;
}

void CSoundActor::FrameAction() {
    CPlacedActors::FrameAction();

    if (isActive & kHasMessage) {
        isActive &= ~kHasMessage;

        if (startMsg.triggerCount) {
            if (ShouldPlay()) {
                GetSound();
                theSampler->Start();
            }

            if (!restart) {
                itsGame->RemoveReceiver(&startMsg);
            }
        }

        if (killMsg.triggerCount) {
            killMsg.triggerCount = 0;
            itsGame->RemoveReceiver(&startMsg);
            stopMsg.triggerCount = 1;
        }

        if (stopMsg.triggerCount) {
            stopMsg.triggerCount = 0;
            gHub->ReleaseLinkAndKillSounds(controlLink);
            controlLink = gHub->GetSoundLink();
        }
    }
}

void CSoundActor::Dispose() {
    if (controlLink) {
        gHub->ReleaseLink(controlLink);
        controlLink = NULL;
    }

    CPlacedActors::Dispose();
}

Boolean CSoundActor::ShouldPlay() {
    if (isAmbient && !(itsGame->soundSwitches & kAmbientSoundToggle))
        return false;

    if (isMusic && !(itsGame->soundSwitches & kMusicToggle))
        return false;

    return true;
}
