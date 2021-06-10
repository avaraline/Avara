/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CDirectObject.c
    Created: Friday, March 11, 1994, 17:15
    Modified: Sunday, September 15, 1996, 21:38
*/

#include "CDirectObject.h"

void CDirectObject::Dispose() { delete this; }
