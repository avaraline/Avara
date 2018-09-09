/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CDome.h
    Created: Sunday, May 26, 1996, 15:11
    Modified: Sunday, May 26, 1996, 15:11
*/

#pragma once
#include "CGlowActors.h"

class CDome : public CGlowActors {
public:
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
};
