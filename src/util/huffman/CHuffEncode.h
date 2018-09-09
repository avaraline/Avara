#pragma once
#include "CAbstractHuffPipe.h"

/**
    This data encodes its input using a form of adaptive Huffman compression.
 */

// Size of output buffer in long ints.
#define ENCODEBUFFERSIZE 128

class CHuffEncode : public CAbstractHuffPipe {
public:
    long codeStrings[NUMSYMBOLS];
    short codeLengths[NUMSYMBOLS];

    long *outputBuffer;
    long bitsInBuffer;

    CHuffEncode() {}
    virtual ~CHuffEncode() {}

    virtual OSErr Open();
    virtual void ClearBuffer();
    virtual OSErr PipeData(Ptr dataPtr, long len);
    virtual void CreateSymbolTables();
    virtual OSErr Close();
    virtual void Dispose();
};
