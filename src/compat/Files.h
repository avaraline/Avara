#pragma once

#include "Types.h"

OSErr FSRead(short refNum, long *count, void *buffPtr);

OSErr FSWrite(short refNum, long *count, const void *buffPtr);
