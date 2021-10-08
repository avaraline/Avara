
#include "CRandomIncarnator.h"
#include <iostream>

CRandomIncarnator::CRandomIncarnator(CIncarnator* incarnatorList) {
    Vector locMin, locMax;
    locMin[0] = locMin[1] = locMin[2] = std::numeric_limits<Fixed>::max();
    locMax[0] = locMax[1] = locMax[2] = std::numeric_limits<Fixed>::min();

    // get the extent of all the Incarnators
    for (CIncarnator* incarnator = incarnatorList;
         incarnator != NULL;
         incarnator = incarnator->nextIncarnator) {
        locMin[0] = std::min(locMin[0], incarnator->location[0]);
        locMin[1] = std::min(locMin[1], incarnator->location[1]);
        locMin[2] = std::min(locMin[2], incarnator->location[2]);
        locMax[0] = std::max(locMax[0], incarnator->location[0]);
        locMax[1] = std::max(locMax[1], incarnator->location[1]);
        locMax[2] = std::max(locMax[2], incarnator->location[2]);
    }

    // place incarnator on circle defined by the extent box, facing in at a random point on the circle
    heading = FRandom();
    Fixed radius = std::max((locMax[0] - locMin[0]), (locMax[2] - locMin[2])) / 2;
    radius = std::max(radius, (Fixed)FIX(3));  // radius could be zero if only 1 incarnator so guard that
    location[0] = (locMin[0] + locMax[0]) / 2 - FMul(radius, FOneSin(heading));
    location[2] = (locMin[2] + locMax[2]) / 2 - FMul(radius, FOneCos(heading));
    location[1] = locMax[1];

    SDL_Log("RANDOM INCARNATOR: location = %s, heading=%d\n", FormatVector(location, 3).c_str(), heading);
}
