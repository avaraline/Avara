/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CIncarnator.h
    Created: Tuesday, May 23, 1995, 17:39
    Modified: Saturday, February 17, 1996, 04:18
*/

#pragma once
#include "CPlacedActors.h"

class CIncarnator;

class CIncarnator : public CPlacedActors {
public:
    CIncarnator *nextIncarnator;
    long colorMask = 0;
    long useCount = 0;
    MessageRecord startMsg = {0};
    MessageRecord stopMsg = {0};
    Boolean enabled = 0;
    Fixed distance = 0;  // randomized distance to nearest opponent

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void FrameAction();
    virtual bool UseForRandomIncarnator() { return true; }
};
