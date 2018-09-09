/*
    Copyright ©1994, Juri Munkki
    All rights reserved.

    File: CharWordLongPointer.c
    Created: Wednesday, November 30, 1994, 08:22
    Modified: Wednesday, November 30, 1994, 08:42
*/

#include "CharWordLongPointer.h"

void CopyEvenPascalString(charWordLongP *uniP, StringPtr dest, short maxChars) {
    short realLen;
    short copyChars;
    char *p;

    realLen = 1 + uniP->uc[0];
    if (realLen > maxChars) {
        copyChars = maxChars;
    } else {
        copyChars = realLen;
    }

    p = uniP->c;

    while (copyChars--) {
        *dest++ = *p++;
    }

    uniP->c += realLen + (realLen & 1);
}
