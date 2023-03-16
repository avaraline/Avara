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
    Ptr outPointer;
    int outCount;

    HuffTreeNode **lookupBuf;
    Boolean singleSymbolData;
    unsigned char theSingleSymbol;
    int codeStrings[NUMSYMBOLS];
    short codeLengths[NUMSYMBOLS];

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
