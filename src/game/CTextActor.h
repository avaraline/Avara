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
    Str255 theMessage;
    MessageRecord in;
    MessageRecord startMsg;
    MessageRecord stopMsg;
    Boolean enabled;
    Boolean showEveryone;

    long nextShowTime;
    long restartDelay;
    Vector location;
    Fixed radius;
    long frequency;
    Boolean areaFlag;
    short alignment;

    long textTimer;
    long showTime;

    short soundId;
    Fixed soundVol;

    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
    virtual void FrameAction();
    virtual void Dispose();
};
