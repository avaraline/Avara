//
//  PlayerRatingsSimpleElo.cpp
//  Avara
//
//  Created by Tom on 5/12/23.
//

#include "PlayerRatingsSimpleElo.h"

#include "CAvaraGame.h"
#include "CAvaraApp.h"

#include <SDL2/SDL.h>
#include <fstream>
#include <iostream>
#include <sstream>

#include <json.hpp>

#include "Debug.h"

PlayerRatingsSimpleElo::PlayerRatingsSimpleElo() :
    ratingsMap("playerRatings")
{};

// this macro tells nholmann::json how to serialize/deserialize the Rating struct
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Rating, count, rating);

inline float kFactor(int ratingsCount, size_t playerCount) {
    // kFactor starts at 60/sqrt(N-1) then gradually reduces to 30/sqrt(N-1)
    // the numerator helps the rating get in the right ballpark quickly for new players
    // and the denomiator helps to reduce the effect of many players on the score
    return std::max(30.0, 60.0 - ratingsCount/2) / sqrt(playerCount - 1.0);
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
            adjustments[currId].rating += kFactor(ratingsMap[currId].count, playerResults.size()) * delta;
            if (delta > 0.05) {
                // don't increase count if prohibitive favorite (e.g. playing bots)
                adjustments[currId].count++;
            }
            adjustments[prevId].rating -= kFactor(ratingsMap[prevId].count, playerResults.size()) * delta;
            adjustments[prevId].count++;
        }
        prevPlayer = currPlayer;
    }
    // apply all the adjustments
    std::ostringstream oss;
    int outCount = 0;
    for (auto result: playerResults) {
        auto playerId = result.playerId;

        // if a player changed the CASE of their name (FreD to fREd)...
        if (playerId != ratingsMap.find(playerId)->first) {
            // pull the old node out of the map and reinsert with the new name
            auto rating = ratingsMap.extract(playerId);
            rating.key() = playerId;
            ratingsMap.insert(std::move(rating));
        }

        ratingsMap[playerId].rating += adjustments[playerId].rating;
        ratingsMap[playerId].count += adjustments[playerId].count > 0 ? 1 : 0;  // don't count game more than once
        ratingsMap[playerId].rating = std::roundf(ratingsMap[playerId].rating*4)/4;

        // output updated Elo ratings if "showElow" pref set to true
        if (Debug::IsEnabled("elo") || gApplication->Boolean(kShowElo)) {
            if (result == playerResults.front()) {
                gCurrentGame->itsApp->AddMessageLine("Updated Elo Ratings:");
            }
            // format results like this (stop displaying game count over 99):
            //           PlayerName = 1622(+22)
            //   LongPlayerName[22] = 1477( -6)
            std::string numPlayed(ratingsMap[playerId].count > 99 ?
                                  "" :
                                  "[" + std::to_string(ratingsMap[playerId].count) + "]");
            oss << std::right << std::setw(19)
                << (playerId + numPlayed + " = ")
                << std::fixed << std::setprecision(0)
                << std::setw(4) << ratingsMap[playerId].rating
                << "(" << std::showpos  << std::setw(3) << adjustments[playerId].rating << std::noshowpos << ")" ;
            // display 2 results on each message line
            if ((++outCount % 2) == 0 || result == playerResults.back()) {
                gCurrentGame->itsApp->AddMessageLine(oss.str());
                oss.str("");
            }
        }
    }

    // remove blank playerId from ratings (could happen if someone disonnects before game is recorded)
    ratingsMap.erase("");
    ratingsMap.Write();
}


// return player ratings ordered from highest to lowest
std::vector<std::pair<std::string,Rating>> PlayerRatingsSimpleElo::GetRatings(std::vector<std::string> playerIds) {
    std::vector<std::pair<std::string,Rating>> ratings;
    for (auto player: playerIds) {
        auto iter = ratingsMap.find(player);
        if (iter == ratingsMap.end()) {
            // don't add it to ratingsMap just yet, return the default initial rating (1500) for this named player
            ratings.push_back({player, {}});
        } else {
            // use the actual key/value in the map to match the case
            ratings.push_back(*iter);
        }
    }
    // sort players by their ratings, best to worst
    std::sort(ratings.begin(), ratings.end(), [&](std::pair<std::string,Rating> const &lhs, std::pair<std::string,Rating> const &rhs) {
        return rhs.second.rating < lhs.second.rating;
    });
    return ratings;
}

