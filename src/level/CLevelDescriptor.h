/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: CLevelDescriptor.h
    Created: Wednesday, November 30, 1994, 08:10
    Modified: Thursday, December 7, 1995, 03:12
*/

#pragma once

#include "CDirectObject.h"
#include "Types.h"

#define LEVELLISTRESTYPE 'LEDI'
#define LEVELLISTRESID 128
#define MAXENABLES 64

class CLevelDescriptor : public CDirectObject {
public:
    short orderNumber;
    OSType tag;
    Str255 name;
    Str255 access;
    Str255 intro;
    Boolean fromFile;

    short enablesNeeded;
    short enablesReceived;

    short countEnables;
    OSType winEnables[MAXENABLES];

    class CLevelDescriptor *nextLevel;

    virtual void ILevelDescriptor(Ptr levelInfo, short levelsLeft);
    virtual void Dispose();
};

CLevelDescriptor *LoadLevelListFromResource(OSType *currentDir);
