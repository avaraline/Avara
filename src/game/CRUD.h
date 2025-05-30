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

#include "LevelInfo.h"

class CRUD {
public:
    virtual ~CRUD() {};
    virtual void RecordGameStart(int gameId, const LevelInfo &info) = 0;

    typedef std::vector<LevelInfo> RecentLevelsList;
    virtual RecentLevelsList GetRecentLevels(int limit = 50) = 0;
};

#endif /* CRUD_h */
