/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CWallDoor.h
    Created: Thursday, December 7, 1995, 09:19
    Modified: Thursday, December 7, 1995, 09:19
*/

#pragma once
#include "CDoor2Actor.h"

class CWallDoor : public CDoor2Actor {
public:
    virtual void LoadPart(short ind, short resId);
};
