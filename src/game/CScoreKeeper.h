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

class CAvaraGame;

typedef struct {
    long points;
    char team;
    char exitRank;
    short lives;
    short kills;
} PlayerScoreRecord;

typedef struct {
    PlayerScoreRecord player[kMaxAvaraPlayers];
    long teamPoints[kMaxTeamColors + 1];
} AvaraScoreRecord;

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

    virtual void IScoreKeeper(CAvaraGame *theGame);
    virtual void Dispose();

    // virtual	OSErr			OpenPlugIn(FSSpec *theFile);
    // virtual	void			ClosePlugIn();
    // virtual	void			CallPlugIn();

    virtual void EndScript();
    virtual void StartResume(Boolean didStart);
    virtual void PlayerIntros();
    virtual void StopPause(Boolean didPause);
    virtual void NetResultsUpdate();

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

    virtual void ZeroScores();
    virtual void ReceiveResults(AvaraScoreRecord *newResults);
    // virtual	void			DrawOnePlayerResults(short slot, Rect *toRect);
    // virtual	void			DrawResultsSummary(Rect *toRect);
    // virtual	void			RegularClick(EventRecord *theEvent, Rect *theRect);

    // virtual	void			FilterConsoleLine(StringPtr theString, short align);
};