std::map<int, std::vector<std::string>> PlayerRatingsSimpleElo::SplitIntoTeams(std::vector<int> colors, std::vector<std::string> playerIds) {
    // ratings already sorted from best to worst
    std::vector<std::pair<std::string,Rating>> playerRatings = GetRatings(playerIds);

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
    for (auto [player, rating]: playerRatings) {
        // the first team in the multimap will always contain the lowest sum because it sorts on the key where we put the sum,
        // so extract the node with the first team, add this player, update the sum, then put it back in the multimap.  rinse. repeat.
        auto node = teamMap.extract(teamMap.begin());
        node.mapped().push_back(player);
        node.key() = node.key() + expectation(rating.rating, midRating);
        teamMap.insert(std::move(node));
    }

    // now put all the results into a map of color=>vector{names} ("strongest" team first)
    std::map<int, std::vector<std::string>> colorTeamMap;
    int colorIdx = 0;
    for (auto iter = teamMap.rbegin(); iter != teamMap.rend(); iter++, colorIdx++) {
        auto players = iter->second;
        if (Debug::IsEnabled("elo")) {
            std::ostringstream oss;
            for (auto player: players) {
                oss << player << "(" << ratingsMap[player].rating-midRating << ") ";
            }
            DBG_Log("elo", "team %zu: sum = %.3f, [ %s]\n", colorTeamMap.size()+1, iter->first, oss.str().c_str());
        }
        colorTeamMap[colors[colorIdx]] = players;
    }

    return colorTeamMap;
}

std::map<int, std::vector<std::string>> PlayerRatingsSimpleElo::SplitIntoBestTeams(std::vector<int> colors, std::vector<std::string> playerIds) {

    // ratings already sorted from best to worst
    std::vector<std::pair<std::string,Rating>> playerRatings = GetRatings(playerIds);
    // compute halfway between best and worst ratings
    float midRating = (playerRatings.begin()->second.rating + playerRatings.rbegin()->second.rating) / 2.0;

    // gather up the expectation values for each player
    std::map<std::string, float> expectations;
    for (auto [player, rating]: playerRatings) {
        expectations[player] = expectation(rating.rating, midRating);
    }

    // loop over the possible number of teams and compute expectation variance
    int maxTeams = int(std::min(colors.size(), playerIds.size()));
    int bestNumTeams = 2;
    float bestVariance = std::numeric_limits<float>::max();
    std::map<int, std::map<int, std::vector<std::string>>> splitCandidates {};
    for (int numTeams = 2; numTeams <= maxTeams; numTeams++) {
        // start with 2 teams and work up
        // slice out numTeams colors and split into that many teams
        std::vector<int> subColors(&colors[0], &colors[0]+numTeams);
        splitCandidates[numTeams] = SplitIntoTeams(subColors, playerIds);
        // compute the total expectation for each team and the overall variance of this combination of teams
        float sum = 0;
        float sumSqr = 0;
        for (auto [teamColor, teamPlayers]: splitCandidates[numTeams]) {
            float teamExp = 0;
            for (auto playerId: teamPlayers) {
                // sum of expectations for a team is used as the measure that we want to minimize variance on
                teamExp += expectations[playerId];
            }
            sum += teamExp;
            sumSqr += teamExp*teamExp;
        }
        float var = (sumSqr - (sum * sum) / numTeams) / (numTeams - 1);
        DBG_Log("elo", "%d-team split variance = %.3f\n", numTeams, var);
        if (var < bestVariance) {
            bestVariance = var;
            bestNumTeams = numTeams;
        }
    }

    return splitCandidates[bestNumTeams];
}
