/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CSoundHub.c
    Created: Sunday, December 25, 1994, 11:06
    Modified: Monday, August 19, 1996, 21:54
*/

#include "CSoundHub.h"

#include "AssetManager.h"
#include "CBasicSound.h"
#include "CHuffProcessor.h"
#include "CRateSound.h"
#include "CSoundMixer.h"
#include "Memory.h"

#include <SDL2/SDL.h>

void CSoundHubImpl::CreateSound(short kind) {
    CBasicSound *aSound;

    switch (kind) {
        case hubBasic:
            aSound = new CBasicSound;
            break;
        case hubRate:
            aSound = new CRateSound;
            break;
        default:
            aSound = NULL;
    }

    if (aSound) {
        aSound->nextSound = soundList[kind];
        aSound->itsHub = this;
        aSound->hubId = kind;
        soundList[kind] = aSound;
    }
}

void CSoundHubImpl::Restock(CBasicSound *aSound) {
    aSound->nextSound = soundList[aSound->hubId];
    soundList[aSound->hubId] = aSound;

    aSound->itsSamples = nullptr;
}

CBasicSound *CSoundHubImpl::Acquire(short kind) {
    CBasicSound *aSound;

    if (soundList[kind] == NULL)
        CreateSound(kind);

    aSound = soundList[kind];
    if (aSound) {
        soundList[kind] = aSound->nextSound;
        aSound->Reset();
    }

    return aSound;
}

void CSoundHubImpl::ISoundHub(short numOfEachKind, short initialLinks) {
    short i, j;

    itsMixer = NULL;

    for (j = 0; j < hubSoundKinds; j++) {
        soundList[j] = NULL;

        for (i = 0; i < numOfEachKind; i++) {
            CreateSound(j);
        }
    }

    muteFlag = false;

    itsCompressor = new CHuffProcessor;
    itsCompressor->Open();

    soundLinkStorage = NULL;
    firstFreeLink = NULL;

    if (initialLinks > 0)
        CreateSoundLinks(initialLinks);
}

void CSoundHubImpl::AttachMixer(CSoundMixer *aMixer) {
    itsMixer = aMixer;
    muteFlag = itsMixer->maxChannels == 0;
}

void CSoundHubImpl::Dispose() {
    CBasicSound *aSound, *nextSound;
    short i;

    if (itsMixer) {
        itsMixer->Dispose();
    }

    for (i = 0; i < hubSoundKinds; i++) {
        aSound = soundList[i];
        while (aSound) {
            nextSound = aSound->nextSound;
            aSound->Dispose();
            aSound = nextSound;
        }
    }

    itsCompressor->Dispose();
    DisposeSoundLinks();
    CDirectObject::Dispose();
}

CBasicSound *CSoundHubImpl::GetSoundSampler(short kind, short resId) {
    CBasicSound *aSound;
    std::shared_ptr<OggFile> theSamples;

    aSound = Acquire(kind);
    if (aSound) {
        aSound->itsMixer = itsMixer;

        if (muteFlag) {
            theSamples = nullptr;
        } else {
            auto maybeOgg = AssetManager::GetOgg(resId);
            theSamples = (maybeOgg)
                ? *maybeOgg
                : nullptr;
        }
        aSound->UseSamples(theSamples);
        aSound->SetRate(FIX1);
    }

    return aSound;
}

void CSoundHubImpl::CreateSoundLinks(short n) {
    Ptr newStorage;
    SoundLink *linkP;

    newStorage = NewPtr(sizeof(Ptr) + sizeof(SoundLink) * n);
    if (newStorage) {
        *(Ptr *)newStorage = soundLinkStorage;
        soundLinkStorage = newStorage;
        linkP = (SoundLink *)(newStorage + sizeof(Ptr));

        while (n--) {
            *(SoundLink **)linkP = firstFreeLink;
            firstFreeLink = linkP;
            linkP++;
        }
    }
}

void CSoundHubImpl::DisposeSoundLinks() {
    while (soundLinkStorage) {
        Ptr temp;

        temp = *(Ptr *)soundLinkStorage;
        DisposePtr(soundLinkStorage);
        soundLinkStorage = temp;
    }
}

