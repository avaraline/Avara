/*
    Copyright ©1992-1994, Juri Munkki
    All rights reserved.

    File: RamFiles.h
    Created: Friday, October 23, 1992, 15:57
    Modified: Thursday, October 13, 1994, 12:03
*/

#include "Memory.h"
#include "Types.h"

short IncreaseRamFile(Handle theFile, long *realSize, long *logicalSize, long increase);

short IncreaseByClump(Handle theFile, long *realSize, long *logicalSize, long increase, long clumpSize);

short OddIncreaseRamFile(Handle theFile, long *realSize, long *logicalSize, long increase);

short OddIncreaseByClump(Handle theFile, long *realSize, long *logicalSize, long increase, long clumpSize);

void PackRamFile(Handle theFile, long *realSize, long *logicalSize);
