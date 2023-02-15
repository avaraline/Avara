#include "System.h"
#include "Debug.h"

#include <SDL.h>
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

static std::map<QHdrPtr, std::deque<QElemPtr>> gQueues;

void Enqueue(QElemPtr qElement, QHdrPtr qHeader) {
    // SDL_Log("Enqueue(%x, %x)\n", qElement, qHeader);
    if (gQueues.count(qHeader) == 1) {
        std::deque<QElemPtr> &q = gQueues.at(qHeader);
        if (!q.empty()) {
            // Point the previous back of the queue to the new element
            q.back()->qLink = qElement;
        }
        q.push_back(qElement);
        qHeader->qHead = q.front();
        qHeader->qTail = qElement;
        // SDL_Log("  - new element (%x, front=%x)\n", qElement, q.front());
    } else {
        std::deque<QElemPtr> newQueue = {qElement};
        gQueues.insert(std::make_pair(qHeader, newQueue));
        qHeader->qHead = qHeader->qTail = qElement;
        // SDL_Log("  - inserting into gQueues with key %lx\n", qHeader);
        DBG_Log("q", "Enqueue: gQueues has %ld elements\n", gQueues.size());
    }
    // New element has no next link
    qElement->qLink = NULL;
}

OSErr Dequeue(QElemPtr qElement, QHdrPtr qHeader) {
    // SDL_Log("Dequeue(%x, %x)\n", qElement, qHeader);
    QElemPtr lastElement = NULL;
    if (gQueues.count(qHeader) == 1) {
        std::deque<QElemPtr> &q = gQueues.at(qHeader);
        if (q.empty()) {
            return qErr;
        }
        for (size_t i = 0; i < q.size(); i++) {
            QElemPtr curElement = q.at(i);
            if (curElement == qElement) {
                if (lastElement) {
                    lastElement->qLink = curElement->qLink;
                }
                // SDL_Log("  - old size = %d\n", q.size());
                q.erase(q.begin() + i);
                // SDL_Log("  - new size = %d\n", q.size());
                qHeader->qHead = q.empty() ? NULL : q.front();
                qHeader->qTail = q.empty() ? NULL : q.back();
                // SDL_Log("  - found it\n", qElement);
                return noErr;
            }
            lastElement = curElement;
        }
    }
    return qErr;
}

void DisposeQueue(QHdrPtr qHeader) {
    gQueues.erase(qHeader);
    DBG_Log("q", "DisposeQueues: gQueues now has %ld elements\n", gQueues.size());
}

size_t QueueSize() {
    return gQueues.size();
}
