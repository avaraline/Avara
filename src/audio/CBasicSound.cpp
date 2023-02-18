/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CBasicSound.c
    Created: Friday, December 23, 1994, 10:57
    Modified: Tuesday, September 3, 1996, 21:27
*/

#include "CBasicSound.h"

#include "CSoundHub.h"
#include "CSoundMixer.h"

#include <assert.h>

void CBasicSound::Release() {
    if (itsHub) {
        if (motionLink)
            itsHub->ReleaseLink(motionLink);
        if (controlLink)
            itsHub->ReleaseLink(controlLink);

        itsHub->Restock(this);
    }
}

void CBasicSound::SetLoopCount(short count) {
    loopCount[0] = loopCount[1] = count;
}

void CBasicSound::SetRate(Fixed theRate) {
    //	Not applicable
}

UnsignedFixed CBasicSound::GetSampleRate() {
    return itsMixer->samplingRate;
}

void CBasicSound::SetSoundLength(Fixed seconds) {
    int numSamples;

    numSamples = FMul(seconds >> 8, GetSampleRate() >> 8);
    numSamples -= sampleLen;
    if (numSamples > 0 && loopEnd > loopStart) {
        loopCount[0] = numSamples / (loopEnd - loopStart);
        loopCount[1] = loopCount[0];
    } else {
        loopCount[0] = 0;
        loopCount[1] = 0;
    }
}

void CBasicSound::Start() {
    itsMixer->AddSound(this);
}

void CBasicSound::SetVolume(Fixed vol) {
    masterVolume = vol;
    volumes[0] = masterVolume >> (17 - VOLUMEBITS);
    volumes[1] = volumes[0];
}

void CBasicSound::SetDirectVolumes(Fixed vol0, Fixed vol1) {
    volumes[0] = vol0 >> (17 - VOLUMEBITS);
    volumes[1] = vol1 >> (17 - VOLUMEBITS);
}

void CBasicSound::StopSound() {
    stopNow = true;
}

void CBasicSound::CheckSoundLink() {
    SoundLink *aLink;
    Fixed *s, *d;
    Fixed timeConversion;
    Fixed t;

    aLink = motionLink;

    timeConversion = itsMixer->timeConversion;

    s = aLink->nSpeed;
    d = aLink->speed;
    *d++ = FMul(*s++, timeConversion);
    *d++ = FMul(*s++, timeConversion);
    *d++ = FMul(*s++, timeConversion);

    s = aLink->nLoc;
    d = aLink->loc;
    *d++ = *s++;
    *d++ = *s++;
    *d++ = *s++;

    s = aLink->speed;
    d = aLink->loc;
    t = aLink->t;

    *d++ -= *s++ * t;
    *d++ -= *s++ * t;
    *d++ -= *s++ * t;

    aLink->meta = metaNoData;
}

void CBasicSound::SetSoundLink(SoundLink *linkPtr) {
    if (linkPtr)
        linkPtr->refCount++;

    if (motionLink)
        itsHub->ReleaseLink(motionLink);

    motionLink = linkPtr;
}

void CBasicSound::SetControlLink(SoundLink *linkPtr) {
    if (linkPtr)
        linkPtr->refCount++;

    if (controlLink)
        itsHub->ReleaseLink(controlLink);

    controlLink = linkPtr;
}

void CBasicSound::Reset() {
    motionLink = NULL;
    controlLink = NULL;
    itsSamples = NULL;
    sampleLen = 0;
    sampleData = NULL;
    loopStart = 0;
    loopEnd = 0;
    loopCount[0] = loopCount[1] = 0;
    stopNow = false;
    volumeMax = VOLUMERANGE >> 1;
    distanceDelay = true;
}

void CBasicSound::UseSamplePtr(Sample *samples, int numSamples) {
    currentCount[0].i = 0;
    currentCount[0].f = 0;

    currentCount[1].i = 0;
    currentCount[1].f = 0;

    sampleLen = numSamples;
    sampleData = samples;
    loopStart = 0;
    loopEnd = 0;
    loopCount[0] = loopCount[1] = 0; //	Don't loop.
}

