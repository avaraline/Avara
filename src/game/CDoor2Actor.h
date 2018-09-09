/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CDoor2Actor.h
    Created: Tuesday, November 21, 1995, 18:37
    Modified: Tuesday, November 21, 1995, 18:41
*/

#pragma once
#include "CDoorActor.h"

class CDoor2Actor : public CDoorActor {
public:
    Fixed midState;
    Vector midDeltas;
    Vector midTwists;

    virtual void PlaceParts();
    virtual void ProgramDoorVariables();
    virtual void ReadDoorVariables();
};
