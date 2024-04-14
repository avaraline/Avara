/*
    Copyright ©1993-1995, Juri Munkki
    All rights reserved.

    File: CTagBase.h
    Created: Sunday, July 25, 1993, 07:48
    Modified: Wednesday, June 7, 1995, 08:23
*/

#pragma once

#include "CBaseObject.h"

#pragma warning(disable : 4200)
typedef struct {
    long tag;
    long dataLen;
    char data[];
} StoredTagDataBaseElement;

typedef struct {
    long offset; //	If negative, then link to next free master block.
    long tag;
    long hashLink; //	Index of next item in this hash list
    union {
        long link; //	Link to next free master pointer.
        short flags; //	A field for your own use. See instructions near method declarations.
    } lf;
} TagMasterBlock;

#pragma warning(disable : 4200)
typedef struct {
    long masterIndex; //	Index to owner master block. Negative, if block has been released.
    long dataLen; //	Length of data
    char data[]; //	the data.
} TagDataBlock;

#define TAGMASTERCLUMP 32L //	Allocate 32 masters at a time
#define TAGBASECLUMP 1024L //	Allocate at least 1KB of string space every time
#define TAGMAXWASTE 2048L //	Don't waste more than 2KB of string space before compacting
#define TAGBASEHASHSIZE 64

class CTagBase : public CBaseObject {
public:
    CTagBase() {}
    virtual ~CTagBase() {}

    long hashTable[TAGBASEHASHSIZE] = {0};

    long masterCount = 0; //	Amount of allocated master blocks.
    long realMasterSize = 0; //	Real memory allocated to master blocks.
    long logicalMasterSize = 0; //	Used memory for master blocks.
    Handle masterBlocksHandle = 0;
    TagMasterBlock **masterBlocks = {0}; //	Handle to master blocks.

    long firstFreeMaster = 0; //	Index of first free master block or -1 if none.

    long realTagBaseSize = 0; //	Real memory allocated for TagBase blocks.
    long logicalTagBaseSize = 0; //	Used memory for TagBase blocks (with garbage blocks).
    Handle tagBaseBlocks; //	Handle to string blocks.

    long wastedSpace = 0; //	Amount of garbage bytes in logicalStringSize.

    virtual void ITagBase(); //	Initialize to empty - always call after new
    virtual void Dispose(); //	To release all memory

    virtual void Lock();
    virtual void Unlock();

    virtual long GetNextTag(long *key);
    virtual void *GetNextPointer(long *key);
    virtual long LookupTag(long tag);
    virtual long WriteEntry(long tag, long len, void *theData);
    virtual short ReadEntry(long tag, long *len, void *theData);
    virtual Handle ReadIntoHandle(long tag);
    virtual long WriteDefault(long tag, long len, void *theData);

    virtual void *GetEntryPointer(long tag);
    virtual long GetEntrySize(long tag);

    virtual long StoreData(long tag, long len, void *theData);
    virtual short RecallData(long index, long *len, void *theData);
    virtual short ReleaseData(long index); //	Purge string and master block from heap.

    /*
    **	Some utility routines for storing and retrieving common data types:
    */
    virtual void WriteString(long tag, StringPtr theString);
    virtual void ReadString(long tag, StringPtr theString);

    virtual void WriteShort(long tag, short value);
    virtual short ReadShort(long tag, short defaultValue);

    virtual void WriteLong(long tag, long value);
    virtual long ReadLong(long tag, long defaultValue);

    virtual void WriteRect(long tag, Rect *value);
    virtual void ReadRect(long tag, Rect *value);

    virtual void WriteHandle(long tag, Handle contents);
    virtual Handle ReadHandle(long tag);
    virtual OSErr ReadOldHandle(long tag, Handle oldHandle);

    /*
    **	The flags field can be used to mark strings with a number. You can
    **	then use this number as you wish, although it was originally designed
    **	so that you can use any bit combination to signify a string that should
    **	be deleted. You can for instance mark all the strings you use with a
    **	certain value (let's say 1). If you then waste some of your reference
    **	indices (by overwriting them while deleting data from a table, for instance),
    **	you don't need to worry about deallocation, since after the operation,
    **	you can again change the flag value (let's say to 0) and then call
    **	ReleaseFlagged with the value you first stored (ReleaseFlagged(1,1)).
    **
    **	The result is that only those strings that remain in your table will
    **	be stored in the string heap. Neat, huh?
    */
    virtual short GetFlags(long index); //	Get the flags field.
    virtual short SetFlags(long index, short flags); //	Set the flags field.
    virtual void ReleaseFlagged(short flagMask, short flagValue); // read above.

    //	Internal routines:
    virtual short CreateMoreMasters(); //	Called when running out of master blocks
    virtual long AllocateSpace(long len, void *theData); //	Used to write allocate and copy data
    virtual void GarbageCollect(); //	Compact heap. You may use it, if necessary.
};
