#include "CHuffEncode.h"

/**
    The encoder writes by ORing to the output buffer, so whenever the buffer has to be reused, it
    should be cleared to all 0.
 */
void CHuffEncode::ClearBuffer() {
    short i;
    long *bp;
    bp = outputBuffer;
    for (i = 0; i <= ENCODEBUFFERSIZE; i++) {
        *bp++ = 0;
    }
}

OSErr CHuffEncode::Open() {
    short theErr;
    theErr = CAbstractHuffPipe::Open();
    if (theErr == noErr) {
        outputBuffer = (long *)NewPtr(sizeof(long) * (ENCODEBUFFERSIZE + 1));

        theErr = MemError();
        if (theErr == noErr) {
            bitsInBuffer = 0;
            ClearBuffer();
        }
    }
    return theErr;
}

/**
    Create a mapping from characters to bit patterns. Lengths are stored in a different array. Bit
    patterns may be up to 24 bits long (the limitation comes from the output code).

    A stack is used to eliminate recursion while walking the tree.
 */
void CHuffEncode::CreateSymbolTables() {
    short walkerStack[33];
    short *stackPtr;
    short codeLen = 1;
    long codeString = 0;
    HuffTreeNode *theNode;

    theNode = BuildTree();
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

/**
    Use the tables to encode data. Pipe the buffer to the next pipe segment when the internal buffer
    is full. Recreate the compression tree at defined intervals (see CAbstractHuffPipe.h).
 */
OSErr CHuffEncode::PipeData(Ptr dataPtr, long len) {
    unsigned char theChar;
    unsigned char *dataP;
    long *tempBitP;
    OSErr theErr = noErr;

    dataP = (unsigned char *)dataPtr;

    while (len--) {
        if (CHANGEOFPATTERNS) {
            CreateSymbolTables();
        }
        dataCount++;
        theChar = *dataP++;
        symbCounters[theChar]++;

        // Find the nearest possible address and write out the pattern.
        // Note that patterns may be only up to 25 bits due to this method of writing bits.
        tempBitP = (long *)((char *)outputBuffer + (bitsInBuffer >> 3));
#ifdef INTEL_ARCH
        {
            unsigned char *destP = (unsigned char *)tempBitP;
            unsigned long orData;

            orData = codeStrings[theChar] << (32 - ((bitsInBuffer & 7) + codeLengths[theChar]));
            *destP++ |= orData >> 24;
            *destP++ |= orData >> 16;
            *destP++ |= orData >> 8;
            *destP |= orData;
        }
#else
        *tempBitP |= codeStrings[theChar] << (32 - ((bitsInBuffer & 7) + codeLengths[theChar]));
#endif
        bitsInBuffer += codeLengths[theChar];

        // Is the buffer full?
        if (bitsInBuffer > 32 * ENCODEBUFFERSIZE) {
            long tempBits;

            if (outputStream) {
                theErr = outputStream->PipeData((Ptr)outputBuffer, ENCODEBUFFERSIZE * sizeof(long));
                if (theErr) {
                    len = 0;
                }
            }

            bitsInBuffer -= 32 * ENCODEBUFFERSIZE;
            tempBits = outputBuffer[ENCODEBUFFERSIZE];
            ClearBuffer();
            outputBuffer[0] = tempBits;
        }
    }

    return theErr;
}

/**
    Flush the internal buffer and then close the pipe.
 */
OSErr CHuffEncode::Close() {
    short extraBits;
    long *tempBitP;
    OSErr theErr = noErr;
    OSErr closeErr;

    extraBits = 7 & -bitsInBuffer;

    if (extraBits) {
        short theCode;
        for (theCode = 0; codeLengths[theCode] <= extraBits; theCode++)
            ;

        tempBitP = (long *)((char *)outputBuffer + (bitsInBuffer >> 3));
        *tempBitP |= codeStrings[theCode] << (32 - ((bitsInBuffer & 7) + codeLengths[theCode]));
    }

    if (outputStream && bitsInBuffer) {
        theErr = outputStream->PipeData((Ptr)outputBuffer, (bitsInBuffer + 7) >> 3);
    }

    closeErr = CAbstractHuffPipe::Close();
    if (theErr == noErr) {
        theErr = closeErr;
    }

    return theErr;
}

void CHuffEncode::Dispose() {
    if (outputBuffer) {
        DisposePtr((Ptr)outputBuffer);
    }
}
