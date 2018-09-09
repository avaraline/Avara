/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: GoodyRecord.h
    Created: Saturday, February 24, 1996, 08:50
    Modified: Tuesday, August 27, 1996, 04:16
*/

#pragma once

typedef struct {
    short grenades;
    short missiles;
    short boosters;
    short lives;
    Fixed shield;
    Fixed power;
    short boostTime;
} GoodyRecord;
