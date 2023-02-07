/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CPlayerManager.h
    Created: Saturday, March 11, 1995, 05:33
    Modified: Sunday, September 15, 1996, 20:43
*/

#pragma once
#include "CDirectObject.h"
#include "KeyFuncs.h"
#include "PlayerConfig.h"

#include <SDL.h>
#include <unordered_map>
#include <deque>
#include <string>

enum {
    kLNotConnected,
    kLConnected,
    kLActive,
    kLLoaded,
    kLWaiting,
    kLTrying,
    kLMismatch,
    kLNotFound,
    kLPaused,
    kLNoVehicle,
    kLAway,
    kLReady,    // implies kLLoaded, i.e. Loaded & Ready to start

    kStringNorth,
    kStringSouth,
    kStringEast,
    kStringWest
};

#define FUNCTIONBUFFERS 32*8  // 8x to accommodate extra frames with high-FPS

class CAbstractPlayer;
class CAvaraGame;
class CNetManager;

#define kMaxMessageChars 127

static const char* lThing_utf8    = "\xC2\xAC ";     // ¬
static const char* checkMark_utf8 = "\xE2\x88\x9A";  // ✓
static const char* triangle_utf8  = "\xCE\x94";      // Δ
static const char* clearChat_utf8 = "\x1B";          // Fn-Del on Mac


class CPlayerManager {
protected:
    static CPlayerManager* theLocalPlayer;
public:
    static CPlayerManager* LocalPlayer() { return theLocalPlayer; };  // the current/local player

    virtual std::string GetChatString(int maxChars) = 0;
    virtual std::string GetChatLine() = 0;
    virtual CAbstractPlayer* GetPlayer() = 0;
    virtual void SetPlayer(CAbstractPlayer*) = 0;
    virtual short Slot() = 0;
    virtual void SetLocal() = 0;
    virtual void AbortRequest() = 0;
    virtual Boolean IsLocalPlayer() = 0;
    virtual bool CalculateIsLocalPlayer() = 0;

    virtual void GameKeyPress(char c) = 0;
    virtual FunctionTable *GetFunctions() = 0;
    virtual void DeadOrDone() = 0;
    virtual short Position() = 0;
    virtual Str255& PlayerName() = 0;
    virtual std::string GetPlayerName() = 0;
    virtual std::deque<char>& LineBuffer() = 0;
    virtual void Dispose() = 0;
    virtual void NetDisconnect() = 0;
    virtual short IsRegistered() = 0;
    virtual void IsRegistered(short) = 0;
    virtual Str255& PlayerRegName() = 0;
    virtual short LoadingStatus() = 0;
    virtual void SetPlayerStatus(short newStatus, long theWin) = 0;
    virtual bool IsAway() = 0;

    virtual void ChangeNameAndLocation(StringPtr theName, Point location) = 0;
    virtual void SetPosition(short pos) = 0;
    virtual void RosterKeyPress(unsigned char c) = 0;
    virtual void RosterMessageText(short len, const char *c) = 0;
    virtual short LevelCRC() = 0;
    virtual OSErr LevelErr() = 0;
    virtual std::string LevelTag() = 0;
    virtual void LevelCRC(short) = 0;
    virtual void LevelErr(OSErr) = 0;
    virtual void LevelTag(std::string) = 0;
    virtual void LoadStatusChange(short serverCRC, OSErr serverErr, std::string serverTag) = 0;
    virtual void ResumeGame() = 0;
    virtual uint32_t DoMouseControl(Point *deltaMouse, Boolean doCenter) = 0;
    virtual void HandleEvent(SDL_Event &event) = 0;
    virtual void SendFrame() = 0;
    virtual void ViewControl() = 0;
    virtual Fixed RandomKey() = 0;
    virtual void RandomKey(Fixed) = 0;
    virtual CAbstractPlayer *ChooseActor(CAbstractPlayer *actorList, short myTeamColor) = 0;
    virtual short GetStatusChar() = 0;
    virtual short GetMessageIndicator() = 0;

    virtual void StoreMugShot(Handle mugHandle) = 0;
    virtual Handle GetMugShot() = 0;

    virtual CAbstractPlayer *TakeAnyActor(CAbstractPlayer *actorList) = 0;
    virtual short PlayerColor() = 0;
    virtual Boolean IncarnateInAnyColor() = 0;
    virtual void ResendFrame(long theFrame, short requesterId, short commandCode) = 0;
    virtual void SpecialColorControl() = 0;
    virtual PlayerConfigRecord& TheConfiguration() = 0;
    virtual Handle MugPict() = 0;
    virtual void MugPict(Handle) = 0;
    virtual long MugSize() = 0;
    virtual long MugState() = 0;
    virtual void MugSize(long) = 0;
    virtual void MugState(long) = 0;
    virtual long WinFrame() = 0;
    virtual void ProtocolHandler(struct PacketInfo *thePacket) = 0;
    virtual void IncrementAskAgainTime(int) = 0;
    virtual void SetShowScoreboard(bool b) = 0;
    virtual bool GetShowScoreboard() = 0;
};

