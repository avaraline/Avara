#include "Memory.h"

#include <cstdlib>
#include <cstring>
#include <map>

std::map<Handle, Size> handleSizeMap;

Ptr NewPtr(Size logicalSize) {
    return (Ptr)(operator new(logicalSize));
}

void DisposePtr(Ptr p) {
    if (p) {
        delete p;
    }
}

Handle NewHandle(Size s) {
    Ptr p = NewPtr(s);
    Handle h = new Ptr(p);
    handleSizeMap[h] = s;
    return h;
}

Handle NewHandleClear(Size byteCount) {
    return NewHandle(byteCount);
}

void DisposeHandle(Handle hand) {
    if (hand) {
        handleSizeMap.erase(hand);
        delete *hand;
        delete hand;
    }
}

void HLock(Handle hand) {}

void HUnlock(Handle hand) {}

short HGetState(Handle hand) {
    return 0;
}

void HSetState(Handle hand, short state) {}

OSErr HandToHand(Handle *hand) {
    Size handSize = GetHandleSize(*hand);
    Handle newHandle = NewHandle(handSize);
    BlockMoveData(**hand, *newHandle, handSize);
    *hand = newHandle;
    return noErr;
}

OSErr HandAndHand(Handle hand1, Handle hand2) {
    Size oldSize = GetHandleSize(hand2);
    Size newSize = oldSize + GetHandleSize(hand1);
    SetHandleSize(hand2, newSize);
    BlockMoveData(*hand1, (*hand2) + oldSize, newSize - oldSize);
    return noErr;
}

OSErr PtrAndHand(const void *ptr1, Handle hand2, long size) {
    return noErr;
}

OSErr PtrToXHand(const void *srcPtr, Handle dstHndl, long size) {
    return noErr;
}

Size GetHandleSize(Handle hand) {
    return handleSizeMap[hand];
}

void SetHandleSize(Handle hand, Size newSize) {
    Size oldSize = handleSizeMap[hand];
    if (newSize > oldSize) {
        Ptr newData = NewPtr(newSize);
        // Wonder if this needs to zero out first?
        BlockMoveData(*hand, newData, std::min(oldSize, newSize));
        delete *hand;
        *hand = newData;
    }
    handleSizeMap[hand] = newSize;
}

void BlockMoveData(const void *srcPtr, void *destPtr, Size byteCount) {
    memmove(destPtr, srcPtr, byteCount);
}

OSErr MemError() {
    return noErr;
}
