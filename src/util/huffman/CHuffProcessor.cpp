#include "CHuffProcessor.h"

#ifndef INTEL_ARCH
#define MEM_LONG(val) val
#define MEM_SHORT(val) val
#else
#define MEM_LONG(val) ((val << 24) | ((val << 8) & 0xFF0000) | ((val >> 8) & 0xFF00) | ((val >> 24) & 0xFF))
#define MEM_SHORT(val) (short)((val << 8) | ((val >> 8) & 0xFF))
#endif

/**
    Decoding the data is a bit more complicated than writing it, since we have to walk the tree to
    find what maps to what. We can, however, take a shortcut by reading HUFFHANDLELOOKUPBITS bits at
    a time. If the code we reach is shorter than this, all we need to do is back up a few bits. If
    we did not reach a leaf node with the lookup, we continue bit by bit until we reach a leaf.

    This routine creates the lookup table for HUFFHANDLELOOKUPBITS bits and it creates a table of
    lengths so that we can find out how many bits were needed for the last char.

    Recursion has been eliminated with a stack and the routine is a modified version of the one used
    to create the tables in the encoder object.

    The routine goes through the patterns in order starting with 000... and ending at 111...
    Because of this, the "holes" in the lookup table can be filled as we go. The last pointer
    written to the table has to be replicated until the end of the table.

    If BuildTree returns a leaf node, then the tree can only have one symbol in it and using huffman
    compression doesn't make any sense (since the code consists of only one repeated symbol).
 */
void CHuffProcessor::CreateLookupBuffer() {
    short lastLookup;
    short walkerStack[33];
    short *stackPtr;
    short codeLen = 1;
    int codeString = 0;
    HuffTreeNode *theNode;
    HuffTreeNode *lastNode;

    theNode = BuildTree();

    singleSymbolData = (theNode->left < 0);
    theSingleSymbol = theNode->right;

    // Must be branch node.
    if (!singleSymbolData) {
        stackPtr = walkerStack + 1;
        *stackPtr++ = theNode->right;
        theNode = &nodes[theNode->left];
        lastLookup = 0;

        do {
            if (codeLen == HUFFHANDLELOOKUPBITS || (theNode->left < 0 && codeLen < HUFFHANDLELOOKUPBITS)) {
                int codeIndex;
                short i;
                codeIndex = codeString << (HUFFHANDLELOOKUPBITS - codeLen);
                lastNode = lookupBuf[lastLookup];
                while (lastLookup < codeIndex) {
                    lookupBuf[lastLookup++] = lastNode;
                }
                lookupBuf[codeIndex] = theNode;
            }

            if (theNode->left < 0) {
                codeLengths[theNode->right] = codeLen;
                theNode = &nodes[*--stackPtr];
                while (codeString & 1) {
                    codeLen--;
                    codeString >>= 1;
                };
                codeString |= 1;
            } else {
                *stackPtr++ = theNode->right;
                theNode = &nodes[theNode->left];
                codeLen++;
                codeString += codeString;
            }
        } while (stackPtr != walkerStack);

        lastNode = lookupBuf[lastLookup];
        while (lastLookup < HUFFHANDLELOOKUPSIZE) {
            lookupBuf[lastLookup++] = lastNode;
        }
    }
}

/**
    Create a mapping from characters to bit patterns. Lengths are stored in a different array. Bit
    patterns may be up to 24 bits int (the limitation comes from the output code).

    A stack is used to eliminate recursion while walking the tree.

    If BuildTree returns a leaf node, then the tree can only have one symbol in it and using huffman
    compression doesn't make any sense (since the code consists of only one repeated symbol).
 */
void CHuffProcessor::CreateSymbolTable() {
    short walkerStack[33];
    short *stackPtr;
    short codeLen = 1;
    int codeString = 0;
    HuffTreeNode *theNode;

    theNode = BuildTree();

    singleSymbolData = (theNode->left < 0);

    // Must be branch node.
    if (!singleSymbolData) {
        stackPtr = walkerStack + 1;
        *stackPtr++ = theNode->right;
        theNode = &nodes[theNode->left];

        do {
            if (theNode->left < 0) {
                codeStrings[theNode->right] = codeString;
                codeLengths[theNode->right] = codeLen;
                theNode = &nodes[*--stackPtr];
                while (codeString & 1) {
                    codeLen--;
                    codeString >>= 1;
                };
                codeString |= 1;
            } else {
                *stackPtr++ = theNode->right;
                theNode = &nodes[theNode->left];
                codeLen++;
                codeString += codeString;
            }
        } while (stackPtr != walkerStack);
    }
}

