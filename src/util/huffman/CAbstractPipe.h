#pragma once

#include "Types.h"

class CAbstractPipe {
public:
    class CAbstractPipe *outputStream;

    CAbstractPipe() {}
    virtual ~CAbstractPipe() {}

    // In general, pipe methods are used in this order:
    virtual OSErr Open();
    virtual OSErr PipeTo(CAbstractPipe *output);
    virtual OSErr PipeData(Ptr dataPtr, long len);
    virtual OSErr Close();
};
