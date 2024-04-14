/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CTextActor.h
    Created: Saturday, January 20, 1996, 03:33
    Modified: Thursday, February 22, 1996, 08:07
*/

#pragma once
#include "CAbstractActor.h"

class CTextActor : public CAbstractActor {
public:
    std::string theMessage = "";
    MessageRecord in = {0};
    MessageRecord startMsg = {0};
    MessageRecord stopMsg = {0};
    Boolean enabled = 0;
    Boolean showEveryone = 0;

    FrameNumber nextShowTime = 0;
    long restartDelay = 0;
    Vector location = {0};
    Fixed radius = 0;
    FrameNumber frequency = 0;
    Boolean areaFlag = 0;
    short alignment = 0;

    FrameNumber textTimer = 0;
    long showTime = 0;

    short soundId = 0;
    Fixed soundVol = 0;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void FrameAction();
    virtual ~CTextActor();
};