void CHuffProcessor::DecodeAll(unsigned char *source, unsigned char *dest) {
    int i;
    int bitData;
    int *bitPointer;
    int bitPosition = 0;
    HuffTreeNode *theNode;
    short theCode;

    i = dataCount;
    while (i--) {
        // Read in some bits into bitData.
        bitPointer = (int *)(source + (bitPosition >> 3));
#ifdef INTEL_ARCH
        {
            unsigned char *charPointer = (unsigned char *)bitPointer;

            bitData = (((unsigned int)charPointer[0] << 24) | ((unsigned int)charPointer[1] << 16) |
                          ((unsigned int)charPointer[2] << 8) | (unsigned int)charPointer[3])
                      << (bitPosition & 7);
        }
#else
        bitData = (*bitPointer) << (bitPosition & 7);
#endif

        // Use the lookup table to see what these bits decode to:
        theNode = lookupBuf[(bitData >> (32 - HUFFHANDLELOOKUPBITS)) & (HUFFHANDLELOOKUPSIZE - 1)];

        // If we didn't reach a leaf node, proceed bit by bit.
        if (theNode->left >= 0) {
            int mask = 1L << (31 - HUFFHANDLELOOKUPBITS);

            // Decode one bit at a time and walk the tree.
            do {
                if (mask & bitData) {
                    theNode = nodes + theNode->right;
                } else {
                    theNode = nodes + theNode->left;
                }
                mask >>= 1;
            } while (theNode->left >= 0);
        }

        // Move the decoded symbol to theCode
        theCode = theNode->right;
        *dest++ = theCode;

        // Mark n bits as read.
        bitPosition += codeLengths[theCode];
    }
}

int CHuffProcessor::GetUncompressedLen(Ptr compressedData) {
    HuffDataHeader *inHeader;
    inHeader = (HuffDataHeader *)compressedData;
    dataCount = MEM_LONG(inHeader->decodedSize);
    return dataCount;
}

void CHuffProcessor::Uncompress(Ptr fromPtr, Ptr destPtr) {
    HuffDataHeader *inHeader;
    unsigned int *freqCounts;
    unsigned short *freqCountsShort;
    short i, j, k;
    HuffTreeNode *lookupMem[HUFFHANDLELOOKUPSIZE];

    LockThis();

    lookupBuf = lookupMem;
    inHeader = (HuffDataHeader *)fromPtr;
    dataCount = MEM_LONG(inHeader->decodedSize);
    outPointer = destPtr;

    if (destPtr && dataCount) {
        freqCounts = (unsigned int *)(sizeof(HuffDataHeader) + fromPtr);

        k = 0;

        freqCountsShort = (unsigned short *)freqCounts;

        for (i = 0; i < (NUMSYMBOLS >> 5); i++) {
            int flags;
            flags = MEM_LONG(inHeader->countBitmap[i]);

            for (j = 0; j < 32; j++) {
                if (dataCount > 65535) {
                    int temp = (flags < 0) ? *freqCounts++ : 0;
                    symbCounters[k++] = MEM_LONG(temp);
                } else {
                    short temp = (flags < 0) ? *freqCountsShort++ : 0;
                    symbCounters[k++] = MEM_SHORT(temp);
                }
                flags += flags;
            }
        }

        if (dataCount <= 65535) {
            freqCounts = (unsigned int *)freqCountsShort;
        }

        CreateLookupBuffer();

        if (singleSymbolData) {
            Ptr p = destPtr;
            while (dataCount--) {
                *p++ = theSingleSymbol;
            }
        } else {
            DecodeAll((unsigned char *)freqCounts, (unsigned char *)destPtr);
        }
    }

    UnlockThis();
}

