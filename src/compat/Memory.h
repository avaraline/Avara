#pragma once

#include "Types.h"

Ptr NewPtr(Size);
void DisposePtr(Ptr);

Handle NewHandle(Size);
Handle NewHandleClear(Size);
void DisposeHandle(Handle);

#define DisposHandle(h) DisposeHandle(h)

void HLock(Handle);
void HUnlock(Handle);

short HGetState(Handle);
void HSetState(Handle, short);

OSErr HandToHand(Handle *);
OSErr HandAndHand(Handle, Handle);
OSErr PtrAndHand(const void *, Handle, long);

OSErr PtrToXHand(const void *srcPtr, Handle dstHndl, long size);

Size GetHandleSize(Handle);
void SetHandleSize(Handle, Size);

void BlockMoveData(const void *srcPtr, void *destPtr, Size byteCount);

OSErr MemError();
