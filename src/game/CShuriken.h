/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CShuriken.h
    Created: Tuesday, December 5, 1995, 05:30
    Modified: Tuesday, December 5, 1995, 06:58
*/

#pragma once
#include "CAbstractMissile.h"

#define SHURIKENSPEED FIX3(1500)

class CShuriken final : public CAbstractMissile {
public:
    Fixed rotation;

    CShuriken(CDepot *theDepot);
    virtual void MoveForward();
    virtual bool HandlesFastFPS() { return true; }
};
