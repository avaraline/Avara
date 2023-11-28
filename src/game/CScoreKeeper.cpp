/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CScoreKeeper.c
    Created: Sunday, May 5, 1996, 13:11
    Modified: Saturday, August 17, 1996, 00:08
*/

#include "CScoreKeeper.h"

#include "CAbstractPlayer.h"
#include "CApplication.h"
#include "CAvaraApp.h"
#include "CAvaraGame.h"
#include "CPlayerManager.h"
//#include "CRosterWindow.h"
//#include "Aliases.h"
#include "CNetManager.h"
#include "CommDefs.h"
//#include "JAMUtil.h"
//#include "CInfoPanel.h"
#include "InfoMessages.h"
//#include "Palettes.h"
#include "Debug.h"

#define kScoringStringsList 135

enum { //	For personal pages:
    kResultsStr = 1,
    kPersonalScoresStr,
    kTeamScoresStr,
    kScoreRankStr,
    kExitRankStr,

    //	For summary page:
    kPlayerStr,
    kTeamStr,
    kPointsStr,
    kPRStr,
    kXRStr,
    kLivesStr,
    kKillsStr,

    kTeamNames
};

void CScoreKeeper::IScoreKeeper(CAvaraGame *theGame) {
    itsGame = theGame;
    // resRefNum = 0;
    // appResRefNum = CurResFile();

    iface.command = 0;
    iface.result = 0;
    iface.capabilities = 0;
    iface.plugIn = NULL;
    iface.maxPlayers = kMaxAvaraPlayers;
    iface.maxTeams = kMaxTeamColors;
    iface.frameTime = itsGame->frameTime;
    iface.frameNumber = -1;

    iface.resultsHandle = NULL; //GetResource('TEXT', 300);
    // DetachResource(iface.resultsHandle);

    iface.resultsChanged = false;
    // iface.resultsWindow = ((CAvaraApp *)gApplication)->theRosterWind->itsWindow;
    // SetRect(&iface.resultsRect, 0,0, 0,0);
    // iface.theEvent = NULL;
    iface.levelName = itsGame->loadedLevel;
    iface.levelName = itsGame->loadedDesigner;
    iface.levelName = itsGame->loadedInfo;
    iface.directory = itsGame->loadedFilename;
    iface.playerID = 0;
    iface.playerTeam = 0;
    iface.playerLives = 0;
    iface.playerName = 0;
    iface.winFrame = -1;

    iface.scorePoints = 0;
    iface.scoreEnergy = 0;
    iface.scoreReason = 0;
    iface.scoreTeam = 0;
    iface.scoreID = 0;

    iface.consoleLine = NULL;
    iface.consoleJustify = static_cast<long>(MsgAlignment::Center);

    entryPoint = NULL;

    ResetScores();

    netScores = localScores;

    playerRatings = std::make_unique<PlayerRatingsSimpleElo>();
}

void CScoreKeeper::Dispose() {
    DisposeHandle(iface.resultsHandle);
    iface.resultsHandle = NULL;

    CDirectObject::Dispose();
}

void CScoreKeeper::EndScript() {
    iface.command = ksiLevelLoaded;
    iface.levelName = itsGame->loadedLevel;
    iface.directory = itsGame->loadedSet;
    // this method called on level-load regardless if local player is playing or not, clear out the names here
    for (auto &name: playerNames) {
        name.clear();
    }
}

void CScoreKeeper::StartResume(Boolean didStart) {
    if (didStart) {
        iface.command = ksiLevelStarted;
        ResetScores();
    } else {
        iface.command = ksiLevelRestarted;
    }
}

void CScoreKeeper::ResetWins() {
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        localScores.player[i].serverWins = 0;
    }
}

void CScoreKeeper::PlayerLeft() {
    if(itsGame->itsNet->PlayerCount() == 1) {
        ResetWins();
    }
}

void CScoreKeeper::PlayerJoined() {
    NetResultsUpdate();
}

void CScoreKeeper::PlayerIntros() {
    short i;
    CNetManager *theNet = itsGame->itsNet.get();
    CPlayerManager *thePlayer;

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        thePlayer = theNet->playerTable[i].get();
        if (thePlayer->GetPlayer()) {
            localScores.player[i].lives = thePlayer->GetPlayer()->lives;
            localScores.player[i].team = thePlayer->GetPlayer()->teamColor;

            iface.playerLives = thePlayer->GetPlayer()->lives;
            iface.playerTeam = thePlayer->GetPlayer()->teamColor;
            iface.playerID = i;
            iface.command = ksiPlayerIntro;
            iface.winFrame = thePlayer->GetPlayer()->winFrame;
            iface.playerName = thePlayer->PlayerName();
        }
    }
}

