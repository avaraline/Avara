/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: CPlayerActor.h
    Created: Tuesday, November 22, 1994, 05:45
    Modified: Monday, March 20, 1995, 16:48
*/

#pragma once
#include "CAbstractPlayer.h"

class CBSPPart;
class CPlayerMissile;

class CPlayerActor : public CAbstractPlayer {
public:
    virtual void LoadParts();
    virtual void StartSystems();
    virtual void PlaceParts();
    virtual void AvoidBumping();
};
