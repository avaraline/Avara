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

void PlayerRatingsSimpleElo::UpdateRatings(std::vector<PlayerResult> &playerResults) {
    // can't currently handle multiple players on the same team
    std::map<short, int> teamMap = {};
    for (auto player: playerResults) {
        if (++teamMap[player.teamColor] > 1) {
            DBG_Log("elo", "more than 1 player on teamColor = %d, not computing player ratings\n", player.teamColor);
            return;
        }
    }

    // sort results according to remaining lives and score... worst to first
    std::sort(playerResults.begin(), playerResults.end(), [](PlayerResult const &lhs, PlayerResult const &rhs) {
        // if only 1 person has lives > 0, they are the winner      (last man standing)
        // if multiple people have lives > 0, compare their scores  (e.g. timed levels with 99 lives)
        // all people with lives==0 compares scores
        return
            ((lhs.lives == 0 && (rhs.lives > 0 || lhs.score < rhs.score)) ||
             (lhs.lives > 0 && rhs.lives > 0 && lhs.score < rhs.score));
    });

    std::map<std::string, Rating> adjustments = {};
    PlayerResult prevPlayer = {};
    for (auto result: playerResults) {
        auto currPlayer = result;
        std::string currId = currPlayer.playerId;
        adjustments[currId] = {0, 0.0};
        if (!prevPlayer.playerId.empty()) {
            // prevPlayer loses to currPlayer
            std::string prevId = prevPlayer.playerId;
            // expected "probability" for current player to beat previous player...
            float exponent = (ratingsMap[prevId].rating - ratingsMap[currId].rating) / 400.0;
            float expectation = 1.0 / (1.0 + pow(10, exponent));
            // 1.0 is result (win for currPlayer), expectation represent probability they should have won
            float delta = (1.0 - expectation);

            // all adjustments are made all at once after exiting this loop
            adjustments[currId].rating += kFactor(ratingsMap[currId].count) * delta;
            adjustments[currId].count++;
            adjustments[prevId].rating -= kFactor(ratingsMap[prevId].count) * delta;
            adjustments[prevId].count++;
        }
        prevPlayer = currPlayer;
    }
    // apply all the adjustments
    for (auto result: playerResults) {
        auto playerId = result.playerId;
        ratingsMap[playerId].rating += adjustments[playerId].rating;
        ratingsMap[playerId].count += adjustments[playerId].count;
        ratingsMap[playerId].rating = std::roundf(ratingsMap[playerId].rating);
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
