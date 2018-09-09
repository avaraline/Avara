/*
    Copyright ©1994, Juri Munkki
    All rights reserved.

    File: BSPConfig.h
    Created: Sunday, December 11, 1994, 01:45
    Modified: Sunday, December 11, 1994, 16:03
*/

#pragma once

#ifndef __BSPCONFIG__
#define __BSPCONFIG__
#define FIXEDTOCOLORCACHE(n) ((n + (1 << 11)) >> 12)
#define COLORLEVELONE 16
#define COLORCACHESIZE 32
#endif
