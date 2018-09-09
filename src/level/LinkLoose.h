/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: LinkLoose.h
    Created: Tuesday, April 4, 1995, 00:46
    Modified: Tuesday, April 4, 1995, 00:46
*/

#pragma once
#include "Types.h"

void *CreateObjectByIndex(short objectId);
void *CreateNamedObject(StringPtr theName);
void InitLinkLoose();
