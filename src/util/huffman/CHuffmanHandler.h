#pragma once
#include "CAbstractHuffPipe.h"

typedef struct {
    uint32_t decodedSize;
    uint32_t countBitmap[NUMSYMBOLS >> 5];
} HuffHandleHeader;

#define HUFFHANDLELOOKUPBITS 10
#define HUFFHANDLELOOKUPSIZE (1 << HUFFHANDLELOOKUPBITS)

class CHuffmanHandler : public CAbstractHuffPipe {
public:
    Handle inHandle;
    Handle outHandle;
    Ptr outPointer;
    long *codeStrings;
    short *codeLengths;
    HuffTreeNode **lookupBuf;
    Boolean singleSymbolData = 0;
    unsigned char theSingleSymbol = 0;

    CHuffmanHandler() {}
    virtual ~CHuffmanHandler() {}

    virtual void CreateLookupBuffer();
    virtual void CreateSymbolTable();
    virtual void WriteCompressed();
    virtual Handle Compress(Handle sourceHandle);
    virtual void DecodeAll(unsigned char *source, unsigned char *dest);
    virtual Handle Uncompress(Handle sourceHandle);
    virtual long GetUncompressedLen(Handle sourceHandle);
    virtual void UncompressTo(Handle sourceHandle, Ptr toPtr);
};
