/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CommDefs.h
    Created: Thursday, March 2, 1995, 22:27
    Modified: Sunday, September 1, 1996, 10:03
*/

#pragma once

#define LATENCY_DEBUG 0

/*
**	Communications packet commands:
*/
enum {
    kpPacketProtocolRefusal = -9,
    kpPacketProtocolLogin, //-8
    kpPacketProtocolLogout, //-7
    kpPacketProtocolReject, //-6
    kpPacketProtocolTOC, //-5
    kpPacketProtocolControl, //	-4 Used for misc control information.

    kpKillConnection, //-3
    kpDisconnect, //-2
    kpError, //-1

    kpLogin = 0,
    kpLoginAck, // 1

    kpNameQuery, // 2
    kpNameChange, // 3
    kpColorChange, // 4
    kpOrderChange, // 5
    kpRosterMessage, // 6
    kpPlayerStatusChange, // 7

    kpLoadLevel, // 8
    kpLevelLoaded, // 9
    kpLevelLoadErr, // 10
    kpStartLevel, // 11
    kpResumeLevel, // 12

    kpReadySynch, // 13
    kpUnavailableSynch, // 14
    kpUnavailableZero, // 15
    kpStartSynch, // 16

    kpKeyAndMouse, // 17
    kpKeyAndMouseRequest, // 18
    kpAskLater, // 19

    kpKillNet, // 20

    kpFastTrack, // 21
    kpRemoveMeFromGame, // 22

    kpGetMugShot, // 23
    kpMugShot, // 24
    kpZapMugShot, // 25

    kpServerOptions, // 26
    kpNewArrival, // 27
    kpKickClient, // 28

    kpLatencyVote, // 29

    kpResultsReport, // 30

    kpPing, // 31
    kpRealName, // 32

    kpLastCommandNumber
};

/*
**	Distributions:
*/

#define kdServerOnly 1
#define kdEveryone -1
