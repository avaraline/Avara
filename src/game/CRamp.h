/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CRamp.h
    Created: Wednesday, January 24, 1996, 05:34
    Modified: Wednesday, January 24, 1996, 05:34
*/

#pragma once
#include "CPlacedActors.h"

class CRamp : public CPlacedActors {
public:
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual bool UseForRandomIncarnator() { return true; }
};
