/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CAreaActor.h
    Created: Sunday, December 4, 1994, 12:53
    Modified: Saturday, January 20, 1996, 04:24
*/

#pragma once
#include "CAbstractActor.h"
#include "FastMat.h"

class CAreaActor : public CAbstractActor {
public:
    Vector location = {0};
    Fixed radius = 0;
    MsgType enterMsg = 0;
    MsgType exitMsg = 0;

    short frequency = 0;
    short freqCount = 0;
    Boolean areaFlag = 0;
    MaskType watchBits = 0;
    short watchTeams = 0;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void FrameAction();
};
