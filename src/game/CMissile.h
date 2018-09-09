/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: CMissile.h
    Created: Sunday, December 18, 1994, 03:28
    Modified: Wednesday, March 8, 1995, 06:29
*/

#pragma once
#include "CAbstractMissile.h"

#define kMissileBSP 801

class CMissile : public CAbstractMissile {
public:
    virtual void IAbstractMissile(CDepot *theDepot);
};
