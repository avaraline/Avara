/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CAbstractCommServer.h
    Created: Monday, October 30, 1995, 18:47
    Modified: Monday, October 30, 1995, 18:46
*/

#pragma once
#include "CCommManager.h"

class CAbstractCommServer : public CCommManager {
public:
    Boolean isServing;

    virtual void ICommServer(short clientCount, short bufferCount);
    virtual void StartServing();
    virtual void StopServing();
};
