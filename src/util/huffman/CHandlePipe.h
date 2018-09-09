/**
    A fairly trivial class that outputs all its input to a handle.
 */

#pragma once
#include "CAbstractPipe.h"

class CHandlePipe : public CAbstractPipe {
public:
    Handle outputHandle;

    CHandlePipe() {}
    virtual ~CHandlePipe() {}

    virtual OSErr Open();

    virtual void PipeToHandle(Handle output);
    virtual Handle GetDataHandle();

    virtual OSErr PipeData(Ptr dataPtr, long len);
    virtual void Dispose();
};
