/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: CSoundHub.h
    Created: Sunday, December 25, 1994, 10:22
    Modified: Saturday, December 2, 1995, 01:48
*/

#pragma once
#include "CBasicSound.h"
#include "CDirectObject.h"
#include "SoundSystemDefines.h"

#define EXTRASOUNDLINKCOUNT 32

enum { hubBasic, hubRate, hubSoundKinds };

class CSoundMixer;
class CHuffProcessor;

class CSoundHub {
public:
    virtual Fixed* EarLocation() = 0;
    virtual Fixed DistanceToLevelOne() = 0;
    virtual void MuteFlag(Boolean) = 0;
    virtual void AttachMixer(CSoundMixer *aMixer) = 0;
    //virtual void CreateSound(short kind) = 0;

    virtual SampleHeaderHandle LoadSample(short resId) = 0;
    virtual SampleHeaderHandle PreLoadSample(short resId) = 0;
    virtual SampleHeaderHandle RequestSample(short resId) = 0;
    virtual void FreeUnusedSamples() = 0;

    virtual void FreeOldSamples() = 0;
    virtual void FlagOldSamples() = 0;

    virtual void Restock(CBasicSound *aSound) = 0;
    //virtual CBasicSound *Aquire(short kind) = 0;
    virtual CBasicSound *GetSoundSampler(short kind, short resId) = 0;

    //virtual void CreateSoundLinks(short n) = 0;
    //virtual void DisposeSoundLinks() = 0;
    virtual SoundLink *GetSoundLink() = 0;
    virtual void ReleaseLink(SoundLink *linkPtr) = 0;
    virtual void ReleaseLinkAndKillSounds(SoundLink *linkPtr) = 0;

    virtual void SetMixerLink(SoundLink *newLink) = 0;
    virtual SoundLink *UpdateRightVector(Fixed *right) = 0;
    virtual int ReadTime() = 0;
    virtual void HouseKeep() = 0;

    virtual void Dispose() = 0;
    virtual void MixerDispose() = 0;
    virtual void HushFlag(bool) = 0;
    virtual bool Stereo() = 0;
};
class CSoundHubImpl : public CDirectObject, public CSoundHub {
public:
    CHuffProcessor *itsCompressor;
    CSoundMixer *itsMixer;
    CBasicSound *soundList[hubSoundKinds];
    SampleHeaderHandle sampleList;

    Ptr soundLinkStorage;
    SoundLink *firstFreeLink;
    Boolean muteFlag;

    virtual void ISoundHub(short numOfEachKind, short initialLinks);
    virtual void AttachMixer(CSoundMixer *aMixer);
    virtual void CreateSound(short kind);

    virtual SampleHeaderHandle LoadSample(short resId);
    virtual SampleHeaderHandle PreLoadSample(short resId);
    virtual SampleHeaderHandle RequestSample(short resId);
    virtual void FreeUnusedSamples();

    virtual void FreeOldSamples();
    virtual void FlagOldSamples();

    virtual void Restock(CBasicSound *aSound);
    virtual CBasicSound *Aquire(short kind);
    virtual CBasicSound *GetSoundSampler(short kind, short resId);

    virtual void CreateSoundLinks(short n);
    virtual void DisposeSoundLinks();
    virtual SoundLink *GetSoundLink();
    virtual void ReleaseLink(SoundLink *linkPtr);
    virtual void ReleaseLinkAndKillSounds(SoundLink *linkPtr);

    virtual void SetMixerLink(SoundLink *newLink);
    virtual SoundLink *UpdateRightVector(Fixed *right);
    virtual int ReadTime();
    virtual void HouseKeep();

    virtual void MuteFlag(Boolean);
    virtual void Dispose();
    virtual Fixed* EarLocation();
    virtual Fixed DistanceToLevelOne();
    virtual void MixerDispose();
    virtual void HushFlag(bool);
    virtual bool Stereo();
};

void UpdateSoundLink(SoundLink *theLink, Fixed *s, Fixed *v, unsigned int t);
void ZeroSoundLink(SoundLink *theLink);
void PlaceSoundLink(SoundLink *theLink, Fixed *s);
