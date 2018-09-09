/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CWallSolid.h
    Created: Friday, December 8, 1995, 04:30
    Modified: Friday, December 8, 1995, 04:30
*/

#pragma once
#include "CSolidActor.h"

class CWallSolid : public CSolidActor {
public:
    virtual void LoadPart(short ind, short resId);
};
