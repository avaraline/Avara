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
    Vector location;
    Fixed radius;
    MsgType enterMsg;
    MsgType exitMsg;

    short frequency;
    short freqCount;
    Boolean areaFlag;
    MaskType watchBits;
    short watchTeams;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void FrameAction();
};
