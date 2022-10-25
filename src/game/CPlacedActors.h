/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: CPlacedActors.h
    Created: Tuesday, November 22, 1994, 05:45
    Modified: Thursday, December 7, 1995, 10:32
*/

#pragma once
#include "CAbstractActor.h"
#include "FastMat.h"

class CPlacedActors : public CAbstractActor {
public:
    Vector location;
    Fixed heading;

    virtual void IAbstractActor();
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual bool UseForRandomIncarnator() { return false; }
};
