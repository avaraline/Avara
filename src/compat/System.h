#pragma once

#include "Types.h"

// (60 ticks/sec = 16.66667 msec) the old mac method TickCount() ticked at this rate
#define MSEC_TO_TICK_COUNT(msec) ((msec)*60/1000)
#define TICK_COUNT_TO_MSEC(tick) ((tick)*1000/60)
uint64_t TickCount();

void Enqueue(QElemPtr qElement, QHdrPtr qHeader);
OSErr Dequeue(QElemPtr qElement, QHdrPtr qHeader);
bool HasQueue(QHdrPtr qHeader);
