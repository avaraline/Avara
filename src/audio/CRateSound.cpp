/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CRateSound.c
    Created: Monday, December 26, 1994, 09:06
    Modified: Tuesday, September 3, 1996, 21:27
*/

#include "CRateSound.h"

#include "CSoundMixer.h"
#include "FastMat.h"

void CRateSound::Reset() {
    CBasicSound::Reset();

    midRate = FIX1;
    masterRate = FIX1;
}

void CRateSound::UseSamplePtr(Sample *samples, int numSamples) {
    CBasicSound::UseSamplePtr(samples, numSamples);
    SetRate(FIX1);
}

#define MINIMUMRATE (65536L >> (16 - BASESOUNDBUFFERBITS))

void CRateSound::SetRate(Fixed aRate) {
    Fixed adjustedRate;

    if (itsSamples)
        adjustedRate = FMul((*itsSamples)->baseRate, aRate);
    else
        adjustedRate = aRate;

    adjustedRate = FMul(adjustedRate, itsMixer->standardRate);

    if (adjustedRate < MINIMUMRATE)
        adjustedRate = MINIMUMRATE;
    midRate = adjustedRate;
    masterRate = adjustedRate;

    chanRate[0] = masterRate;
    chanRate[1] = masterRate;
}

UnsignedFixed CRateSound::GetSampleRate() {
    return FMul(itsMixer->samplingRate, masterRate);
}

void CRateSound::FirstFrame() {
    SanityCheck();

    phaseBalance = 0;

    if (motionLink && distanceDelay) {
        int delta;

        if (motionLink->meta == metaNewData)
            CheckSoundLink();
        CalculatePosition(itsMixer->frameStartTime);

        delta = FMul(masterRate, FMul(FSqroot(squareAcc), itsMixer->distanceToSamples));
        currentCount[0].i -= delta;
        currentCount[1].i -= delta;
    }
}

short CRateSound::CalcVolume(int16_t theChannel) {
    //short theVol;
    //int endSample;
    //unsigned int fracAdd;

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

    if (theChannel == 0) {
        chanDoneFlags[0] = chanDoneFlags[1] = false;

        if (motionLink) {
            Fixed startDistance;
            Fixed dopplerShift;
            Fixed rateMultip;
            CSoundMixer *m = itsMixer;

            CalculateMotionVolume();

            if (m->stereo)
                startDistance = distance;
            else
                startDistance = FSqroot(squareAcc);

            CalculatePosition(m->frameEndTime);

            distance = FSqroot(squareAcc);
            dopplerShift = FMul(m->distanceToSamples, startDistance - distance);

            rateMultip = FIX1 + (dopplerShift << (16 - itsMixer->soundBufferBits)); //	‚Ä¢‚Ä¢‚Ä¢

            if (rateMultip > FIX(4))
                midRate = masterRate * 4;
            else if (rateMultip < FIX3(500))
                midRate = masterRate >> 1;
            else
                midRate = FMul(masterRate, rateMultip);

            if (m->stereo) {
                Fixed rateDiff;

                rateDiff = FMul(midRate, balance) >> (itsMixer->soundBufferBits - 4); //	‚Ä¢‚Ä¢‚Ä¢
                chanRate[1] = midRate + rateDiff - phaseBalance;
                chanRate[0] = midRate - rateDiff + phaseBalance;
                phaseBalance = rateDiff;
            } else {
                chanRate[0] = midRate;
                chanRate[1] = midRate;
            }

            if (currentCount[0].i < -(chanRate[0] >> (16 - itsMixer->soundBufferBits)) //	‚Ä¢‚Ä¢‚Ä¢
                || (loopCount[0] == 0 && currentCount[0].i >= sampleLen))
                volumes[0] = 0;

            if (itsMixer->stereo) {
                if (currentCount[1].i < -(chanRate[1] >> (16 - itsMixer->soundBufferBits)) //	‚Ä¢‚Ä¢‚Ä¢
                    || (loopCount[1] == 0 && currentCount[1].i >= sampleLen))
                    volumes[1] = 0;
            }
        }
    }

    return volumes[theChannel];
}

void CRateSound::WriteFrame(int16_t theChannel, int16_t volumeAllowed) {
    WordSample *d;
    WordSample *converter;
    SampleIndex ind;
    short thisCount;
    int didCount;
    //int maxOutput;
    int loopCopy = loopCount[theChannel];
    Fixed rateCopy = chanRate[theChannel];

    converter = itsMixer->volumeLookup[volumeAllowed - 1];
    d = itsMixer->mixTo[theChannel];

    ind = currentCount[theChannel];
    thisCount = itsMixer->soundBufferSize;

    if (ind.i < 0) {
        didCount = FDiv(ind.i, rateCopy);
        thisCount += didCount;
        d -= didCount;
        ind.i = 0;
        ind.f = 0;
    }

    while (thisCount > 0) {
        if (loopCopy) {
            didCount = RateMixer(sampleData, d, converter, thisCount, loopEnd, &ind, rateCopy);
            d += didCount;
            thisCount -= didCount;

            if (ind.i >= loopEnd) {
                if (loopCopy > 0)
                    loopCopy--;
                ind.i += loopStart - loopEnd;
            }
        } else {
            RateMixer(sampleData, d, converter, thisCount, sampleLen, &ind, rateCopy);
            thisCount = 0;
        }
    }

    chanDoneFlags[theChannel] = true;
    currentCount[theChannel] = ind;
    loopCount[theChannel] = loopCopy;
}

Boolean CRateSound::EndFrame() {
    short chan, chanCount;
    short stopCount;

    chanCount = itsMixer->stereo + 1;
    stopCount = chanCount;

    if (stopNow || (motionLink && motionLink->meta == metaKillNow) ||
        (controlLink && controlLink->meta == metaKillNow)) {
        stopNow = true;
    } else {
        int thisCount;
        int remaining;
        unsigned int fracAdd;
        Fixed rateCopy;

        if (controlLink && controlLink->meta == metaSuspend) {
            return false;
        }

        for (chan = 0; chan < chanCount; chan++) {
            if (loopCount[chan] == 0 && currentCount[chan].i >= sampleLen) {
                stopCount--;
            } else if (chanDoneFlags[chan] == false) {
                rateCopy = chanRate[chan];

                fracAdd = ((unsigned short)rateCopy) << itsMixer->soundBufferBits; //	‚Ä¢‚Ä¢‚Ä¢
                fracAdd += currentCount[chan].f;
                currentCount[chan].f = fracAdd;

                // ORIGINAL: thisCount = (fracAdd >> 16) + rateCopy >> (16-itsMixer->soundBufferBits);
                thisCount = ((fracAdd >> 16) + rateCopy) >> (16 - itsMixer->soundBufferBits);

                do {
                    if (loopCount[chan]) {
                        remaining = currentCount[chan].i + thisCount - loopEnd;
                        if (remaining > 0) {
                            if (loopCount[chan] > 0) //	Negative loopCount will loop forever.
                                loopCount[chan]--;

                            currentCount[chan].i += loopStart - loopEnd;
                            thisCount -= remaining;
                        } else
                            remaining = 0;
                    } else
                        remaining = 0;

                    currentCount[chan].i += thisCount;
                    thisCount = remaining;

                } while (thisCount);
            }
        }
    }

    if (stopCount == 0)
        stopNow = true;

    return stopNow;
}
