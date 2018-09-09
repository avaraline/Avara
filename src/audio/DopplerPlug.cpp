/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: DopplerPlug.c
    Created: Friday, May 5, 1995, 16:21
    Modified: Sunday, August 18, 1996, 23:03
*/

#define DOPPLERPLUG

#include "SoundSystemDefines.h"

/*
    The RateMixer function is used by sound channels that have a playing
    rate other than 1.0. The samples are stored at source. The copying
    ends when the buffer is full (outCount samples have been written) or
    we reach endOffset. The record pointed to by 'current' is updated at
    the end and the number of samples not yet written is returned.
*/
short RateMixer(Sample *source,
    WordSample *dest,
    WordSample *converter,
    short outCount,
    int endOffset,
    SampleIndex *current,
    Fixed theRate) {
    int offset;
    int fracOffset;
    int rate;
    int fracRate;
    int i;

    if (outCount > 0) {
        i = outCount;
        offset = current->i - endOffset;
        fracOffset = current->f;

        source += endOffset;

        if (offset < 0) {
            rate = theRate;
            fracRate = (unsigned short)rate;
            rate >>= 16;

            do {
                *(dest++) += converter[source[offset]];
                fracOffset = fracRate + (unsigned short)fracOffset;
                offset += (fracOffset >> 16) + rate;
            } while (--i && offset < 0);

            current->i = offset + endOffset;
            current->f = fracOffset;
        }

        outCount -= i;
    }

    return outCount;
}

void InterleaveStereoChannels(WordSample *leftCh, WordSample *rightCh, WordSample *blendTo, short bufferSize) {
    short i = bufferSize;

    do {
        *blendTo++ = *leftCh++;
        *blendTo++ = *rightCh++;
    } while (--i);
}
