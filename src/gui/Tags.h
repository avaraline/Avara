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

class Tags {
    typedef std::map<std::string, std::map<std::string, std::set<std::string>>> TagMap;  // set->level->tags
    static TagMap _tags;
    static std::string TagsPath();
    static bool ReadTags();
    static bool WriteTags();
    static void TrimTags();

public:
    typedef std::pair<std::string, std::string> LevelURL;
    static const std::set<std::string>& GetTagsForLevel(LevelURL level);
    static std::string NormalizeTagName(std::string);
    static void AddTagToLevel(LevelURL level, std::string tag);
    static void DeleteTagFromLevel(LevelURL level, std::string tag);
    static std::set<LevelURL> GetLevelsMatchingTag(std::string tag);
};


#endif /* Tags_h */
