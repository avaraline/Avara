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
    std::string theMessage;
    MessageRecord in;
    MessageRecord startMsg;
    MessageRecord stopMsg;
    Boolean enabled;
    Boolean showEveryone;

    FrameNumber nextShowTime;
    long restartDelay;
    Vector location;
    Fixed radius;
    FrameNumber frequency;
    Boolean areaFlag;
    short alignment;

    FrameNumber textTimer;
    long showTime;

    short soundId;
    Fixed soundVol;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void FrameAction();
    virtual void Dispose();
};
