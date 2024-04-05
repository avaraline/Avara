#pragma once
#include "CAbstractActor.h"

class CTeamColorAdjuster : public CAbstractActor {
public:
    long colorMask;

    void BeginScript();
    virtual CAbstractActor *EndScript();
};
