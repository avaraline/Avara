/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CScoreKeeper.h
    Created: Sunday, May 5, 1996, 13:08
    Modified: Friday, July 26, 1996, 05:20
*/

#pragma once
#include "AvaraDefines.h"
#include "AvaraScoreInterface.h"
#include "CDirectObject.h"
#include "PlayerRatingsSimpleElo.h"

#define PLAYER_SCORE_FIELD_COUNT 6
#define NET_SCORES_LEN ((kMaxAvaraPlayers * PLAYER_SCORE_FIELD_COUNT) + (kMaxTeamColors + 1)) * 4

class CAvaraGame;

// NOTE: Adding a field to this struct means you should update PLAYER_SCORE_FIELD_COUNT!
typedef struct {
    int32_t points;
    char team;
    char exitRank;
    int16_t lives;
    int16_t kills;
    int16_t serverWins; //number of wins on the current server. reset when player count == 1
} PlayerScoreRecord;

typedef struct {
    PlayerScoreRecord player[kMaxAvaraPlayers];
    int32_t teamPoints[kMaxTeamColors + 1];
} AvaraScoreRecord;

struct FinishRecord {
    int playerIndex;
    std::string playerName;
    int teamColor;
    int lives;
    long score;
};


class CScoreKeeper : public CDirectObject {
public:
    CAvaraGame *itsGame;
    ScoreInterfaceRecord iface;

    AvaraScoreRecord localScores;
    AvaraScoreRecord netScores;
    short exitCount;

    short resRefNum;
    short appResRefNum;
    ScoreInterfaceCallType *entryPoint;

    int32_t scorePayload[NET_SCORES_LEN / 4];

    // player ratings stuff
    std::unique_ptr<PlayerRatingsSimpleElo> playerRatings;

    virtual void IScoreKeeper(CAvaraGame *theGame);
    virtual void Dispose();

    // virtual	OSErr			OpenPlugIn(FSSpec *theFile);
    // virtual	void			ClosePlugIn();
    // virtual	void			CallPlugIn();

    virtual void EndScript();
    virtual void StartResume(Boolean didStart);
    virtual void ResetWins();
    virtual void PlayerIntros();
    virtual void StopPause(Boolean didPause);
    virtual void NetResultsUpdate();
    virtual void PlayerLeft();
    virtual void PlayerJoined();


    /*
    virtual	void			Click(EventRecord *theEvent, Rect *theRect);
    virtual	Handle			GetCustomText();
    virtual	void			DrawCustomWindow(Rect *theRect);
    virtual	void			HideShow(Boolean doHide);
    */
    virtual void Score(ScoreInterfaceReasons reason,
        short team,
        short player,
        long points,
        Fixed energy,
        short hitTeam,
        short hitPlayer);

    virtual void ResetScores();
    virtual void ReceiveResults(int32_t *newResults);
    // virtual	void			DrawOnePlayerResults(short slot, Rect *toRect);
    // virtual	void			DrawResultsSummary(Rect *toRect);
    // virtual	void			RegularClick(EventRecord *theEvent, Rect *theRect);

    // virtual	void			FilterConsoleLine(StringPtr theString, short align);

    std::vector<FinishRecord> DetermineFinishOrder();
    void UpdatePlayerRatings(std::vector<FinishRecord>);
};