void CScoreKeeper::StopPause(Boolean didPause) {
    NetResultsUpdate();

    iface.command = didPause ? ksiLevelPaused : ksiLevelEnded;
}

void CScoreKeeper::NetResultsUpdate() {
    CNetManager *theNet = itsGame->itsNet.get();
    short i, offset;
    CPlayerManager *thePlayer;

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        thePlayer = theNet->playerTable[i].get();
        offset = i * PLAYER_SCORE_FIELD_COUNT;
        if (thePlayer->GetPlayer()) {
            if (thePlayer->Presence() == kzSpectating) {
                localScores.player[i].lives = -2;
            } else {
                localScores.player[i].lives = thePlayer->GetPlayer()->lives;
            }

            scorePayload[offset] = htonl(localScores.player[i].points);
            scorePayload[offset + 1] = htonl(localScores.player[i].team);
            scorePayload[offset + 2] = htonl(localScores.player[i].exitRank);
            scorePayload[offset + 3] = htonl(localScores.player[i].lives);
            scorePayload[offset + 4] = htonl(localScores.player[i].kills);
            scorePayload[offset + 5] = htonl(localScores.player[i].serverWins);
        } else {
            scorePayload[offset] = 0;
            scorePayload[offset + 1] = 0;
            scorePayload[offset + 2] = 0;
            scorePayload[offset + 3] = htonl(-1);
            scorePayload[offset + 4] = 0;
            scorePayload[offset + 5] = 0;
        }
    }

    offset = PLAYER_SCORE_FIELD_COUNT * kMaxAvaraPlayers;
    for (i = 0; i <= kMaxTeamColors; i++) {
        scorePayload[offset + i] = htonl(localScores.teamPoints[i]);
    }

    theNet->itsCommManager->SendPacket(
        kdEveryone, kpResultsReport, 0, 0, 0, NET_SCORES_LEN, (Ptr)&scorePayload);
}

void CScoreKeeper::Score(ScoreInterfaceReasons reason,
    short team,
    short player,
    long points,
    Fixed energy,
    short hitTeam,
    short hitPlayer) {
    iface.command = ksiScore;
    iface.scoreReason = reason;
    iface.playerID = player;
    iface.playerTeam = team;

    ScoreInterfaceEvent event;
    event.player = itsGame->GetPlayerName(player);
    event.team = team;
    event.playerTarget = itsGame->GetPlayerName(hitPlayer);
    event.teamTarget = hitTeam;
    event.damage = points;

    if (player >= 0 && player <= kMaxAvaraPlayers) {
        iface.playerName = itsGame->itsNet->playerTable[player]->PlayerName();
        if (reason == ksiKillBonus && hitPlayer >= 0 && hitPlayer <= kMaxAvaraPlayers) {
            Str255 destStr;

            localScores.player[hitPlayer].lives--;
            if (hitTeam != team) {
                localScores.player[player].kills++;
            }
            itsGame->itsApp->ComposeParamLine(
                destStr, kmAKilledBPlayer, iface.playerName, itsGame->itsNet->playerTable[hitPlayer]->PlayerName());

            event.scoreType = ksiKillBonus;
            event.weaponUsed = itsGame->killReason;
            itsGame->AddScoreNotify(event);

            iface.consoleLine = destStr;
            iface.consoleJustify = static_cast<long>(MsgAlignment::Center);
        }
    } else {
        iface.playerName = NULL;
    }

    if (reason == ksiExitBonus) {
        iface.winFrame = itsGame->frameNumber;
    }

    if(iface.scoreReason == ksiHoldBall) {
        event.scoreType = ksiHoldBall;
        itsGame->AddScoreNotify(event);
        SDL_Log("CAvaraGame::Score Event: HOLD BALL!!");
    }

    if(iface.scoreReason == ksiScoreGoal) {
        event.scoreType = ksiScoreGoal;
        itsGame->AddScoreNotify(event);
        SDL_Log("CAvaraGame::Score Event: GOAL!!");
    }

    iface.scorePoints = points;
    iface.scoreEnergy = energy;
    iface.scoreTeam = hitTeam;
    iface.scoreID = hitPlayer;

    if (player >= 0 && player < kMaxAvaraPlayers) {
        localScores.player[player].points += points;
        if (reason == ksiExitBonus) {
            localScores.player[player].exitRank = 1;
            localScores.player[player].serverWins++;
            exitCount++;
        }
    }

    if (team >= 0 && team <= kMaxTeamColors) {
        localScores.teamPoints[team] += points;
    }

    SDL_Log("CAvaraGame::Score Event: player:%d, hit:%d, reason:%d,\n", iface.playerID, iface.scoreID, iface.scoreReason);
}

