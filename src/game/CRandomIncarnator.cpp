
#include "CRandomIncarnator.h"
#include <iostream>
#include <typeinfo>

CRandomIncarnator::CRandomIncarnator(CAbstractActor* actorList) {
    Vector locMin, locMax;
    bool foundLocation = false;
    locMin[0] = locMin[1] = locMin[2] = std::numeric_limits<Fixed>::max();
    locMax[0] = locMax[1] = locMax[2] = std::numeric_limits<Fixed>::min();

    // get the extent of all the CPlacedActors
    for (CAbstractActor* actor = actorList;
         actor != nullptr;
         actor = actor->nextActor) {
        // check CPlacedActors that set UseForRandomIncarnator() which includes incarnators, goodies, ramps
        CPlacedActors* placedActor = dynamic_cast<CPlacedActors*>(actor);
        if (placedActor != nullptr && placedActor->UseForRandomIncarnator()) {
            locMin[0] = std::min(locMin[0], placedActor->location[0]);
            locMin[1] = std::min(locMin[1], placedActor->location[1]);
            locMin[2] = std::min(locMin[2], placedActor->location[2]);
            locMax[0] = std::max(locMax[0], placedActor->location[0]);
            locMax[1] = std::max(locMax[1], placedActor->location[1]);
            locMax[2] = std::max(locMax[2], placedActor->location[2]);
            foundLocation = true;
        }
    }

    if (!foundLocation) {
        locMin[0] = locMin[1] = locMin[2] = FIX(-9);
        locMax[0] = locMax[1] = locMax[2] = FIX(+9);
    }

    // place incarnator on circle defined by the extent box, facing in at a random point on the circle
    heading = FRandom();
    Fixed radius = std::max((locMax[0] - locMin[0]), (locMax[2] - locMin[2])) / 2;
    radius = std::max(radius, (Fixed)FIX(3));  // radius could be zero if only 1 CPlacedActors so guard that
    location[0] = (locMin[0] + locMax[0]) / 2 - FMul(radius, FOneSin(heading));
    location[2] = (locMin[2] + locMax[2]) / 2 - FMul(radius, FOneCos(heading));
    location[1] = locMax[1];

    SDL_Log("RANDOM INCARNATOR: location = %s, heading=%d\n", FormatVector(location, 3).c_str(), heading);
}
