/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: PlayerConfig.h
    Created: Monday, January 8, 1996, 16:06
    Modified: Saturday, September 7, 1996, 19:42
*/

#pragma once
#include "ARGBColor.h"

typedef struct {
    short numGrenades;
    short numMissiles;
    short numBoosters;
    short hullType;
    short frameLatency;
    short frameTime;
    ARGBColor cockpitColor;
    ARGBColor gunColor;
} PlayerConfigRecord;

typedef struct {
    short hullBSP;
    short maxMissiles;
    short maxGrenades;
    short maxBoosters;

    Fixed mass;

    Fixed energyRatio;
    Fixed energyChargeRatio;

    Fixed shieldsRatio;
    Fixed shieldsChargeRatio;

    Fixed minShotRatio;
    Fixed maxShotRatio;
    Fixed shotChargeRatio;

    Fixed rideHeight;

    Fixed accelerationRatio;
    Fixed jumpPowerRatio;

} HullConfigRecord;
