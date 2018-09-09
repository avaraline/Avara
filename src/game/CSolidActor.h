/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: CSolidActor.h
    Created: Saturday, December 3, 1994, 11:50
    Modified: Friday, December 8, 1995, 04:31
*/

#pragma once
#include "CGlowActors.h"

class CSolidActor : public CGlowActors {
public:
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
};