void CBasicSound::UseSamples(SampleHeaderHandle theSample) {
    if (theSample) {
        itsSamples = theSample;
        UseSamplePtr(sizeof(SampleHeader) + (Sample *)*theSample, (*theSample)->len);
        loopStart = (*itsSamples)->loopStart;
        loopEnd = (*itsSamples)->loopEnd;
        loopCount[0] = loopCount[1] = (*itsSamples)->loopCount;
    } else {
        UseSamplePtr(NULL, 0);
    }
}

void CBasicSound::CalculatePosition(int32_t t) {
    CSoundMixer *m;
    SoundLink *s;
    short i;

    m = itsMixer;
    s = motionLink;

    squareAcc[0] = squareAcc[1] = 0;

    for (i = 0; i < 3; i++) {
        relPos[i] = s->loc[i] - m->motion.loc[i] + (s->speed[i] - m->motion.speed[i]) * t;
        FSquareAccumulate(relPos[i], squareAcc);
    }

    if (0x007Fffff < (unsigned int)squareAcc[0])
        dSquare = 0x7FFFffff;
    else
        dSquare = ((squareAcc[0] & 0x00FFFFFF) << 8) | ((squareAcc[1] & 0xFF000000) >> 24);
}

void CBasicSound::SanityCheck() {
    if (loopStart == loopEnd) {
        loopCount[0] = loopCount[1] = 0;
    }
}

void CBasicSound::FirstFrame() {
    SanityCheck();

    if (motionLink && distanceDelay) {
        int delta;

        if (motionLink->meta == metaNewData)
            CheckSoundLink();
        CalculatePosition(itsMixer->frameStartTime);

        delta = FMul(FSqroot(squareAcc), itsMixer->distanceToSamples);
        currentCount[0].i -= delta;
        currentCount[1].i -= delta;
    }
}

void CBasicSound::CalculateMotionVolume() {
    Fixed rightDot;
    CSoundMixer *m = itsMixer;
    Fixed adjustedVolume;

    if (motionLink->meta == metaNewData)
        CheckSoundLink();

    CalculatePosition(m->frameStartTime);

    if (dSquare == 0)
        dSquare = m->distanceAdjust >> 2;

    adjustedVolume = FMulDivNZ(masterVolume, m->distanceAdjust, dSquare);
    if (adjustedVolume > m->maxAdjustedVolume)
        adjustedVolume = m->maxAdjustedVolume;

    if (m->stereo) {
        distance = FSqroot(squareAcc);

        if (distance > 0) {
            rightDot = FMul(relPos[0], m->rightVector[0]);
            rightDot += FMul(relPos[1], m->rightVector[1]);
            rightDot += FMul(relPos[2], m->rightVector[2]);
            rightDot = FDiv(rightDot, distance);
        } else {
            rightDot = 0; //	Middle, if distance = 0
        }

        if (m->strongStereo) { //	"Hard" stereo effect for loudspeakers users.
            //	Sound position	LEFT		MIDDLE		RIGHT
            //	Left channel	1.0			0.5			0.0
            //	Right channel	0.0			0.5			1.0
            volumes[0] = FMul(FIX1 - rightDot, adjustedVolume) >> (18 - VOLUMEBITS);
            volumes[1] = FMul(FIX1 + rightDot, adjustedVolume) >> (18 - VOLUMEBITS);
        } else { //	"Soft" stereo effect for headphones users.
            //	Sound position	LEFT		MIDDLE		RIGHT
            //	Left channel	1.0			0.66		0.33
            //	Right channel	0.33		0.66		1.0

            adjustedVolume /= 3;
            volumes[0] = FMul(FIX(2) - rightDot, adjustedVolume) >> (18 - VOLUMEBITS);
            volumes[1] = FMul(FIX(2) + rightDot, adjustedVolume) >> (18 - VOLUMEBITS);

            //	With headphones, it is irritating, if one headphone is producing
            //	sound (pressure) and the other one is not. For these cases, the
            //	volume of the otherwise silent channel is set to 1 to avoid the problem.

            if (!volumes[0] ^ !volumes[1]) {
                if (!volumes[0])
                    volumes[0] = 1;
                if (!volumes[1])
                    volumes[1] = 1;
            }
        }

        balance = rightDot;
    } else {
        volumes[0] = adjustedVolume >> (17 - VOLUMEBITS);
        volumes[1] = volumes[0];

        balance = 0;
    }

    if (volumes[0] > volumeMax)
        volumes[0] = volumeMax;
    if (volumes[1] > volumeMax)
        volumes[1] = volumeMax;
}

