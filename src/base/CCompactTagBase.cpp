#include "CCompactTagBase.h"

#include "CHuffmanHandler.h"

Handle CCompactTagBase::ConvertToHandle() {
    Handle raw, compressed;
    CHuffmanHandler *theCompressor;

    compressed = NULL;
    raw = CTagBase::ConvertToHandle();
    if (raw) {
        theCompressor = new CHuffmanHandler;
        theCompressor->Open();
        compressed = theCompressor->Compress(raw);
        DisposeHandle(raw);
        theCompressor->Dispose();
    }

    return compressed;
}

void CCompactTagBase::ConvertFromHandle(Handle theHandle) {
    Handle raw;
    CHuffmanHandler *theCompressor;

    if (theHandle) {
        theCompressor = new CHuffmanHandler;
        theCompressor->Open();
        raw = theCompressor->Uncompress(theHandle);
        theCompressor->Dispose();

        CTagBase::ConvertFromHandle(raw);

        DisposeHandle(raw);
    }
}
