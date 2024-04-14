/*
    Copyright ©1992-1994, Juri Munkki
    All rights reserved.

    File: CBaseObject.h
    Created: Friday, October 23, 1992, 16:22
    Modified: Tuesday, October 18, 1994, 15:49
*/

#pragma once

#include "Memory.h"

class CBaseObject {
protected:
    short lockCounter = 0;

public:
    CBaseObject() {}
    virtual ~CBaseObject() {}

    virtual void IBaseObject(void);
    virtual void Lock(void);
    virtual void Unlock(void);

    virtual void LockThis(void);
    virtual void UnlockThis(void);

    virtual void ForceUnlock(void);
    virtual void Dispose(void);
    virtual CBaseObject *Clone(void);
    virtual void CloneFields(void);
    virtual Handle CloneHandle(Handle theHandle);

    virtual Size HowMuchMemory(void);
};