int CHuffProcessor::GetCompressedLen(Ptr sourceData, int sourceLen) {
    int outSize;
    int outputBits = 0;
    short countDataSize;
    short nonZero = 0;
    int i;

    LockThis();

    dataCount = sourceLen;

    if (dataCount) {
        unsigned char *p;
        int *t;
        t = symbCounters;
        for (i = 0; i < NUMSYMBOLS; i++) {
            t[i] = 0;
        }

        i = sourceLen;
        p = (unsigned char *)sourceData;
        while (i--) {
            (t[*p++])++;
        }

        CreateSymbolTable();

        if (singleSymbolData) {
            outputBits = 0;
            nonZero = 1;
        } else {
            // Count how many bits are needed for output.
            for (i = 0; i < NUMSYMBOLS; i++) {
                if (symbCounters[i]) {
                    nonZero++;
                    outputBits += symbCounters[i] * codeLengths[i];
                }
            }
        }

        countDataSize = dataCount > 65535 ? sizeof(int) : sizeof(short);
        outCount = ((outputBits + 7) >> 3);
        outSize = nonZero * countDataSize + // Stored counts
                  sizeof(HuffDataHeader) + // Header size
                  outCount; // Number of encoded bytes
    } else {
        outSize = sizeof(int);
        outCount = 0;
    }

    UnlockThis();

    return outSize;
}

void CHuffProcessor::Compress(Ptr fromPtr, Ptr destPtr) {
    short i, j, k;
    int outputLen;
    int outputBits = 0;
    short nonZero = 0;
    HuffDataHeader *outHeader;

    LockThis();

    int count;
    int *zeroL;
    char *zeroC;

    zeroL = (int *)destPtr;
    count = outCount >> 2;
    while (count--) {
        *zeroL++ = 0;
    }

    zeroC = (char *)zeroL;
    count = outCount & 3;
    while (count--) {
        *zeroC++ = 0;
    }

    outHeader = (HuffDataHeader *)destPtr;
    outHeader->decodedSize = MEM_LONG(dataCount);

    if (dataCount) {
        unsigned char *inData, *endData;
        int outBits;
        int outLen;
        int bitOffset;
        unsigned int *countTable;
        unsigned short *countTableShort;

        k = 0;
        for (i = 0; i < (NUMSYMBOLS >> 5); i++) {
            int flags;
            for (j = 0; j < 32; j++) {
                flags += flags;
                flags |= (symbCounters[k++] != 0);
            }
            outHeader->countBitmap[i] = MEM_LONG(flags);
        }

        if (dataCount > 65535) {
            countTable = (unsigned int *)(sizeof(HuffDataHeader) + destPtr);
            for (i = 0; i < NUMSYMBOLS; i++) {
                if (symbCounters[i]) {
                    *countTable++ = MEM_LONG(symbCounters[i]);
                }
            }
        } else {
            countTableShort = (unsigned short *)(sizeof(HuffDataHeader) + destPtr);
            for (i = 0; i < NUMSYMBOLS; i++) {
                if (symbCounters[i]) {
                    *countTableShort++ = MEM_SHORT(symbCounters[i]);
                }
            }
            countTable = (unsigned int *)countTableShort;
        }

        if (!singleSymbolData) {
            bitOffset = 0;
            inData = (unsigned char *)fromPtr;
            endData = inData + dataCount;

            while (inData < endData) {
                int *tempBitP;

                outBits = codeStrings[*inData];
                outLen = codeLengths[*inData++];

                tempBitP = (int *)((char *)countTable + (bitOffset >> 3));
#ifdef INTEL_ARCH
                {
                    unsigned char *destP = (unsigned char *)tempBitP;
                    unsigned int orData;

                    orData = outBits << (32 - ((bitOffset & 7) + outLen));
                    *destP++ |= orData >> 24;
                    *destP++ |= orData >> 16;
                    *destP++ |= orData >> 8;
                    *destP |= orData;
                }
#else
                *tempBitP |= outBits << (32 - ((bitOffset & 7) + outLen));
#endif
                bitOffset += outLen;
            }
        }
    }

    UnlockThis();
}