int CSoundHubImpl::ReadTime() {
    return itsMixer->ReadTime();
}

void CSoundHubImpl::SetMixerLink(SoundLink *newLink) {
    SoundLink *oldLink;

    oldLink = itsMixer->motionLink;

    if (newLink != oldLink) {
        if (newLink)
            newLink->refCount++;

        itsMixer->altLink = newLink;
        itsMixer->useAltLink = true;

        itsMixer->motionLink = newLink;
        if (oldLink)
            ReleaseLink(oldLink);
        itsMixer->useAltLink = false;
        itsMixer->motionHub = this;
    }
}

SoundLink *CSoundHubImpl::UpdateRightVector(Fixed *right) {
    itsMixer->UpdateRightVector(right);

    return itsMixer->motionLink;
}

SoundLink *CSoundHubImpl::GetSoundLink() {
    SoundLink *theLink;

    theLink = firstFreeLink;
    if (theLink == 0) {
        CreateSoundLinks(EXTRASOUNDLINKCOUNT);
        theLink = firstFreeLink;
    }

    if (theLink) {
        firstFreeLink = *(SoundLink **)theLink;
        theLink->refCount = 1;
        theLink->meta = metaNoData;
    }

    return theLink;
}

void CSoundHubImpl::ReleaseLink(SoundLink *linkPtr) {
    linkPtr->refCount--;
    if (linkPtr->refCount == 0) {
        *(SoundLink **)linkPtr = firstFreeLink;
        firstFreeLink = linkPtr;
    }
}

void CSoundHubImpl::ReleaseLinkAndKillSounds(SoundLink *linkPtr) {
    linkPtr->refCount--;
    if (linkPtr->refCount > 0) {
        linkPtr->meta = metaKillNow;
    } else {
        *(SoundLink **)linkPtr = firstFreeLink;
        firstFreeLink = linkPtr;
    }
}

void CSoundHubImpl::HouseKeep() {
    itsMixer->HouseKeep();
}
void CSoundHubImpl::MuteFlag(Boolean mute) {
    muteFlag = mute;

}
Fixed* CSoundHubImpl::EarLocation() {
    return itsMixer->motionLink->nLoc;
}
void CSoundHubImpl::MixerDispose() {
    if (itsMixer) {
        itsMixer->Dispose();
    }
}

Fixed CSoundHubImpl::DistanceToLevelOne() {
    return itsMixer->distanceToLevelOne;
}
void CSoundHubImpl::HushFlag(bool flag) {
    itsMixer->hushFlag = flag;
}

bool CSoundHubImpl::Stereo() {
    return itsMixer->stereo;
}

void UpdateSoundLink(SoundLink *theLink, Fixed *s, Fixed *v, unsigned int t) {
    theLink->meta = metaNoData;

    theLink->nLoc[0] = *s++;
    theLink->nLoc[1] = *s++;
    theLink->nLoc[2] = *s++;

    theLink->nSpeed[0] = *v++;
    theLink->nSpeed[1] = *v++;
    theLink->nSpeed[2] = *v++;

    theLink->t = t;

    theLink->meta = metaNewData;
}

void ZeroSoundLink(SoundLink *theLink) {
    theLink->meta = metaNoData;

    theLink->nLoc[0] = 0;
    theLink->nLoc[1] = 0;
    theLink->nLoc[2] = 0;

    theLink->nSpeed[0] = 0;
    theLink->nSpeed[1] = 0;
    theLink->nSpeed[2] = 0;

    theLink->t = 0;

    theLink->meta = metaNewData;
}

void PlaceSoundLink(SoundLink *theLink, Fixed *s) {
    theLink->meta = metaNoData;

    theLink->nLoc[0] = *s++;
    theLink->nLoc[1] = *s++;
    theLink->nLoc[2] = *s++;

    theLink->nSpeed[0] = 0;
    theLink->nSpeed[1] = 0;
    theLink->nSpeed[2] = 0;

    theLink->t = 0;

    theLink->meta = metaNewData;
}
