/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CAbstractCommClient.h
    Created: Monday, October 30, 1995, 18:52
    Modified: Monday, October 30, 1995, 18:56
*/

#pragma once
#include "CCommManager.h"

/*
**	Abstract client class.
*/
class CAbstractCommClient : public CCommManager {
public:
    Boolean isConnected; //	Accessed publicly
    Boolean isOpen; //	Accessed publicly.

    virtual void ICommClient(short bufferCount);
    virtual void Connect();
    virtual void Disconnect();
};
