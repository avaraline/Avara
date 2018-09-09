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
#include "CCompactTagBase.h"
#include "CPlayerManager.h"
//#include "CRosterWindow.h"
//#include "Aliases.h"
#include "CNetManager.h"
#include "CommDefs.h"
//#include "JAMUtil.h"
//#include "CInfoPanel.h"
#include "InfoMessages.h"
//#include "Palettes.h"

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

static short ExitPointsTable[kMaxAvaraPlayers] = {10, 6, 3, 2, 1, 0};

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

    iface.resultsHandle = GetResource('TEXT', 300);
    // DetachResource(iface.resultsHandle);

    iface.resultsChanged = false;
    // iface.resultsWindow = ((CAvaraApp *)gApplication)->theRosterWind->itsWindow;
    // SetRect(&iface.resultsRect, 0,0, 0,0);
    // iface.theEvent = NULL;
    iface.levelName = itsGame->loadedLevel;
    iface.levelName = itsGame->loadedDesigner;
    iface.levelName = itsGame->loadedInfo;
    iface.directoryTag = 0;
    iface.levelTag = 0;
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
    iface.consoleJustify = centerAlign;

    entryPoint = NULL;

    ZeroScores();

    netScores = localScores;
}

void CScoreKeeper::Dispose() {
    DisposeHandle(iface.resultsHandle);
    iface.resultsHandle = NULL;

    CDirectObject::Dispose();
}

void CScoreKeeper::EndScript() {
    iface.command = ksiLevelLoaded;
    iface.levelName = itsGame->loadedLevel;
    iface.directoryTag = itsGame->loadedTag;
    iface.levelTag = itsGame->loadedDirectory;
}

void CScoreKeeper::StartResume(Boolean didStart) {
    if (didStart) {
        iface.command = ksiLevelStarted;
        ZeroScores();
    } else {
        iface.command = ksiLevelRestarted;
    }
}

void CScoreKeeper::PlayerIntros() {
    short i;
    CNetManager *theNet = itsGame->itsNet;
    CPlayerManager *thePlayer;

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        thePlayer = theNet->playerTable[i];
        if (thePlayer->itsPlayer) {
            localScores.player[i].lives = thePlayer->itsPlayer->lives;
            localScores.player[i].team = thePlayer->itsPlayer->teamColor;

            iface.playerLives = thePlayer->itsPlayer->lives;
            iface.playerTeam = thePlayer->itsPlayer->teamColor;
            iface.playerID = i;
            iface.command = ksiPlayerIntro;
            iface.winFrame = thePlayer->itsPlayer->winFrame;
            iface.playerName = thePlayer->playerName;
        }
    }
}

void CScoreKeeper::StopPause(Boolean didPause) {
    NetResultsUpdate();

    iface.command = didPause ? ksiLevelPaused : ksiLevelEnded;
}

void CScoreKeeper::NetResultsUpdate() {
    CNetManager *theNet = itsGame->itsNet;
    short i;
    CPlayerManager *thePlayer;

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        thePlayer = theNet->playerTable[i];
        if (thePlayer->itsPlayer) {
            localScores.player[i].lives = thePlayer->itsPlayer->lives;
        }
    }

    theNet->itsCommManager->SendPacket(
        kdEveryone, kpResultsReport, 0, 0, 0, sizeof(AvaraScoreRecord), (Ptr)&localScores);
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
    if (player >= 0 && player <= kMaxAvaraPlayers) {
        iface.playerName = itsGame->itsNet->playerTable[player]->playerName;
        if (reason == ksiKillBonus && hitPlayer >= 0 && hitPlayer <= kMaxAvaraPlayers) {
            Str255 destStr;

            if (hitTeam != team) {
                localScores.player[player].kills++;
            }
            itsGame->itsApp->ComposeParamLine(
                destStr, kmAKilledBPlayer, iface.playerName, itsGame->itsNet->playerTable[hitPlayer]->playerName);

            iface.consoleLine = destStr;
            iface.consoleJustify = centerAlign;
        }
    } else {
        iface.playerName = NULL;
    }

    if (reason == ksiExitBonus) {
        iface.winFrame = itsGame->frameNumber;
    }

    iface.scorePoints = points;
    iface.scoreEnergy = energy;
    iface.scoreTeam = hitTeam;
    iface.scoreID = hitPlayer;

    if (player >= 0 && player < kMaxAvaraPlayers) {
        localScores.player[player].points += points;
        if (reason == ksiExitBonus) {
            localScores.player[player].exitRank = ExitPointsTable[exitCount];
            exitCount++;
        }
    }

    if (team >= 0 && team <= kMaxTeamColors) {
        localScores.teamPoints[team] += points;
    }
}

void CScoreKeeper::ZeroScores() {
    short i;

    exitCount = 0;
    for (i = 0; i < kMaxAvaraPlayers; i++) {
        localScores.player[i].points = 0;
        localScores.player[i].team = 0;
        localScores.player[i].lives = -1;
        localScores.player[i].exitRank = 0;
        localScores.player[i].kills = 0;
    }

    for (i = 0; i <= kMaxTeamColors; i++) {
        localScores.teamPoints[i] = 0;
    }
}

void CScoreKeeper::ReceiveResults(AvaraScoreRecord *newResults) {
    short *a, *b;
    long len;

    len = sizeof(AvaraScoreRecord) >> 1;
    a = (short *)newResults;
    b = (short *)&netScores;

    while (len--) {
        if (*a++ != *b++) {
            break;
        }
    }

    if (len >= 0) { // CRosterWindow	*theRoster;

        netScores = *newResults;
        // theRoster = ((CAvaraApp *)gApplication)->theRosterWind;

        // theRoster->InvalidateArea(kScoreInformationBox, 0);
    }
}
