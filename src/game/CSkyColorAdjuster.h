/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: CSkyColorAdjuster.h
    Created: Tuesday, November 29, 1994, 08:24
    Modified: Saturday, December 30, 1995, 08:08
*/

#pragma once
#include "CAbstractActor.h"

class CSkyColorAdjuster : public CAbstractActor {
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
};
