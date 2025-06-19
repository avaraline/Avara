#include "CAbstractPipe.h"

/**
    Open a null pipe that does nothing. Basically a /dev/null kind of device.
 */
OSErr CAbstractPipe::Open() {
    outputStream = NULL;
    return noErr;
}

/**
    Set the next pipe in chain.
 */
OSErr CAbstractPipe::PipeTo(CAbstractPipe *output) {
    outputStream = output;
    return noErr;
}

/**
    In essence, this is the Write command for a pipe. Whatever is written with this command is
    processed and then output to the output pipe, if there is one or the use of one is allowed.
 */
OSErr CAbstractPipe::PipeData(Ptr dataPtr, long len) {
    if (outputStream) {
        return outputStream->PipeData(dataPtr, len);
    } else {
        return noErr;
    }
}

/**
    Closes the pipe. Subclasses can flush any data they have buffered and then close their output
    pipe. It's probably not a good idea to close the output pipe at this point, since the user might
    want to use it for more data. Maybe a separate flush command and then a close could be in order?
 */
OSErr CAbstractPipe::Close() {
    if (outputStream) {
        return outputStream->Close();
    } else {
        return noErr;
    }
}
