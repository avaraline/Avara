#include "CHandlePipe.h"

OSErr CHandlePipe::Open() {
    short theErr;
    theErr = CAbstractPipe::Open();
    if (theErr == noErr) {
        outputHandle = NewHandle(0);
        theErr = MemError();
    }
    return theErr;
}

void CHandlePipe::PipeToHandle(Handle output) {
    if (outputHandle) {
        DisposeHandle(outputHandle);
    }
    outputHandle = output;
}

Handle CHandlePipe::GetDataHandle() {
    return outputHandle;
}

OSErr CHandlePipe::PipeData(Ptr dataPtr, long len) {
    if (outputHandle) {
        PtrAndHand(dataPtr, outputHandle, len);
        return MemError();
    } else {
        return writErr;
    }
}

void CHandlePipe::Dispose() {
    if (outputHandle) {
        DisposeHandle(outputHandle);
    }
    CAbstractPipe::Dispose();
}
