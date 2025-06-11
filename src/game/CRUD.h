//
//  CRUD.h
//  Avara
//
//  Created by Tom on 4/19/25.
//

#pragma once

#ifndef CRUD_h
#define CRUD_h

#include <string>
#include <vector>

#include "CAvaraGame.h"
#include "LevelInfo.h"

#include "KeyFuncs.h"
#include "GameFilm.h"

class CRUD {
public:
    virtual ~CRUD() {};
    virtual void RecordGameStart(GamePointer &game) = 0;
    virtual void RecordFrames(GameFilm* gf) = 0;
    
    typedef std::vector<LevelInfo> RecentLevelsList;
    virtual RecentLevelsList GetRecentLevels(int limit = 50) = 0;
    
    virtual void FramesForGamePlayer(int gameId, int slot, FrameFunction* ff) = 0;
};

#endif /* CRUD_h */
