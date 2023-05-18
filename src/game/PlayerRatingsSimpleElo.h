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
//    int lives;
//    long score;
};

struct Rating {
    int count = 0;       // games played
    float rating = 1500; // Elo rating
};

class PlayerRatingsSimpleElo {
    typedef std::map<std::string, Rating> RatingsMap;  // set->[level->[tags]]
    JSONify<RatingsMap> ratingsMap;

public:
    PlayerRatingsSimpleElo();

    RatingsMap GetRatings(std::vector<std::string> playerIds);
    void UpdateRatings(std::vector<PlayerResult> &playerResults);

    std::map<int, std::vector<std::string>> SplitIntoTeams(std::vector<int>, std::vector<std::string> playerIds);
};

#endif /* PlayerRatingsSimpleElo_h */
