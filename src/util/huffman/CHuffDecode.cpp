#include "CHuffDecode.h"

OSErr CHuffDecode::Open() {
    OSErr theErr;
    theErr = CAbstractHuffPipe::Open();

    bytesInBuffer = 0;
    bitPosition = 0;
    outputPosition = 0;

    if (theErr == noErr) {
        bitBuffer = (unsigned char *)NewPtr(BITBUFFERSIZE + sizeof(long));
        theErr = MemError();

        outputBuffer = (unsigned char *)NewPtr(DECODEBUFFERSIZE);
        if (theErr == noErr) {
            theErr = MemError();
        }

        if (CHANGEOFPATTERNS) {
            CreateLookupBuffer();
        }
    } else {
        bitBuffer = 0;
        outputBuffer = 0;
    }

    return theErr;
}

/**
    Decoding the data is a bit more complicated than writing it, since we have to walk the tree to
    find what maps to what. We can, however, take a shortcut by reading HUFFLOOKUPBITS bits at a
    time. If the code we reach is shorter than this, all we need to do is back up a few bits. If we
    did not reach a leaf node with the lookup, we continue bit by bit until we reach a leaf.

    This routine creates the lookup table for HUFFLOOKUPBITS bits and it creates a table of lengths
    so that we can find out how many bits were needed for the last char.

    Recursion has been eliminated with a stack and the routine is a modified version of the one used
    to create the tables in the encoder object.

    The routine goes through the patterns in order starting with 000... and ending at 111...
    Because of this, the "holes" in the lookup table can be filled as we go. The last pointer
    written to the table has to be replicated until the end of the table.
 */
void CHuffDecode::CreateLookupBuffer() {
    short lastLookup;
    short walkerStack[33];
    short *stackPtr;
    short codeLen = 1;
    long codeString = 0;
    HuffTreeNode *theNode;
    HuffTreeNode *lastNode;

    theNode = BuildTree();

    stackPtr = walkerStack + 1;
    *stackPtr++ = theNode->right;
    theNode = &nodes[theNode->left];
    lastLookup = 0;

    do {
        if (codeLen == HUFFLOOKUPBITS || (theNode->left < 0 && codeLen < HUFFLOOKUPBITS)) {
            long codeIndex;
            short i;

            codeIndex = codeString << (HUFFLOOKUPBITS - codeLen);
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
    while (lastLookup < HUFFLOOKUPSIZE) {
        lookupBuf[lastLookup++] = lastNode;
    }
}

/**
    Decodes Huffman encoded data.
 */
OSErr CHuffDecode::PipeData(Ptr dataPtr, long len) {
    OSErr theErr = noErr;
    long bitsInBuffer;
    Boolean decodeOk;

    // Process "len" input bytes.
    while (len) {
        long byteRoom;
        long bitData;
        long *bitPointer;
        unsigned char *p;

        // Copy some data to temp buffer.
        byteRoom = BITBUFFERSIZE - bytesInBuffer;
        if (byteRoom > len) {
            byteRoom = len;
        }

        // Find out where to write the data.
        p = bitBuffer + bytesInBuffer;

        // Mark the bytes as moved.
        bytesInBuffer += byteRoom;
        len -= byteRoom;

        // Move the bytes.
        while (byteRoom--) {
            *p++ = (unsigned char)(*dataPtr++);
        }

        // Update bitsInBuffer to reflect the moved data.
        bitsInBuffer = bytesInBuffer << 3;

        // Decode data until we run out of bits.
        decodeOk = true;
        do {
            HuffTreeNode *theNode;
            short theCode;

            // Read in some bits into bitData.
            bitPointer = (long *)(bitBuffer + (bitPosition >> 3));
#ifdef INTEL_ARCH
            {
                unsigned char *charPointer = (unsigned char *)bitPointer;

                bitData = (((long)charPointer[0] << 24) | ((long)charPointer[1] << 16) | ((long)charPointer[2] << 8) |
                              (long)charPointer[3])
                          << (bitPosition & 7);
            }
#else
            bitData = (*bitPointer) << (bitPosition & 7);
#endif
            // Use the lookup table to see what these bits decode to:
            theNode = lookupBuf[(bitData >> (32 - HUFFLOOKUPBITS)) & (HUFFLOOKUPSIZE - 1)];

            // If we didn't reach a leaf node, proceed bit by bit.
            if (theNode->left >= 0) {
                long mask = 1L << (31 - HUFFLOOKUPBITS);

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

            // Mark n bits as read.
            bitPosition += codeLengths[theCode];

            // If we haven't read past the end of the buffer, decode was ok.
            if (bitPosition < bitsInBuffer) {
                outputBuffer[outputPosition++] = theCode;
                symbCounters[theCode]++;

                // Flush output buffer, if necessary.
                if (outputPosition >= DECODEBUFFERSIZE) {
                    if (outputStream) {
                        theErr = outputStream->PipeData((Ptr)outputBuffer, outputPosition);
                        if (theErr) {
                            decodeOk = false;
                        }
                    }
                    outputPosition = 0;
                }
                dataCount++;
                if (CHANGEOFPATTERNS) {
                    CreateLookupBuffer();
                }
            } else {
                // We tried to read past the end. Back up a little bit and fall out.
                decodeOk = false;
                bitPosition -= codeLengths[theCode];
            }
        } while (decodeOk);

        // Move remaining bits (max 24 unless there was an error) to the beginning of the buffer.
        bitPointer = (long *)(bitBuffer + (bitPosition >> 3));
        *((long *)bitBuffer) = *bitPointer;
        bytesInBuffer = (bitsInBuffer - bitPosition + 7) >> 3;
        bitPosition -= bitsInBuffer - (bytesInBuffer << 3);
    }

    return theErr;
}

/**
    Flush and close.
 */
OSErr CHuffDecode::Close() {
    OSErr theErr = noErr;
    OSErr lateErr;

    if (outputPosition) {
        if (outputStream) {
            theErr = outputStream->PipeData((Ptr)outputBuffer, outputPosition);
        }
    }

    lateErr = CAbstractHuffPipe::Close();

    if (theErr == noErr) {
        theErr = lateErr;
    }

    return theErr;
}

void CHuffDecode::Dispose() {
    if (bitBuffer) {
        DisposePtr((Ptr)bitBuffer);
    }

    if (outputBuffer) {
        DisposePtr((Ptr)outputBuffer);
    }

    CAbstractHuffPipe::Dispose();
}
