//
//  PlayerRatingsSimpleElo.h
//  Avara
//
//  This class implements the Simple Multiplayer Elo ratings system as described here:
//      http://www.tckerrigan.com/Misc/Multiplayer_Elo/
//  As the name suggests, it's a simple extension of the basic Elo calculation that works
//  fairly well for multiplayer games.  The concept is simple; you sort the results then
//  do the classic Elo calculation for ONLY the consecutive pairs in the results list.
//  For example: if there are 3 players, then the Elo calculation is done for
//     1st vs 2nd, 2nd vs 3rd, but NOT 1st vs. 3rd.
//  There are much more complicated algorithms but this one is easy to implement and compares
//  favorably to others (MS TrueSkill is compared at the URL above).  Did I mention it's simple?
//
//  Created by Tom on 5/12/23.
//

#pragma once
#ifndef PlayerRatingsSimpleElo_h
#define PlayerRatingsSimpleElo_h

#include <string>
#include <map>
#include <vector>
#include <functional>

#include "Types.h"
#include "JSONify.h"

struct PlayerResult {
    std::string playerId;
    int teamId;
    bool isTied;   // score tied with previous player in the list.
//    int lives;
//    long score;
    bool operator==(const PlayerResult &rhs) {
        return playerId == rhs.playerId;
    }
};

struct Rating {
    int count = 0;       // games played
    float rating = 1500; // Elo rating
};

// use this to make map keys case-insenstive
struct CaseInsensitiveCompare {
    bool operator() (const std::string& lhs, const std::string& rhs) const {
        std::string s1(lhs);
        std::string s2(rhs);
        std::transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
        std::transform(s2.begin(), s2.end(), s2.begin(), ::tolower);
        return s1 < s2;
     }
};

class PlayerRatingsSimpleElo {
    typedef std::map<std::string, Rating, CaseInsensitiveCompare> RatingsMap;
    JSONify<RatingsMap> ratingsMap;

public:
    PlayerRatingsSimpleElo();

    std::vector<std::pair<std::string,Rating>> GetRatings(std::vector<std::string> playerIds);
    std::pair<std::string,Rating> GetRating(const std::string& playerId);
    void UpdateRatings(std::vector<PlayerResult> &playerResults);
    void UpdateRating(const std::string& playerId, float newRating, bool incrementCount = false);

    std::map<int, std::vector<std::string>> SplitIntoTeams(std::vector<int>, std::vector<std::string> playerIds);
    std::map<int, std::vector<std::string>> SplitIntoBestTeams(std::vector<int>, std::vector<std::string> playerIds);
};

#endif /* PlayerRatingsSimpleElo_h */
