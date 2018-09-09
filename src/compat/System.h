#pragma once

#include "Types.h"

uint64_t TickCount();

void Enqueue(QElemPtr qElement, QHdrPtr qHeader);
OSErr Dequeue(QElemPtr qElement, QHdrPtr qHeader);