class CPlayerManagerImpl : public CDirectObject, public CPlayerManager {
private:

    CAbstractPlayer *itsPlayer;
    CAvaraGame *itsGame;
    // CRosterWindow	*theRoster;
    CNetManager *theNetManager;

    Fixed randomKey;

    FrameFunction frameFuncs[FUNCTIONBUFFERS];
    uint32_t oldHeld;
    uint8_t oldButton;
    short oldModifiers;
    long askAgainTime;

    // Track current frame events here
    uint32_t keysDown, keysUp, keysHeld, dupKeysHeld;
    short mouseX, mouseY;
    uint8_t buttonStatus;

    Handle mugPict;
    long mugSize;
    long mugState;

    Point globalLocation;
    Point oldMouse;
    unsigned long lastMouseControlTime;

    std::deque<char> inputBuffer;
    short bufferEnd;
    short bufferStart;
    Boolean keyboardActive;
    Boolean prevKeyboardActive;

    short position;
    short spaceCount;
    Str255 playerName;
    Str255 playerRegName;
    short isRegistered;
    unsigned char message[kMaxMessageChars + 1];
    std::deque<char> lineBuffer;

    FrameNumber winFrame;
    short loadingStatus;
    short slot;
    short playerColor;

    bool showScoreboard = false;

    Point mouseCenterPosition;
    Boolean isLocalPlayer;

    short levelCRC;
    OSErr levelErr;
    std::string levelTag;

    PlayerConfigRecord theConfiguration;

    std::unordered_map<SDL_Scancode, uint32_t> keyMap; // maps keyboard key to keyFunc

public:

    virtual void IPlayerManager(CAvaraGame *theGame, short id, CNetManager *aNetManager);

    virtual void SetPlayer(CAbstractPlayer *thePlayer);

    virtual bool CalculateIsLocalPlayer();
    virtual uint32_t GetKeyBits();
    virtual uint32_t DoMouseControl(Point *deltaMouse, Boolean doCenter);
    virtual void HandleEvent(SDL_Event &event);
    virtual void HandleKeyDown(uint32_t keyFunc);
    virtual void HandleKeyUp(uint32_t keyFunc);
    virtual void SendFrame();
    virtual void ResumeGame();

    virtual void ProtocolHandler(struct PacketInfo *thePacket);
    virtual void ViewControl();

    virtual FunctionTable *GetFunctions();
    virtual void SendResendRequest(short askCount);

    virtual void Dispose();

    virtual Boolean TestKeyPressed(short funcCode);

    // virtual	void			FlushMessageText(Boolean forceAll);
    virtual void RosterMessageText(short len, const char *c);
    virtual std::string GetChatString(int maxChars);
    virtual std::string GetChatLine();

    virtual void RosterKeyPress(unsigned char c);
    virtual void GameKeyPress(char c);

    virtual void NetDisconnect();
    virtual void ChangeNameAndLocation(StringPtr theName, Point location);
    virtual void SetPosition(short pos);
    virtual void SetPlayerStatus(short newStatus, long theWin);
    virtual void SetPlayerReady(bool isReady);
    virtual bool IsAway();
    virtual void ResendFrame(long theFrame, short requesterId, short commandCode);

    virtual void LoadStatusChange(short serverCRC, OSErr serverErr, std::string serverTag);

    virtual CAbstractPlayer *ChooseActor(CAbstractPlayer *actorList, short myTeamColor);
    virtual CAbstractPlayer *TakeAnyActor(CAbstractPlayer *actorList);
    virtual Boolean IncarnateInAnyColor();

    virtual void AbortRequest();

    virtual void DeadOrDone();

    virtual short GetStatusChar();
    virtual short GetMessageIndicator();

    virtual void StoreMugShot(Handle mugHandle);
    virtual Handle GetMugShot();

    // virtual	void			GetLoadingStatusString(StringPtr theStr);

    virtual void SpecialColorControl();
    virtual short Slot();
    virtual void SetLocal();
    virtual Boolean IsLocalPlayer();
    virtual short Position();
    virtual Str255& PlayerName();
    virtual std::string GetPlayerName();

    virtual std::deque<char>& LineBuffer();
    virtual CAbstractPlayer* GetPlayer();
    virtual short IsRegistered();
    virtual void IsRegistered(short);
    virtual Str255& PlayerRegName();
    virtual short LoadingStatus();
    virtual short LevelCRC();
    virtual OSErr LevelErr();
    virtual std::string LevelTag();
    virtual void LevelCRC(short);
    virtual void LevelErr(OSErr);
    virtual void LevelTag(std::string);
    virtual Fixed RandomKey();
    virtual void RandomKey(Fixed);
    virtual PlayerConfigRecord& TheConfiguration();
    virtual short PlayerColor();
    virtual Handle MugPict();
    virtual void MugPict(Handle);
    virtual long MugSize();
    virtual long MugState();
    virtual void MugSize(long);
    virtual void MugState(long);
    virtual long WinFrame();
    virtual void IncrementAskAgainTime(int);
    virtual void SetShowScoreboard(bool b);
    virtual bool GetShowScoreboard();
};
