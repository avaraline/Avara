/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CRC.h
    Created: Monday, June 5, 1995, 08:28
    Modified: Tuesday, June 6, 1995, 09:26
*/

#pragma once

#include "Memory.h"

unsigned short CRC16(unsigned char *pkt, long len);
unsigned short CRCHandle(Handle theHandle);
