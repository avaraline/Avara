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
#include "CPlayerManager.h"

#include <SDL2/SDL.h>
#include <string>
#include <map>
#include <vector>
#include <memory>

#define NULLNETPACKETS (MINIMUMBUFFERRESERVE)
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
    std::unique_ptr<CCommManager> itsCommManager;
    CProtoControl *itsProtoControl;
    // CRosterWindow	*theRoster;

    short playerCount;
    CPlayerManager *playerTable[kMaxAvaraPlayers];

    char teamColors[kMaxAvaraPlayers];
    int8_t slotToPosition[kMaxAvaraPlayers];
    int8_t positionToSlot[kMaxAvaraPlayers];

    uint16_t activePlayersDistribution;
    uint16_t readyPlayers;
    uint16_t readyPlayersConsensus;
    uint16_t startPlayersDistribution;
    uint16_t totalDistribution;
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
    FrameNumber latencyVoteFrame;
    short maxRoundTripLatency;
    short addOneLatency;
    long subtractOneCheck;
    CPlayerManager *maxPlayer;
    Boolean latencyVoteOpen;
    std::map<int32_t, std::vector<int16_t>> fragmentMap;  // maps FRandSeed to list of players having that seed

    long lastLoginRefusal;

    long lastMsgTick;
    long firstMsgTick;
    //short msgBufferLen;
    //char msgBuffer[kMaxChatMessageBufferLen];
    std::vector<char> msgBuffer;

    ~CNetManager() { Dispose(); };
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
    virtual void RecordNameAndState(short slotId, StringPtr theName, LoadingState status, PresenceType presence);
    virtual void ValueChange(short slot, std::string attributeName, bool value);

    virtual void SwapPositions(short ind1, short ind2);
    virtual void PositionsChanged(char *p);

    virtual void FlushMessageBuffer();
    virtual void BufferMessage(size_t len, char *c);
    virtual void SendRosterMessage(size_t len, char *c);
    virtual void ReceiveRosterMessage(short slotId, short len, char *c);
    virtual void SendColorChange();
    virtual void ReceiveColorChange(char *newColors);

    virtual void DisconnectSome(short mask);
    virtual void HandleDisconnect(short slotId, short why);

    virtual void SendLoadLevel(std::string theSet, std::string theTag, int16_t originalSender = 0);
    virtual void ReceiveLoadLevel(short senderSlot, int16_t distribution, char *setAndTag, Fixed seed);
    virtual void LevelLoadStatus(short senderSlot, short crc, OSErr err, std::string theTag);

    virtual void SendPingCommand(int trips);
    virtual Boolean ResumeEnabled();
    virtual bool CanPlay();
    virtual void SendStartCommand(int16_t originalSender = 0);
    virtual void ReceiveStartCommand(short activeDistribution, int16_t senderSlot, int16_t originalSender);

    virtual void SendResumeCommand(int16_t originalSender = 0);
    virtual void ReceiveResumeCommand(short activeDistribution, short fromSlot, Fixed randomKey, int16_t originalSender);
    virtual void ReceiveReady(short slot, uint32_t readyPlayers, bool resendOurs);

    virtual void ReceivePlayerStatus(short slotId, LoadingState newStatus, PresenceType newPresence, Fixed randomKey, FrameNumber winFrame);
    virtual void ReceiveJSON(short slotId, Fixed randomKey, FrameNumber winFrame, std::string json);

    virtual short PlayerCount();

    virtual short SelfDistribution();
    virtual short AlivePlayersDistribution();
    virtual bool IAmAlive();

    //	Game loop methods:
    size_t SkipLostPackets(int16_t dist);
    virtual Boolean GatherPlayers(Boolean isFreshMission);
    virtual void UngatherPlayers();

    virtual void ResumeGame();
    virtual void FrameAction();
    virtual void HandleEvent(SDL_Event &event);
    virtual void AutoLatencyControl(FrameNumber frameNumber, Boolean didWait);
    virtual bool IsAutoLatencyEnabled();
    virtual bool IsFragmentCheckWindowOpen();
    virtual void ResetLatencyVote();
    virtual void ReceiveLatencyVote(int16_t sender, uint8_t p1, int16_t p2, int32_t p3);
    virtual std::string FragmentMapToString();

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
    virtual void NewArrival(short slot, PresenceType presence);

    virtual void ResultsReport(Ptr results);

    // virtual	void			BuildTrackerTags(CTracker *tracker);
    virtual void LoginRefused();
};
