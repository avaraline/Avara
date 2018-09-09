/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: CTriPyramidActor.h
    Created: Saturday, December 3, 1994, 14:45
    Modified: Wednesday, March 15, 1995, 07:03
*/

#pragma once
#include "CPlacedActors.h"

#define kTriPyramidBSP 650

class CTriPyramidActor : public CPlacedActors {
public:
    virtual CAbstractActor *EndScript();
};
