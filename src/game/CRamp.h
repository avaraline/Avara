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
    virtual void BeginScript() override;
    virtual CAbstractActor *EndScript() override;
    virtual bool IsGeometryStatic() override { return true; };
    virtual bool UseForRandomIncarnator() override { return true; }
};