int16_t CBasicSound::CalcVolume(int16_t theChannel) {
    if (controlLink) {
        if (controlLink->meta == metaSuspend)
            return 0;
        else if (controlLink->meta == metaFade) {
            loopCount[0] = 0;
            loopCount[1] = 0;
        }
    }

    if (motionLink && motionLink->meta == metaFade) {
        loopCount[0] = 0;
        loopCount[1] = 0;
    }

    if (currentCount[0].i <= -itsMixer->soundBufferSize) {
        volumes[theChannel] = 0;
    } else if (motionLink && theChannel == 0) {
        CalculateMotionVolume();
    }

    return volumes[theChannel];
}

void CBasicSound::WriteFrame(int16_t theChannel, int16_t volumeAllowed) {
    //Sample *s;
    //WordSample *d;
    //SampleConvert *converter;
    int thisCount;
    int remaining;
    int baseCount = currentCount[0].i;
    int loopCopy = loopCount[0];

    //converter = &itsMixer->volumeLookup[volumeAllowed - 1];
    //d = itsMixer->mixTo[theChannel];

    if (baseCount >= 0) {
        thisCount = itsMixer->soundBufferSize;
    } else {
        //d -= baseCount;
        thisCount = itsMixer->soundBufferSize + baseCount;
        baseCount = 0;
    }

    do {
        //s = sampleData + baseCount;

        remaining = baseCount + thisCount - loopEnd;
        if (loopCopy && remaining > 0) {
            if (loopCopy > 0) //	Negative loopCount will loop forever.
                loopCopy--;

            thisCount -= remaining;
            baseCount = loopStart;
        } else {
            remaining = 0;
        }

        if (thisCount + baseCount > sampleLen) {
            thisCount = sampleLen - baseCount;
            remaining = 0;
        }

        assert(0 && "port asm");

        /* TODO: port
        asm	{
                clr.w	D0
                move.l	converter,A0
                subq.w	#1,thisCount
                bmi.s	@nothing
        @loop
                move.b	(s)+,D0
                move.w	0(A0,D0.w*2),D1
                add.w	D1,(d)+
                dbra	thisCount,@loop
        @nothing
            }
        */
        thisCount = remaining;
    } while (thisCount);
}

Boolean CBasicSound::EndFrame() {
    if (stopNow || (motionLink && motionLink->meta == metaKillNow) ||
        (controlLink && controlLink->meta == metaKillNow)) {
        stopNow = true;
    } else {
        if (controlLink && controlLink->meta == metaSuspend) {
            return false;
        }

        if (loopCount[0]) {
            int thisCount = itsMixer->soundBufferSize;
            int remaining;

            do {
                if (loopCount[0]) {
                    remaining = currentCount[0].i + thisCount - loopEnd;
                    if (remaining > 0) {
                        if (loopCount[0] > 0) //	Negative loopCount will loop forever.
                            loopCount[0]--;

                        currentCount[0].i = loopStart;
                        thisCount -= remaining;
                    } else
                        remaining = 0;
                } else
                    remaining = 0;

                currentCount[0].i += thisCount;
                thisCount = remaining;

            } while (thisCount);
        } else {
            currentCount[0].i += itsMixer->soundBufferSize;
        }
    }

    if (loopCount[0] == 0 && currentCount[0].i >= sampleLen)
        stopNow = true;

    return stopNow;
}
