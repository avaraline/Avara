#pragma once
#include "CAbstractHuffPipe.h"

typedef struct {
    uint32_t decodedSize;
    uint32_t countBitmap[NUMSYMBOLS >> 5];
} HuffDataHeader;

#define HUFFHANDLELOOKUPBITS 10
#define HUFFHANDLELOOKUPSIZE (1 << HUFFHANDLELOOKUPBITS)

class CHuffProcessor : public CAbstractHuffPipe {
public:
    Ptr outPointer = 0;
    int outCount = 0;

    HuffTreeNode **lookupBuf = 0;
    Boolean singleSymbolData = 0;
    unsigned char theSingleSymbol = 0;
    int codeStrings[NUMSYMBOLS] = {0};
    short codeLengths[NUMSYMBOLS] = {0};

    CHuffProcessor() {}
    virtual ~CHuffProcessor() {}

    virtual void CreateLookupBuffer();
    virtual void CreateSymbolTable();
    virtual void DecodeAll(unsigned char *source, unsigned char *dest);

    virtual int GetUncompressedLen(Ptr compressedData);
    virtual int GetCompressedLen(Ptr sourceData, int sourceLen);

    virtual void Compress(Ptr fromPtr, Ptr destPtr);
    virtual void Uncompress(Ptr fromPtr, Ptr destPtr);
};
