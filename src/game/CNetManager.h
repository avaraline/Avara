/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CNetManager.h
    Created: Monday, May 15, 1995, 22:14
    Modified: Sunday, September 1, 1996, 19:31
*/

#pragma once
#include "AvaraDefines.h"
#include "CCommManager.h"
#include "CDirectObject.h"
#include "KeyFuncs.h"
//#include "LevelScoreRecord.h"
#include "PlayerConfig.h"

#include <SDL2/SDL.h>
#include <string>
#include <vector>

#define NULLNETPACKETS (32 + MINIMUMBUFFERRESERVE)
#define SERVERNETPACKETS (16 * 2 * kMaxAvaraPlayers)
#define CLIENTNETPACKETS (16 * kMaxAvaraPlayers)
#define TCPNETPACKETS (32 * kMaxAvaraPlayers)

#define kMaxChatMessageBufferLen 12

#define kMugShotWindowSize 8

#define kMaxLatencyTolerance 8

enum { kNullNet, kServerNet, kClientNet };

//	Server option bits
enum {
    kAllowLoadBit,
    kAllowResumeBit,
    kAllowLatencyBit,
    kAllowPositionBit,
    kAllowOwnColorBit,
    kFreeColorBit,
    kAllowKickBit,
    kUseAutoLatencyBit
};
void ServerOptionsDialog();

#define kAllowTeamControlBitMask ((1 << kFreeColorBit) + (1 << kAllowOwnColorBit))
class CAvaraGame;
class CCommManager;
class CPlayerManager;
class CProtoControl;
// class	CRosterWindow;
class CAbstractPlayer;
// class	CTracker;

/*
typedef union
{
    AddrBlock	block;
    long		value;
} BlockAndValue;
*/

class CNetManager : public CDirectObject {
public:
    CAvaraGame *itsGame;
    CCommManager *itsCommManager;
    CProtoControl *itsProtoControl;
    // CRosterWindow	*theRoster;

    short playerCount;
    CPlayerManager *playerTable[kMaxAvaraPlayers];

    char teamColors[kMaxAvaraPlayers];
    char slotToPosition[kMaxAvaraPlayers];
    char positionToSlot[kMaxAvaraPlayers];

    short activePlayersDistribution;
    short readyPlayers;
    short unavailablePlayers;
    short startPlayersDistribution;
    short totalDistribution;
    short netStatus;
    CDirectObject *netOwner;
    short deadOrDonePlayers;
    Boolean isConnected;
    Boolean isPlaying;
    Boolean startingGame;

    short serverOptions;
    short loaderSlot;

    PlayerConfigRecord config;

    // TaggedGameResult	gameResult;

    int32_t fragmentCheck;
    Boolean fragmentDetected;

    Boolean autoLatencyEnabled;
    long localLatencyVote;
    long autoLatencyVote;
    long autoLatencyVoteCount;
    long latencyVoteFrame;
    short maxRoundTripLatency;
    short addOneLatency;
    CPlayerManager *maxPlayer;
    Boolean latencyVoteOpen;

    long lastLoginRefusal;

    long lastMsgTick;
    long firstMsgTick;
    //short msgBufferLen;
    //char msgBuffer[kMaxChatMessageBufferLen];
    std::vector<char> msgBuffer;

    virtual void INetManager(CAvaraGame *theGame);
    virtual CPlayerManager* CreatePlayerManager(short);
    virtual void LevelReset();
    virtual void Dispose();
    virtual Boolean ConfirmNetChange();
    virtual void ChangeNet(short netKind, std::string address);
    virtual void ChangeNet(short netKind, std::string address, std::string password);

    virtual void ProcessQueue();
    virtual void SendRealName(short toSlot);
    virtual void RealNameReport(short slot, short regStatus, StringPtr realName);
    virtual void NameChange(StringPtr newName);
    virtual void RecordNameAndLocation(short slotId, StringPtr theName, short status, Point location);
    virtual void ValueChange(short slot, std::string attributeName, bool value);

    virtual void SwapPositions(short ind1, short ind2);
    virtual void PositionsChanged(char *p);

    virtual void FlushMessageBuffer();
    virtual void BufferMessage(short len, char *c);
    virtual void SendRosterMessage(short len, char *c);
    virtual void ReceiveRosterMessage(short slotId, short len, char *c);
    virtual void SendColorChange();
    virtual void ReceiveColorChange(char *newColors);

    virtual void DisconnectSome(short mask);
    virtual void HandleDisconnect(short slotId, short why);

    virtual void SendLoadLevel(std::string theSet, std::string theTag, int16_t originalSender = 0);
    virtual void ReceiveLoadLevel(short senderSlot, int16_t distribution, char *setAndTag, Fixed seed);
    virtual void LevelLoadStatus(short senderSlot, short crc, OSErr err, std::string theTag);

    virtual void SendPingCommand(int totalTrips = 0);
    virtual Boolean ResumeEnabled();
    virtual bool CanPlay();
    virtual void SendStartCommand(int16_t originalSender = 0);
    virtual void ReceiveStartCommand(short activeDistribution, int16_t senderSlot, int16_t originalSender);

    virtual void SendResumeCommand(int16_t originalSender = 0);
    virtual void ReceiveResumeCommand(short activeDistribution, short fromSlot, Fixed randomKey, int16_t originalSender);
    virtual void ReceivedUnavailable(short slot, short fromSlot);

    virtual void ReceivePlayerStatus(short slotId, short newStatus, Fixed randomKey, long winFrame);
    virtual void ReceiveJSON(short slotId, Fixed randomKey, long winFrame, std::string json);

    virtual short PlayerCount();

    virtual short SelfDistribution();
    virtual short AlivePlayersDistribution();
    virtual bool IAmAlive();

    //	Game loop methods:

    virtual Boolean GatherPlayers(Boolean isFreshMission);
    virtual void UngatherPlayers();

    virtual void ResumeGame();
    virtual void FrameAction();
    virtual void HandleEvent(SDL_Event &event);
    virtual void AutoLatencyControl(long frameNumber, Boolean didWait);
    virtual bool IsAutoLatencyEnabled();
    virtual bool IsFragmentCheckWindowOpen();
    virtual void ResetLatencyVote();
    virtual void ViewControl();
    virtual void AttachPlayers(CAbstractPlayer *thePlayer);

    virtual void StopGame(short newStatus);

    virtual void ConfigPlayer(short senderSlot, Ptr configData);
    virtual void DoConfig(short senderSlot);

    virtual void StoreMugShot(Handle mugPict);
    virtual void MugShotRequest(short sendTo, long sendFrom);
    virtual void ReceiveMugShot(short fromPlayer, short seqNumber, long totalLength, short dataLen, Ptr dataBuffer);
    virtual void ZapMugShot(short slot);

    virtual Boolean PermissionQuery(short reason, short index);
    virtual void ChangedServerOptions(short curOptions);
    virtual void NewArrival(short slot);

    virtual void ResultsReport(Ptr results);

    virtual void Beep();

    // virtual	void			BuildTrackerTags(CTracker *tracker);
    virtual void LoginRefused();
};
