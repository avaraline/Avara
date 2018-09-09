/*
    Copyright ©1994-1995, Juri Munkki
    All rights reserved.

    File: RayHit.h
    Created: Sunday, November 27, 1994, 07:38
    Modified: Sunday, September 10, 1995, 10:49
*/

#pragma once
class CSmartPart;

typedef struct {
    Vector origin;
    Vector direction;
    Fixed distance;
    CSmartPart *closestHit;
    short team;
    short playerId;
} RayHitRecord;
