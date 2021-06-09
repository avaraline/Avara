#pragma once
#include "CBaseObject.h"

class CAbstractPipe : public CBaseObject {
public:
    class CAbstractPipe *outputStream = 0;

    CAbstractPipe() {}
    virtual ~CAbstractPipe() {}

    // In general, pipe methods are used in this order:
    virtual OSErr Open();
    virtual OSErr PipeTo(CAbstractPipe *output);
    virtual OSErr PipeData(Ptr dataPtr, long len);
    virtual OSErr Close();
};
