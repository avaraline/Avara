/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: CHologramActor.h
    Created: Tuesday, November 29, 1994, 04:47
    Modified: Wednesday, March 15, 1995, 07:03
*/

#pragma once
#include "CPlacedActors.h"

class CHologramActor : public CPlacedActors {
public:
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
};
