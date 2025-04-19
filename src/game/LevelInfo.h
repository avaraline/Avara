//
//  LevelInfo.h
//  Avara
//
//  Created by Tom on 4/20/25.
//

#pragma once

#ifndef LevelInfo_h
#define LevelInfo_h

#include <string>
#include <vector>

// basically a glorified struct with convenience methods, to access all the level meta data in one spot
class LevelInfo {
public:
    std::string setTag;
    std::string levelName;
    std::string levelTag;
    std::string designer = {};
    std::string information = {};

    LevelInfo(std::string sTag, std::string lName) : setTag(sTag), levelName(lName), levelTag("") {};
    LevelInfo(std::string sTag, std::string lName, std::string lTag) : setTag(sTag), levelName(lName), levelTag(lTag) {};

    std::string URL() const { return "file:///levels/" + setTag + "/" + levelTag; };

    std::vector<std::string> Authors() const;
    std::string TagsString() const;
};

#endif /* LevelInfo_h */
