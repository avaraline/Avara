/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: SoundSystemDefines.h
    Created: Friday, December 23, 1994, 10:58
    Modified: Tuesday, September 3, 1996, 21:21
*/

#pragma once

#include "OggFile.h"
#include "Types.h"

#include <memory>

#define HSOUNDRESTYPE 'HSND'
#define BITSPERSAMPLE 7
#define SAMPLERANGE (1 << BITSPERSAMPLE)
#define VOLUMEBITS 7
#define VOLUMERANGE (1 << VOLUMEBITS)

/*	When using 8 bit sound output hardware, very low volumes tend
**	to sound very bad, since the amount of bits that are played
**	is reduced so much. The following constant is the cut off volume
**	below which volumes are set to 0.
*/
#define MINIMUM8BITVOLUME 2

/*
**	The following constant is the maximum allowed volume before stereo
**	calculations are made. This is to ensure that loud sounds playing
**	close to the observer do not max out on both channels.
*/
#define MAXADJUSTEDVOLUME FIX3(1000)

#define BASESOUNDBUFFERBITS 10
#define BASESOUNDBUFFERSIZE (1 << BASESOUNDBUFFERBITS)
#define SILENCE 128
#define WORDSILENCE 0

typedef uint8_t Sample;
typedef int16_t WordSample;

enum { metaNoData, metaNewData, metaKillNow, metaSuspend, metaFade };

typedef struct {
    int32_t i;
    uint16_t f;
} SampleIndex;

#define FIXSAMPLEINDEX(ind) *(long *)(1 + (int16_t *)&ind)

typedef struct {
    int16_t refCount;
    int16_t meta;

    Fixed loc[3];
    Fixed speed[3];

    Fixed nLoc[3];
    Fixed nSpeed[3];
    Fixed t;

} SoundLink;

typedef WordSample SampleConvert[SAMPLERANGE];

int16_t RateMixer(std::shared_ptr<OggFile> source,
    WordSample *dest,
    WordSample *converter,
    int16_t outCount,
    int32_t endOffset,
    SampleIndex *current,
    UnsignedFixed theRate);

void InterleaveStereoChannels(WordSample *leftCh, WordSample *rightCh, WordSample *blendTo, size_t bufferSize);
