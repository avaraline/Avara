//
//  PlayerRatingsSimpleElo.cpp
//  Avara
//
//  Created by Tom on 5/12/23.
//

#include "PlayerRatingsSimpleElo.h"

#include <SDL2/SDL.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include <json.hpp>

#include "Debug.h"


PlayerRatingsSimpleElo::PlayerRatingsSimpleElo() {
    ReadRatings();
}

std::string PlayerRatingsSimpleElo::RatingsPath() {
    char *path = SDL_GetPrefPath("Avaraline", "Avara");
    std::string jsonPath = std::string(path) + "playerRatings.json";
    SDL_free(path);
    return jsonPath;
}

// this macro tells nholmann::json how to serialize/deserialize the Rating struct
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Rating, count, rating);
void PlayerRatingsSimpleElo::ReadRatings() {
    std::ifstream in(RatingsPath());
    if (!in.fail()) {
        nlohmann::json jrat;
        in >> jrat;
        ratingsMap = jrat.get<RatingsMap>();
    } else {
        ratingsMap = {};
    }
}

void PlayerRatingsSimpleElo::WriteRatings() {
    try {
        std::ostringstream oss;
        nlohmann::json jrat = ratingsMap;
        oss << std::setw(4) << jrat << std::endl;
        std::ofstream out(RatingsPath());
        out << oss.str();
    } catch (std::exception& e) {
        SDL_Log("ERROR WRITING TAGS FILE=%s", e.what());
    }
}

inline float kFactor(int count) {
    // for first 40 games kFactor is higher then it sticks at 24 after that
    return std::max(30.0, 60.0 - count/2);
}

// expected probability for player1 to beat player2
inline float expectation(float player1Rating, float player2Rating) {
    float exponent = (player2Rating - player1Rating) / 400.0;
    return 1.0 / (1.0 + pow(10, exponent));
}

// playerResults are assumed to be in order from worst to first
void PlayerRatingsSimpleElo::UpdateRatings(std::vector<PlayerResult> &playerResults) {
    // can't currently handle multiple players on the same team
    std::map<short, int> teamMap = {};
    for (auto player: playerResults) {
        if (++teamMap[player.teamId] > 1) {
            DBG_Log("elo", "more than 1 player on teamId = %d, not computing player ratings\n", player.teamId);
            return;
        }
    }

    std::map<std::string, Rating> adjustments = {};
    PlayerResult prevPlayer = {};
    for (auto result: playerResults) {
        auto currPlayer = result;
        std::string currId = currPlayer.playerId;
        adjustments[currId] = {0, 0.0};
        if (!prevPlayer.playerId.empty()) {
            // prevPlayer loses to currPlayer
            std::string prevId = prevPlayer.playerId;

            // 1.0 is result (win for currPlayer), expectation represents probability they should have won
            float delta = (1.0 - expectation(ratingsMap[currId].rating, ratingsMap[prevId].rating));

            // all adjustments are made all at once after exiting this loop
            adjustments[currId].rating += kFactor(ratingsMap[currId].count) * delta;
            if (delta > 0.05) {
                // don't increase count if prohibitive favorite (e.g. playing bots)
                adjustments[currId].count++;
            }
            adjustments[prevId].rating -= kFactor(ratingsMap[prevId].count) * delta;
            adjustments[prevId].count++;
        }
        prevPlayer = currPlayer;
    }
    // apply all the adjustments
    for (auto result: playerResults) {
        auto playerId = result.playerId;
        ratingsMap[playerId].rating += adjustments[playerId].rating;
        ratingsMap[playerId].count += adjustments[playerId].count > 0 ? 1 : 0;  // don't count game more than once
        ratingsMap[playerId].rating = std::roundf(ratingsMap[playerId].rating*4)/4;
        DBG_Log("elo", "rating[%d] for %s = %.0f (%+.1f)\n",
                ratingsMap[playerId].count, playerId.c_str(), ratingsMap[playerId].rating, adjustments[playerId].rating);
    }

    WriteRatings();
}


PlayerRatingsSimpleElo::RatingsMap PlayerRatingsSimpleElo::GetRatings(std::vector<std::string> playerIds) {
    RatingsMap ratings;
    for (auto player: playerIds) {
        if (ratingsMap.find(player) == ratingsMap.end()) {
            // don't add it to ratingsMap just yet, return the default initial rating for this player
            ratings[player] = {};
        } else {
            ratings[player] = ratingsMap[player];
        }
    }
    return ratings;
}

std::map<int, std::vector<std::string>> PlayerRatingsSimpleElo::SplitIntoTeams(std::vector<int> colors, std::vector<std::string> playerIds) {
    RatingsMap playerRatings = GetRatings(playerIds);
    // sort players by their ratings, best to worst
    std::sort(playerIds.begin(), playerIds.end(), [&](std::string const &lhs, std::string const &rhs) {
        return playerRatings[rhs].rating < playerRatings[lhs].rating;
    });

    // compute halfway between best and worst ratings
    float midRating = (playerRatings.begin()->second.rating + playerRatings.rbegin()->second.rating) / 2.0;

    // the keys will contain the sum of probabilities for all the players on each team
    std::multimap<float, std::vector<std::string>> teamMap;
    for (int i = 0; i < colors.size(); i++) {
        teamMap.insert({0, std::vector<std::string>()});
    }

    // now iterate over the players and distribute them to the team with the smallest sum of probabilities.
    // this will effectively put the top 1..N players in different teams, then player N+1 will go on team N,
    // player N+2 will go onto whichever team has the smallest total (likely team N-1) and so on.
    // (another possibile algorithm: multiply probabilities of player losing to midRating?)
    for (auto player: playerIds) {
        // the first team in the multimap will always contain the lowest sum because it sorts on the key where we put the sum,
        // so extract the node with the first team, add this player, update the sum, then put it back in the multimap.  rinse. repeat.
        auto node = teamMap.extract(teamMap.begin());
        node.mapped().push_back(player);
        node.key() = node.key() + expectation(playerRatings[player].rating, midRating);
        teamMap.insert(std::move(node));
    }

    // now put all the results into a map of color=>vector<name> ("strongest" team first)
    std::map<int, std::vector<std::string>> colorTeamMap;
    int colorIdx = 0;
    for (auto iter = teamMap.rbegin(); iter != teamMap.rend(); iter++, colorIdx++) {
        auto players = iter->second;
        if (Debug::IsEnabled("elo")) {
            std::ostringstream oss;
            for (auto player: players) {
                oss << player << "(" << playerRatings[player].rating-midRating << ") ";
            }
            DBG_Log("elo", "team %lu: sum = %.3f, [ %s]\n", colorTeamMap.size()+1, iter->first, oss.str().c_str());
        }
        colorTeamMap[colors[colorIdx]] = players;
    }

    return colorTeamMap;
}