void CScoreKeeper::ResetScores() {
    CPlayerManager *thePlayer;
    short i;

    exitCount = 0;
    for (i = 0; i < kMaxAvaraPlayers; i++) {
        localScores.player[i].points = 0;
        localScores.player[i].team = 0;
        localScores.player[i].lives = -1;
        localScores.player[i].exitRank = 0;
        localScores.player[i].kills = 0;

        thePlayer = itsGame->itsNet->playerTable[i].get();
        if (thePlayer->GetPlayer() == NULL) {
            localScores.player[i].serverWins = 0;
        }
    }

    //clear server wins if server is empty
    if(itsGame->itsNet->PlayerCount() == 1) {
        localScores.player[0].serverWins = 0;
    }

    for (i = 0; i <= kMaxTeamColors; i++) {
        localScores.teamPoints[i] = 0;
    }
}

void CScoreKeeper::ReceiveResults(int32_t *newResults) {
    short i, offset;
    bool gameIsFinal = false;
    int sumPoints = 0;
    static int sumPointsCheck = 0;

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        offset = i * PLAYER_SCORE_FIELD_COUNT;
        if (playerNames[i].empty()) {
            // get the name as soon as available (and keep around in case they leave before game end)
            playerNames[i] = itsGame->itsNet->playerTable[i]->GetPlayerName();
        }
        netScores.player[i].points = ntohl(newResults[offset]);
        netScores.player[i].team = (char) ntohl(newResults[offset + 1]);
        netScores.player[i].exitRank = (char) ntohl(newResults[offset + 2]);
        netScores.player[i].lives = (int16_t) ntohl(newResults[offset + 3]);
        netScores.player[i].kills = (int16_t) ntohl(newResults[offset + 4]);
        netScores.player[i].serverWins = (int16_t) ntohl(newResults[offset + 5]);

        // if any exitRank is set, the game is over
        gameIsFinal = gameIsFinal || (netScores.player[i].exitRank > 0);
        sumPoints += netScores.player[i].points;

        //copy serverWins to localScores
        localScores.player[i].serverWins = (int16_t) ntohl(newResults[offset + 5]);

        DBG_Log("score", "player=%s, points=%6d, team=%d, exitRank=%d, lives=%d, kills=%d, wins=%d\n",
                playerNames[i].c_str(),
                netScores.player[i].points,
                netScores.player[i].team,
                netScores.player[i].exitRank,
                netScores.player[i].lives,
                netScores.player[i].kills,
                netScores.player[i].serverWins);
    }

    offset = PLAYER_SCORE_FIELD_COUNT * kMaxAvaraPlayers;
    for (i = 0; i <= kMaxTeamColors; i++) {
        netScores.teamPoints[i] = ntohl(newResults[offset + i]);
    }

    // the sumPointsCheck helps to ensure we only execute this code block once (maybe better if we added a gameId?)
    if (gameIsFinal && sumPoints != sumPointsCheck) {
        sumPointsCheck = sumPoints;
        std::vector<FinishRecord> rankings = DetermineFinishOrder();
        UpdatePlayerRatings(rankings);
    }
}

std::vector<FinishRecord> CScoreKeeper::DetermineFinishOrder() {
    std::vector<FinishRecord> finishOrder = {};
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        // deduce whether player is playing
        if (netScores.player[i].lives >= 0) {
            finishOrder.push_back({
                i,
                playerNames[i],
                netScores.player[i].team,
                netScores.player[i].lives,
                netScores.player[i].points});
        }
    }

    // sort results according to remaining lives and score... worst to first
    std::sort(finishOrder.begin(), finishOrder.end(), [](FinishRecord const &lhs, FinishRecord const &rhs) {
        // if only 1 person has lives > 0, they are the winner      (last man standing)
        // if multiple people have lives > 0, compare their scores  (e.g. timed levels with 99 lives)
        // all people with lives==0 compares scores
        return
            ((lhs.lives == 0 && (rhs.lives > 0 || lhs.score < rhs.score)) ||
             (lhs.lives > 0 && rhs.lives > 0 && lhs.score < rhs.score));
    });

    return finishOrder;
}

void CScoreKeeper::UpdatePlayerRatings(std::vector<FinishRecord> finishOrder) {
    // send the final results to playerRatings
    std::vector<PlayerResult> playerResults = {};
    for (auto finRec: finishOrder) {
        playerResults.push_back({finRec.playerName, finRec.teamColor});
    }
    // must pass in the results ordered last to first
    playerRatings->UpdateRatings(playerResults);
}
