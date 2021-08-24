/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: CharWordLongPointer.h
    Created: Wednesday, November 30, 1994, 08:03
    Modified: Monday, August 21, 1995, 05:09
*/

#pragma once

#include "Types.h"

typedef union {
    char *c;
    short *w;
    int *l;

    unsigned char *uc;
    unsigned short *uw;
    unsigned int *ul;

    StringPtr str;

} charWordLongP;
