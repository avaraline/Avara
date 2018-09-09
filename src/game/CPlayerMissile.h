/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CPlayerMissile.h
    Created: Saturday, November 26, 1994, 11:17
    Modified: Monday, January 29, 1996, 14:44
*/

#pragma once
#include "CAbstractMissile.h"

#define PLAYERMISSILESPEED FIX3(4000 + 2000)
#define PLAYERMISSILELIFE 25
#define PLAYERMISSILERANGE (PLAYERMISSILELIFE * PLAYERMISSILESPEED)

class CPlayerMissile : public CAbstractMissile {
public:
    Fixed rotation;
    Fixed spin;

    virtual void IAbstractMissile(CDepot *theDepot);
    virtual void SetSpin(Boolean spinDirection);
    virtual void MoveForward();
};
