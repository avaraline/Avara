
#include "CRandomIncarnator.h"
#include <iostream>
#include <typeinfo>

CRandomIncarnator::CRandomIncarnator(const Vector &locMin, const Vector &locMax) {
    // place incarnator on circle defined by the extent box, facing in at a random point on the circle
    heading = FRandom();
    Fixed radius = std::max((locMax[0] - locMin[0]), (locMax[2] - locMin[2])) / 2;
    radius = std::max(radius, (Fixed)FIX(3));  // radius could be zero if only 1 CPlacedActors so guard that
    location[0] = (locMin[0] + locMax[0]) / 2 - FMul(radius, FOneSin(heading));
    location[2] = (locMin[2] + locMax[2]) / 2 - FMul(radius, FOneCos(heading));
    location[1] = locMax[1];

    SDL_Log("RANDOM INCARNATOR: location = %s, heading=%d\n", FormatVector(location, 3).c_str(), heading);
}
