/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: PlayerConfig.h
    Created: Monday, January 8, 1996, 16:06
    Modified: Saturday, September 7, 1996, 19:42
*/

#pragma once
#include "ARGBColor.h"
#include "ColorManager.h"
#include "FastMat.h"

struct PlayerConfigRecord {
    // hull config
    uint16_t numGrenades {};
    uint16_t numMissiles {};
    uint16_t numBoosters {};
    uint16_t hullType {};
    // server config
    uint16_t frameLatencyMin {};
    uint16_t frameLatencyMax {};
    uint16_t frameTime {};
    uint16_t spawnOrder {};
    // hull colors
    ARGBColor hullColor { (*ColorManager::getMarkerColor(0)).WithA(0xff) };
    ARGBColor trimColor { (*ColorManager::getMarkerColor(1)).WithA(0xff) };
    ARGBColor cockpitColor { (*ColorManager::getMarkerColor(2)).WithA(0xff) };
    ARGBColor gunColor { (*ColorManager::getMarkerColor(3)).WithA(0xff) };
};

typedef struct {
    uint16_t hullBSP;
    uint16_t maxMissiles;
    uint16_t maxGrenades;
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
