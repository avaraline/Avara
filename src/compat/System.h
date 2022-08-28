#pragma once

#include "Types.h"

// (60 ticks/sec = 16.66667 msec) the old mac method TickCount() ticked at this rate
#define MSEC_PER_TICK_COUNT (1000.0/60)
uint64_t TickCount();

void Enqueue(QElemPtr qElement, QHdrPtr qHeader);
OSErr Dequeue(QElemPtr qElement, QHdrPtr qHeader);
