//
//  Tags.h
//  Avara
//
//  Created by Tom Anderson on 4/30/23.
//

#ifndef Tags_h
#define Tags_h

#include <map>
#include <set>
#include <string>

#include "JSONify.h"

class Tags {
    typedef std::map<std::string, std::map<std::string, std::set<std::string>>> TagMap;  // set->level->tags
    static JSONify<TagMap> _tags;
    static void TrimTags();

public:
    typedef std::pair<std::string, std::string> LevelURL;
    static const std::set<std::string>& GetTagsForLevel(LevelURL level);
    static std::string TagsStringForLevel(LevelURL level);
    static std::string NormalizeTagName(std::string);
    static void AddTagToLevel(LevelURL level, std::string tag);
    static void DeleteTagFromLevel(LevelURL level, std::string tag);
    static std::set<LevelURL> GetLevelsMatchingTag(std::string tag);
};


#endif /* Tags_h */
