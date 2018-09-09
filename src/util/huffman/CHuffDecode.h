#pragma once
#include "CAbstractHuffPipe.h"

#define HUFFLOOKUPBITS 8
#define HUFFLOOKUPSIZE (1 << HUFFLOOKUPBITS)
#define BITBUFFERSIZE 512
#define DECODEBUFFERSIZE 512

class CHuffDecode : public CAbstractHuffPipe {
public:
    long outputPosition;
    long bitPosition;
    long bytesInBuffer;
    char codeLengths[NUMSYMBOLS];
    HuffTreeNode *lookupBuf[HUFFLOOKUPSIZE];
    unsigned char *outputBuffer;
    unsigned char *bitBuffer;

    CHuffDecode() {}
    virtual ~CHuffDecode() {}

    virtual OSErr Open();
    virtual void CreateLookupBuffer();
    virtual OSErr PipeData(Ptr dataPtr, long len);
    virtual OSErr Close();
    virtual void Dispose();
};
