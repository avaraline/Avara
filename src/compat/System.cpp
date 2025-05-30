#include "System.h"
#include "Debug.h"

#include <SDL2/SDL.h>
#include <deque>
#include <iterator>
#include <map>
#include <string>

uint64_t TickCount() {
    // see https://stackoverflow.com/a/35962360 ?
    // Approximate ms -> tick (1/60th of a second in old mac parlance)
    // return SDL_GetTicks() >> 4;
    return MSEC_TO_TICK_COUNT(SDL_GetTicks());
}

void InitQueue(QHdrPtr qHeader) {
    qHeader->qHead = NULL;
    qHeader->qTail = NULL;
    qHeader->qSize = 0;
}

void Enqueue(QElemPtr qElement, QHdrPtr qHeader) {
    if (!qElement || !qHeader) return;

    if (qHeader->qTail) {
        qHeader->qTail->qLink = qElement;
        qHeader->qTail = qElement;
    }
    else {
        qHeader->qHead = qHeader->qTail = qElement;
    }
    qElement->qLink = NULL;
    qHeader->qSize += 1;
}

OSErr Dequeue(QElemPtr qElement, QHdrPtr qHeader) {
    if (!qElement || !qHeader) return qErr;

    if (qElement == qHeader->qHead && qElement == qHeader->qTail) {
        // Special case for when there is only one item in the queue.
        qHeader->qHead = qHeader->qTail = NULL;
        qHeader->qSize = 0;
        return noErr;
    }

    QElemPtr remove = qHeader->qHead, last = NULL;
    while (remove) {
        if (remove == qElement) break;
        last = remove;
        remove = remove->qLink;
    }

    if (remove) {
        if (remove == qHeader->qHead) {
            // Removing the head; all we need to do is advance the head pointer.
            qHeader->qHead = qHeader->qHead->qLink;
        }
        else if (remove == qHeader->qTail) {
            // Removing the tail; the last element becomes the tail (and points to NULL).
            last->qLink = NULL;
            qHeader->qTail = last;
        }
        else {
            // Otherwise, the last element points to the element following the one removed.
            last->qLink = remove->qLink;
        }
        qHeader->qSize -= 1;
        return noErr;
    }

    return qErr;
}
