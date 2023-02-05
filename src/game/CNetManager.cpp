/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CNetManager.c
    Created: Monday, May 15, 1995, 22:25
    Modified: Tuesday, September 17, 1996, 03:21
*/
#include <algorithm> // std::max

#include "CNetManager.h"

#include "CApplication.h"
#include "CAvaraGame.h"
#include "CPlayerManager.h"
#include "CProtoControl.h"
#include "CUDPComm.h"
#include "CommDefs.h"
#include "CommandList.h"
//#include "CRosterWindow.h"
#include "CAbstractPlayer.h"
#include "CAvaraApp.h"
//#include "CInfoPanel.h"
#include "CScoreKeeper.h"
#include "InfoMessages.h"
#include "Messages.h"
//#include "CAsyncBeeper.h"
//#include "Ambrosia_Reg.h"
//#include "CTracker.h"
#include "Preferences.h"
#include "System.h"
#include "Beeper.h"

#include "Debug.h"
#include <string.h>

#define AUTOLATENCYPERIOD 3840  // msec (divisible by 64)
#define AUTOLATENCYDELAY  448   // msec (divisible by 64)
#define LOWERLATENCYCOUNT   2
#define HIGHERLATENCYCOUNT  12    // 4*(10/240) frames at fps=16ms, 1*10/60 frames at fps=64ms, works for all fps values
#define DECREASELATENCYPERIOD (itsGame->TimeToFrameCount(AUTOLATENCYPERIOD*8))  // 30.72 seconds

#if ROUTE_THRU_SERVER
    #define kAvaraNetVersion 666
#else
    #define kAvaraNetVersion 10
#endif

#define kMessageBufferMaxAge 90
#define kMessageBufferMinAge 30
#define kMessageWaitTime 12

extern Fixed FRandSeed;

void CNetManager::INetManager(CAvaraGame *theGame) {
    short i;

    itsGame = theGame;
    readyPlayers = 0;
    unavailablePlayers = 0;

    netStatus = kNullNet;
    itsCommManager = new CCommManager;
    itsCommManager->ICommManager(NULLNETPACKETS);

    itsProtoControl = new CProtoControl;
    itsProtoControl->IProtoControl(itsCommManager, itsGame);

    // theRoster = ((CAvaraApp *)gApplication)->theRosterWind;

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        playerTable[i] = CreatePlayerManager(i);
        slotToPosition[i] = i;
        positionToSlot[i] = i;
        teamColors[i] = i; //(i/3) * 2;
    }

    totalDistribution = 0;
    playerCount = 0;
    isConnected = false;
    isPlaying = false;
    startingGame = false;

    netOwner = NULL;
    loaderSlot = 0;

    serverOptions = theGame->itsApp->Number(kServerOptionsTag);

    lastMsgTick = TickCount();
    firstMsgTick = lastMsgTick;

    msgBuffer.clear();

    lastLoginRefusal = 0;
}

CPlayerManager* CNetManager::CreatePlayerManager(short id) {
    CPlayerManagerImpl *pm = new CPlayerManagerImpl;
    pm->IPlayerManager(itsGame, id, this);
    return pm;
}

void CNetManager::LevelReset() {
    playerCount = 0;
}
void CNetManager::Dispose() {
    short i;

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        playerTable[i]->Dispose();
    }

    itsProtoControl->Dispose();
    itsCommManager->Dispose();
}

Boolean CNetManager::ConfirmNetChange() {
    /*
    CCommander	*theActive;
    short		dItem;

    theActive = gApplication->BeginDialog();
    dItem = CautionAlert(402, NULL);
    gApplication->EndDialog(theActive);

    return dItem == 1;
    */
    return true;
}

void CNetManager::ChangeNet(short netKind, std::string address) {
    ChangeNet(netKind, address, "");
}

void CNetManager::ChangeNet(short netKind, std::string address, std::string password) {
    CCommManager *newManager = NULL;
    Boolean confirm = true;
    //CAvaraApp *theApp = itsGame->itsApp;

    if (netKind != netStatus || !isConnected) {
        if (netStatus != kNullNet || !isConnected) {
            if (isConnected) {
                confirm = ConfirmNetChange();
            }
        }

        if (confirm) {
            switch (netKind) {
                case kNullNet:
                    newManager = new CCommManager;
                    newManager->ICommManager(NULLNETPACKETS);
                    break;
                case kServerNet:
                    CUDPComm *theServer;
                    theServer = new CUDPComm;
                    theServer->IUDPComm(kMaxAvaraPlayers - 1, TCPNETPACKETS, kAvaraNetVersion, itsGame->frameTime);
                    theServer->StartServing();
                    newManager = theServer;
                    confirm = theServer->isConnected;
                    break;
                case kClientNet:
                    CUDPComm *theClient;
                    theClient = new CUDPComm;
                    theClient->IUDPComm(kMaxAvaraPlayers - 1, TCPNETPACKETS, kAvaraNetVersion, itsGame->frameTime);
                    theClient->Connect(address, password);
                    newManager = theClient;
                    confirm = theClient->isConnected;
                    break;
            }
        }

        if (confirm && newManager) {
            itsProtoControl->Detach();
            itsCommManager->Dispose();
            itsCommManager = newManager;
            itsProtoControl->Attach(itsCommManager);
            netStatus = netKind;
            isConnected = true;
            DisconnectSome(kdEveryone);

            totalDistribution = 0;
            itsCommManager->SendPacket(kdServerOnly, kpLogin, 0, 0, 0, 0L, NULL);
            if (itsGame->loadedTag.length() > 0) {
                itsGame->LevelReset(true);
                // theRoster->InvalidateArea(kBottomBox, 0);
            }
            itsGame->itsApp->BroadcastCommand(kNetChangedCmd);
        } else {
            if (newManager)
                newManager->Dispose();
        }
    } else {
        playerTable[itsCommManager->myId]->NetDisconnect();
        itsCommManager->SendPacket(kdServerOnly, kpLogin, 0, 0, 0, 0L, NULL);
        itsCommManager->SendPacket(kdEveryone, kpZapMugShot, 0, 0, 0, 0L, NULL);
        gApplication->BroadcastCommand(kNetChangedCmd);
    }
}

void CNetManager::ProcessQueue() {
    long curTicks;

    itsCommManager->ProcessQueue();

    if (msgBuffer.size() > 0) {
        curTicks = TickCount();

        if ((curTicks - firstMsgTick > kMessageBufferMaxAge) ||
            ((curTicks - firstMsgTick > kMessageBufferMinAge) && (curTicks - lastMsgTick > kMessageWaitTime))) {
            FlushMessageBuffer();
        }
    }

    /*
    if(gAsyncBeeper->beepChannel)
    {	gAsyncBeeper->BeepDoneCheck();
    }
    */
}

void CNetManager::SendRealName(short toSlot) {
    Str255 realName = "-Unnamed";
    char regStatus = 1;
    short distr;

    // Get this from somewhere, prefs maybe
    realName[0] = 7;
    if (itsCommManager->myId == toSlot) {
        distr = kdEveryone;
    } else {
        distr = 1 << toSlot;
    }
    itsCommManager->SendPacket(distr, kpRealName, regStatus, 0, 0, realName[0] + 1, (Ptr)realName);
}

void CNetManager::RealNameReport(short slotId, short regStatus, StringPtr realName) {
    CPlayerManager *thePlayer;

    thePlayer = playerTable[slotId];
    if (regStatus < 0) {
        thePlayer->IsRegistered(true);
        BlockMoveData(realName, thePlayer->PlayerRegName(), realName[0] + 1);
    } else {
        thePlayer->IsRegistered(false);
        GetIndString(thePlayer->PlayerRegName(), 133, 5 + regStatus);
    }

    // theRoster->InvalidateArea(kRealNameBox, slotToPosition[slotId]);
}

void CNetManager::NameChange(StringPtr newName) {
    short theStatus;
    /*
    MachineLocation		myLocation;
    Point				loc;

    ReadLocation(&myLocation);
    loc.h = myLocation.longitude >> 16;
    loc.v = myLocation.latitude >> 16;
    */
    theStatus = playerTable[itsCommManager->myId]->LoadingStatus();
    itsCommManager->SendPacket(kdEveryone, kpNameChange, 0, theStatus, 0, newName[0] + 1, (Ptr)newName);
}

void CNetManager::ValueChange(short slot, std::string attributeName, bool value) {
    std::string json = "{\"" + attributeName + "\":" + (value == true ? "true" : "false") + "}";
    char* c = const_cast<char*>(json.c_str());
    SDL_Log("sending json %s", c);
    itsCommManager->SendPacket(kdEveryone, kpJSON, slot, 0, 0, strlen(c), c);
}

void CNetManager::RecordNameAndLocation(short theId, StringPtr theName, short status, Point location) {
    if (theId >= 0 && theId < kMaxAvaraPlayers) {
        totalDistribution |= 1 << theId;
        if (status != 0)
            playerTable[theId]->SetPlayerStatus(status, -1);

        playerTable[theId]->ChangeNameAndLocation(theName, location);
    }
}

void CNetManager::SwapPositions(short ind1, short ind2) {
    char p[kMaxAvaraPlayers];
    char temp;
    short i;

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        p[i] = positionToSlot[i];
    }

    temp = p[ind1];
    p[ind1] = p[ind2];
    p[ind2] = temp;
    itsCommManager->SendPacket(kdEveryone, kpOrderChange, 0, 0, 0, kMaxAvaraPlayers, p);
}

void CNetManager::PositionsChanged(char *p) {
    short i;

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        if (p[i] != positionToSlot[i]) {
            positionToSlot[i] = p[i];
            playerTable[positionToSlot[i]]->SetPosition(i);
        }
    }

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        slotToPosition[positionToSlot[i]] = i;
    }
}

void CNetManager::FlushMessageBuffer() {
    if (msgBuffer.size() > 0) {
        char *msg = new char[msgBuffer.size()];
        std::copy(msgBuffer.begin(), msgBuffer.end(), msg);
        itsCommManager->SendPacket(~(1 << itsCommManager->myId), kpRosterMessage, 0, 0, 0, msgBuffer.size(), msg);
        delete [] msg;
        msgBuffer.clear();
    }
}

void CNetManager::BufferMessage(short len, char *c) {
    if (len) {
        lastMsgTick = TickCount();
        if (msgBuffer.size() == 0) {
            firstMsgTick = lastMsgTick;
        }

        while (len--) {
            msgBuffer.push_back(*c++);
            //msgBuffer[msgBufferLen++] = *c++;
        }

        if (msgBuffer.size() >= kMaxChatMessageBufferLen) {
            FlushMessageBuffer();
        }
    }
}

void CNetManager::SendRosterMessage(short len, char *c) {
    if (len > kMaxChatMessageBufferLen) {
        FlushMessageBuffer();
        itsCommManager->SendPacket(kdEveryone, kpRosterMessage, 0, 0, 0, len, c);
    } else {
        itsCommManager->SendPacket(1 << itsCommManager->myId, kpRosterMessage, 0, 0, 0, len, c);

        if (len + msgBuffer.size() > kMaxChatMessageBufferLen) {
            FlushMessageBuffer();
        }

        BufferMessage(len, c);
    }
}

void CNetManager::ReceiveRosterMessage(short slotId, short len, char *c) {
    CPlayerManager *thePlayer;

    if (slotId >= 0 && slotId < kMaxAvaraPlayers) {
        char *cp;
        short i;

        cp = c;
        i = len;
        thePlayer = playerTable[slotId];
        while (i--) {
            thePlayer->RosterKeyPress(*cp++);
        }

        // theRoster->InvalidateArea(kUserBoxBottomLine, slotToPosition[slotId]);

        thePlayer->RosterMessageText(len, c);
    }
}

void CNetManager::SendColorChange() {
    itsCommManager->SendPacket(kdEveryone, kpColorChange, 0, 0, 0, kMaxAvaraPlayers, teamColors);
}

void CNetManager::ReceiveColorChange(char *newColors) {
    short i;

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        if (newColors[i] != teamColors[i]) {
            teamColors[i] = newColors[i];
            // theRoster->InvalidateColorBox(i);
        }
    }
}

void CNetManager::DisconnectSome(short mask) {
    short i;

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        if ((1L << i) & mask) {
            playerTable[i]->NetDisconnect();
        }
    }

    totalDistribution &= ~mask;
}

void CNetManager::HandleDisconnect(short slotId, short why) {
    itsCommManager->DisconnectSlot(slotId);

    if (slotId == itsCommManager->myId) {
        isConnected = false;
        netOwner = NULL;
        DisconnectSome(kdEveryone);
        itsCommManager->SendPacket(1 << slotId, kpKillNet, 0, 0, 0, 0, 0);
    } else {
        DisconnectSome(1L << slotId);
        itsGame->scoreKeeper->PlayerLeft();
    }
}

void CNetManager::SendLoadLevel(std::string theSet, std::string levelTag, int16_t originalSender /* default = 0 */) {
    CAvaraApp *theApp;
    PacketInfo *aPacket;
    SDL_Log("SendLoadLevel(%s, %s, %d)\n", theSet.c_str(), levelTag.c_str(), originalSender);

    ProcessQueue();

    aPacket = itsCommManager->GetPacket();

    theApp = itsGame->itsApp;

    aPacket->command = kpLoadLevel;
    aPacket->p1 = 0;
    aPacket->p2 = originalSender;
    aPacket->p3 = FRandSeed;
    if (itsCommManager->myId == 0) {
        // to avoid multiple simultaneous loads, only the server sends the kpLoadLevel requests to everyone
        SDL_Log("  server sending to everyone\n");
        aPacket->distribution = kdEveryone;
    } else {
        // clients ask the server to forward the kpLoadLevel request to everyone
        SDL_Log("  client sending to server only\n");
        aPacket->distribution = kdServerOnly;
    }

    std::string setAndLevel = theSet + "/" + levelTag;

    aPacket->dataLen = setAndLevel.length() + 1;
    BlockMoveData(setAndLevel.c_str(), aPacket->dataBuffer, setAndLevel.length() + 1);

    /* TODO: implement
    theApp->GetDirectoryLocator((DirectoryLocator *)aPacket->dataBuffer);

    *(Fixed *)(aPacket->dataBuffer+sizeof(DirectoryLocator)) = TickCount();
    aPacket->dataLen = sizeof(DirectoryLocator)+sizeof(Fixed);
    aPacket->command = kpLoadLevel;
    aPacket->p1 = 0;
    aPacket->p2 = 0;
    aPacket->p3 = theLevelTag;
    aPacket->distribution = kdEveryone;
    */
    itsCommManager->WriteAndSignPacket(aPacket);
}

void CNetManager::ReceiveLoadLevel(short senderSlot, int16_t originalSender, char *theSetAndTag, Fixed seed) {
    CAvaraApp *theApp;
    OSErr iErr;
    short crc = 0;

    SDL_Log("ReceiveLoadLevel(%d, %d, %s, %d)\n", senderSlot, originalSender, theSetAndTag, seed);

    std::string setAndTag(theSetAndTag);
    auto pos = setAndTag.find("/");
    std::string set = setAndTag.substr(0, pos);
    std::string tag = setAndTag.substr(pos + 1, std::string::npos);

    if (senderSlot != 0) {
        // The server will forward clients' kpLoadLevel message to kdEveryone.
        // Everyone else waits until they get the message from the server.
        // This ensures that all kpLoadLevel requests are processed in the same order for everyone.
        if (itsCommManager->myId == 0) {
            SDL_Log("  server sending level load of %s on behalf of %s\n",
                    theSetAndTag, playerTable[senderSlot]->GetPlayerName().c_str());
            SendLoadLevel(set, tag, senderSlot);
        }
    } else if (isPlaying) {
        // save off the player that originally loaded this level to help loading side games
        loaderSlot = originalSender;
    } else {
        loaderSlot = originalSender;
        CPlayerManager *sendingPlayer = playerTable[originalSender];

        theApp = itsGame->itsApp;
        FRandSeed = seed;
        iErr = theApp->LoadLevel(set, tag, sendingPlayer);

        PacketInfo *aPacket;
        aPacket = itsCommManager->GetPacket();
        aPacket->distribution = kdEveryone;
        aPacket->p1 = 0;
        aPacket->p3 = 0;
        if (iErr) {
            aPacket->command = kpLevelLoadErr;
            aPacket->p2 = iErr;
            //itsCommManager->SendPacket(kdEveryone, kpLevelLoadErr, 0, iErr, 0, 0, 0);
        } else {
            aPacket->command = kpLevelLoaded;
            aPacket->p2 = crc;
            //itsCommManager->SendPacket(kdEveryone, kpLevelLoaded, 0, crc, 0, 0, 0);
        }

        aPacket->dataLen = tag.length() + 1;
        BlockMoveData(tag.c_str(), aPacket->dataBuffer, tag.length() + 1);
        itsCommManager->WriteAndSignPacket(aPacket);
    }

    /* TODO: implement
    DirectoryLocator *	theDirectory = theDir;

    if(!isPlaying)
    {	loaderSlot = senderSlot;
        theApp = (CAvaraApp *)gApplication;
        itsCommManager->SendPacket(kdEveryone, kpLevelLoadErr, 0, 1, theTag, 0,0);
        FRandSeed = *(Fixed *)(theDirectory + 1);
        FRandSeedBeta = FRandSeed;

        iErr = theApp->FetchLevel(theDirectory, theTag, &crc);

        if(iErr)
        {	itsCommManager->SendPacket(kdEveryone, kpLevelLoadErr, 0, iErr, theTag, 0,0);
        }
        else
        {	itsCommManager->SendPacket(kdEveryone, kpLevelLoaded, 0, crc, theTag, 0,0);
        }
    }
    */
}

void CNetManager::LevelLoadStatus(short senderSlot, short crc, OSErr err, std::string theTag) {
    size_t i;

    SDL_Log("LevelLoadStatus(senderSlot=%d, crc=%d, err=%d, tag=%s)\n", senderSlot, crc, err, theTag.c_str());
    SDL_Log("   loaderSlot = %d\n", loaderSlot);

    CPlayerManager *thePlayer;

    thePlayer = playerTable[senderSlot];
    thePlayer->LevelCRC(crc);
    thePlayer->LevelTag(theTag);
    thePlayer->LevelErr(err);

    if (senderSlot == loaderSlot) {
        for (i = 0; i < kMaxAvaraPlayers; i++) {
            playerTable[i]->LoadStatusChange(crc, err, theTag);

        }

        SDL_Log("CNetManager::LevelLoadStatus loop x%lu\n", i);
    } else {
        thePlayer->LoadStatusChange(
            playerTable[loaderSlot]->LevelCRC(), playerTable[loaderSlot]->LevelErr(), playerTable[loaderSlot]->LevelTag());
    }

    // reset startingGame flag whenever a new level is loaded
    startingGame = false;
}

Boolean CNetManager::GatherPlayers(Boolean isFreshMission) {
    short i;
    Boolean goAhead;
    long lastTime, debugTime;

    totalDistribution = 0;
    for (i = 0; i < kMaxAvaraPlayers; i++) {
        playerTable[i]->ResumeGame();
    }

    // itsCommManager->SendUrgentPacket(kdEveryone, kpFastTrack, 0, 0, fastTrack.addr.value, 0,0);
    SDL_Log("CNetManager::GatherPlayers activePlayersDistribution = 0x%02x\n", activePlayersDistribution);
    itsCommManager->SendUrgentPacket(activePlayersDistribution, kpReadySynch, 0, 0, 0, 0, 0);
    lastTime = debugTime = TickCount();
    do {
        if (TickCount() > debugTime) {
            SDL_Log("CNetManager::GatherPlayers loop\n");
            debugTime = TickCount() + MSEC_TO_TICK_COUNT(1000);
        }
        ProcessQueue();
        goAhead = (TickCount() - lastTime < 1800);
        // TODO: waiting dialog with cancel
    } while (
        ((activePlayersDistribution & (unavailablePlayers | readyPlayers)) ^ activePlayersDistribution) && goAhead);

    if (unavailablePlayers) {
        itsCommManager->SendPacket(unavailablePlayers, kpUnavailableZero, 0, 0, 0, 0, NULL);
        if (goAhead) {
            goAhead = isFreshMission;

            if (goAhead) {
                activePlayersDistribution &= ~unavailablePlayers;
                startPlayersDistribution = activePlayersDistribution;
            }
        }
    }

    readyPlayers = 0;
    unavailablePlayers = 0;
    startingGame = false;

    return goAhead;
}

void CNetManager::UngatherPlayers() {
    isPlaying = false;
}

void CNetManager::ResumeGame() {
    short i;
    Point tempPoint;
    CPlayerManager *thePlayerManager;
    long lastTime;
    Boolean notReady;

    SDL_Log("CNetManager::ResumeGame\n");
    config.frameLatency = gApplication->Get<float>(kLatencyToleranceTag) / itsGame->fpsScale;
    config.frameTime = itsGame->frameTime;
    config.hullType = gApplication->Number(kHullTypeTag);
    config.numGrenades = 6;
    config.numMissiles = 3;
    config.numBoosters = 3;

    // Pull grenade/missile/booster counts from HULL resource
    long hullRes = ReadLongVar(iFirstHull + config.hullType);
    Handle hullHandle = GetResource('HULL', hullRes);
    if (hullHandle) {
        HullConfigRecord hull = **(HullConfigRecord **)hullHandle;
        hull.maxMissiles = ntohs(hull.maxMissiles);
        hull.maxGrenades = ntohs(hull.maxGrenades);
        hull.maxBoosters = ntohs(hull.maxBoosters);
        config.numGrenades = hull.maxGrenades;
        config.numMissiles = hull.maxMissiles;
        config.numBoosters = hull.maxBoosters;
        ReleaseResource(hullHandle);
    }

    // This is what pulled the counts from CLevelListWind
    itsGame->itsApp->BroadcastCommand(kConfigurePlayerCmd);

    ResetLatencyVote();
    addOneLatency = 0;
    subtractOneCheck = 0;
    localLatencyVote = 0;
    latencyVoteFrame = itsGame->NextFrameForPeriod(AUTOLATENCYPERIOD);

    thePlayerManager = playerTable[itsCommManager->myId];
    if (thePlayerManager->GetPlayer()) {
        thePlayerManager->DoMouseControl(&tempPoint, !(itsGame->moJoOptions & kJoystickMode));

        PlayerConfigRecord copy;
        BlockMoveData(&config, &copy, sizeof(PlayerConfigRecord));
        copy.numGrenades = ntohs(config.numGrenades);
        copy.numMissiles = ntohs(config.numMissiles);
        copy.numBoosters = ntohs(config.numBoosters);
        copy.hullType = ntohs(config.hullType);
        copy.frameLatency = ntohs(config.frameLatency);
        copy.frameTime = ntohs(config.frameTime);

        itsCommManager->SendUrgentPacket(
            kdEveryone, kpStartSynch, 0, kLActive, FRandSeed, sizeof(PlayerConfigRecord), (Ptr)&copy);

        //	Synchronize players:
        lastTime = TickCount();
        notReady = true;
        do {
            short statusTest;

            statusTest = 0;
            for (i = 0; i < kMaxAvaraPlayers; i++) {
                if (playerTable[i]->LoadingStatus() == kLActive) {
                    statusTest |= 1 << i;
                }
            }

            if ((statusTest & activePlayersDistribution) == activePlayersDistribution) {
                notReady = false;
            }

            ProcessQueue();
            // Do we need this?
            // itsGame->itsApp->DoUpdate();

            // TODO: waiting for players dialog with cancel
        } while (TickCount() - lastTime < 1800 && notReady);

        for (i = 0; i < kMaxAvaraPlayers; i++) {
            if (activePlayersDistribution & (1 << i)) {
                DoConfig(i);
            }
        }

        ProcessQueue();
        if (notReady) { // SysBeep(10);
            SDL_Log("NOT READY\n");
            itsGame->statusRequest = kAbortStatus;
        }
    } else {
        itsCommManager->SendUrgentPacket(kdEveryone, kpRemoveMeFromGame, 0, 0, 0, 0, 0);
        itsGame->statusRequest = kNoVehicleStatus;
        // SysBeep(10);
    }
}

void CNetManager::HandleEvent(SDL_Event &event) {
    playerTable[itsCommManager->myId]->HandleEvent(event);
}

void CNetManager::FrameAction() {
    playerTable[itsCommManager->myId]->SendFrame();
    itsCommManager->ProcessQueue();

    if (!isConnected) {
        SDL_Log("NOT CONNECTED!!\n");
        itsGame->statusRequest = kAbortStatus;
    }
}

void CNetManager::AutoLatencyControl(long frameNumber, Boolean didWait) {
    if (didWait) {
        localLatencyVote++;
    }

    if (frameNumber >= latencyVoteFrame) {
        long autoLatencyPeriod = itsGame->TimeToFrameCount(AUTOLATENCYPERIOD);
        if ((frameNumber % autoLatencyPeriod) == 0) {
            long maxRoundLatency;
            short maxId = 0;

            // only compute latency numbers to/from players still playing
            if (IAmAlive()) {

                latencyVoteFrame = frameNumber;  // record the actual frame where the vote is initiated
                maxRoundLatency = itsCommManager->GetMaxRoundTrip(AlivePlayersDistribution(), &maxId);
                maxPlayer = playerTable[maxId];
                uint8_t llv = std::min(long(UINT8_MAX), localLatencyVote);  // just in case, p1 can only accept a max of 256

                itsCommManager->SendUrgentPacket(
                    activePlayersDistribution, kpLatencyVote, llv, maxRoundLatency, FRandSeed, 0, NULL);
                #if LATENCY_DEBUG
                    SDL_Log("*** fn=%ld SENDING kpLatencyVote to %hx, localLatencyVote=%ld, maxRoundLatency=%ld FRandSeed=%d\n",
                            frameNumber, activePlayersDistribution, localLatencyVote, maxRoundLatency, FRandSeed);
                #endif
            } else {
                // spectator just sends FRandSeed to self for fragmentation check
                itsCommManager->SendUrgentPacket(
                    SelfDistribution(), kpLatencyVote, 0, 0, FRandSeed, 0, NULL);
            }
            localLatencyVote = 0;
            latencyVoteOpen = true;
        } else if ((frameNumber % autoLatencyPeriod) == itsGame->TimeToFrameCount(AUTOLATENCYDELAY) && latencyVoteOpen) {

            if (fragmentDetected) {
                itsGame->itsApp->MessageLine(kmFragmentAlert, MsgAlignment::Center);
                itsGame->itsApp->AddMessageLine(FragmentMapToString(), MsgAlignment::Left, MsgCategory::Error);
            }

            if (IsAutoLatencyEnabled() && autoLatencyVoteCount) {
                autoLatencyVote /= autoLatencyVoteCount;
                DBG_Log("lt", "====autoLatencyVote = %ld\n", autoLatencyVote);
                // if, on average, players had to wait more than some percent of frames during this latency vote period,
                // then add 1 frame to the LT calculation
                if (autoLatencyVote > HIGHERLATENCYCOUNT) {
                    addOneLatency++;
                    // don't let it go above 1.0 LT
                    addOneLatency = std::min(short(1.0/itsGame->fpsScale), addOneLatency);
                    DBG_Log("lt", "  ++addOneLatency increased = %hd\n", addOneLatency);
                    subtractOneCheck = frameNumber + DECREASELATENCYPERIOD;
                } else if (autoLatencyVote > LOWERLATENCYCOUNT) {
                    // vote too high to reduce addOneLatency, push subtractOneCheck forward
                    DBG_Log("lt", "   >addOneLatency keeping = %hd\n", addOneLatency);
                    subtractOneCheck = frameNumber + DECREASELATENCYPERIOD;
                } else if (addOneLatency > 0 && frameNumber >= subtractOneCheck) {
                    // if no significant waiting seen for 8 CONSECUTIVE autoLatency votes, about 30 seconds, let it creep back down 1 fps frame
                    addOneLatency--;
                    DBG_Log("lt", "  --addOneLatency decreased = %hd\n", addOneLatency);
                    subtractOneCheck = frameNumber + DECREASELATENCYPERIOD;
                }

                // Usually maxFrameLatency is determined primarily by maxRoundTripLatency...
                // but addOneLatency helps account for deficiencies in the calculation by measuring how often clients had to wait too long for packets to arrive
                short maxFrameLatency = addOneLatency + itsGame->RoundTripToFrameLatency(maxRoundTripLatency);

                SDL_Log("*** fn=%ld RTT=%d, Classic LT=%.2lf, add=%lf --> FL=%d\n",
                        frameNumber, maxRoundTripLatency,
                        (maxRoundTripLatency) / (2.0*CLASSICFRAMETIME), addOneLatency*itsGame->fpsScale, maxFrameLatency);

                itsGame->SetFrameLatency(maxFrameLatency, 2, maxPlayer);
            }

            ResetLatencyVote();
            latencyVoteFrame = itsGame->NextFrameForPeriod(AUTOLATENCYPERIOD, latencyVoteFrame);
            // SDL_Log("*** next latencyVoteFrame = %ld\n", latencyVoteFrame);
        }
    }
}

bool CNetManager::IsAutoLatencyEnabled() {
    return (serverOptions & (1 << kUseAutoLatencyBit));
}

bool CNetManager::IsFragmentCheckWindowOpen() {
    // Start considering fragmentation checks a little before the next latencyVoteFrame (because other clients might
    // send votes early, like mail-in votes). This check is used to prevent a possible fragmentation false-positive
    // when a vote arrives AFTER the previous vote count has been processed and could be misinterpretted as a frag.
    return (itsGame->frameNumber > latencyVoteFrame - itsGame->TimeToFrameCount(AUTOLATENCYDELAY));
}

// reset all the variables that keep track of the latency vote and fragment check
void CNetManager::ResetLatencyVote() {
    fragmentCheck = 0;
    fragmentDetected = false;
    autoLatencyVote = 0;
    autoLatencyVoteCount = 0;
    maxRoundTripLatency = 0;
    maxPlayer = nullptr;
    latencyVoteOpen = false;
    fragmentMap.clear();
}

void CNetManager::ReceiveLatencyVote(int16_t sender,
                                     uint8_t p1,        // localLatencyVote (uint8_t because it can go as high as 3840/16=240)
                                     int16_t p2,        // maxRoundLatency
                                     int32_t p3) {      // FRandSeed

    DBG_Log("lt", "CNetManager::ReceiveLatencyVote(%d, %d, %hd, %d)\n", sender, p1, p2, p3);
    autoLatencyVoteCount++;
    autoLatencyVote += p1;

    maxRoundTripLatency = std::max(maxRoundTripLatency, p2);

    playerTable[sender]->RandomKey(p3);

    // to be considered for fragmentation, packet must be received in the voting time window
    if (IsFragmentCheckWindowOpen()) {
        // keep track of who is which FRandSeed. Normally everyone on the same value, but with frags this identifies who is different
        if (fragmentMap.find(p3) == fragmentMap.end()) {
            fragmentMap[p3] = std::vector<int16_t>();
        }
        fragmentMap[p3].push_back(sender);

        if (fragmentCheck == 0) {
            // the first vote received dictates what the fragmentCheck value is, not necessarily the current player
            fragmentDetected = false;
            fragmentCheck = p3;
            // SDL_Log("autoLatencyVoteCount = 1, setting fragmentCheck = %d, in frameNumber %u", fragmentCheck, itsGame->frameNumber);
        } else {
            // any votes after the first must have a matching p3 value
            if (fragmentCheck != p3) {
                SDL_Log("FRAGMENTATION %d != %d in frameNumber %u", fragmentCheck, p3, itsGame->frameNumber);
                fragmentDetected = true;
            // } else {
            //     SDL_Log("No frags detected so far %d == %d in frameNumber %u", fragmentCheck, p3, itsGame->frameNumber);
            }
        }
    } else {
        SDL_Log("LatencyVote with checksum=%d received outside of the normal voting window not used for fragment check. fn=%u",
                p3, itsGame->frameNumber);
    }
}

std::string CNetManager::FragmentMapToString() {
    std::ostringstream os;
    bool firstFrag = true;
    for (auto const& pair : fragmentMap) {
        if (!firstFrag) { os << std::endl; }
        os << std::hex << std::setfill('0') << std::setw(sizeof(int32_t)*2) << pair.first << ": [";
        for (auto const& slot : pair.second) {
            os << " " << playerTable[slot]->GetPlayerName();
        }
        os << " ]";
        firstFrag = false;
    }
    return os.str();
}

void CNetManager::ViewControl() {
    playerTable[itsCommManager->myId]->ViewControl();
}

void CNetManager::SendPingCommand(int totalTrips) {
    if (totalTrips > 0) {
        itsCommManager->SendPacket(kdEveryone - (1 << itsCommManager->myId),
                                   kpPing, 0, 0, totalTrips-1, 0, NULL);
   }
}

bool CNetManager::CanPlay() {
   return (!isPlaying && !playerTable[itsCommManager->myId]->IsAway());
}

void CNetManager::SendStartCommand(int16_t originalSender) {
    SDL_Log("CNetManager::SendStartCommand(%d)\n", originalSender);

    activePlayersDistribution = 0;
    startPlayersDistribution = 0;
    // set readyPlayers partly as an indicator that a start command is being processed
    readyPlayers = 0;
    unavailablePlayers = 0;

    if (itsCommManager->myId == 0) {
        // to avoid multiple simultaneous starts, only the server sends the kpStartLevel requests to everyone
        for (int i = 0; i < kMaxAvaraPlayers; i++) {
            SDL_Log("  loadingStatus[%d] = %d\n", i, playerTable[i]->LoadingStatus());
            if (playerTable[i]->LoadingStatus() == kLLoaded || playerTable[i]->LoadingStatus() == kLReady) {
                activePlayersDistribution |= 1 << i;
            }
        }
        SDL_Log("  server sending kpStartLevel to everyone = 0x%02x\n", activePlayersDistribution);
        startingGame = true;
    } else {
        // clients ask the server to forward the kpStartLevel request to everyone
        SDL_Log("  client sending kpStartLevel to server only = 0x01\n");
        activePlayersDistribution = kdServerOnly;
        startingGame = false;
    }

    itsCommManager->SendPacket(activePlayersDistribution, kpStartLevel, originalSender, activePlayersDistribution, 0, 0, 0);
}

void CNetManager::ReceiveStartCommand(short activeDistribution, int16_t senderSlot, int16_t originalSender) {
    SDL_Log("CNetManager::ReceiveStartCommand(0x%02x, %d, %d)\n", activeDistribution, senderSlot, originalSender);

    if (senderSlot != 0) {
        // The server will forward clients' kpStartLevel message to kdEveryone,
        // iff readyPlayers hasn't been set, to make sure we aren't sending multiple start commands.
        if (itsCommManager->myId == 0) {
            if (!startingGame) {
                SDL_Log("  server sending kpStartLevel on behalf of %s\n",
                        playerTable[senderSlot]->GetPlayerName().c_str());
                SendStartCommand(senderSlot);
            } else {
                SDL_Log("  server NOT sending kpStartLevel on behalf of %s because it's already trying to start a game\n",
                        playerTable[senderSlot]->GetPlayerName().c_str());
            }
        }
    } else {
        if (CanPlay()) {
            deadOrDonePlayers = 0;
            activePlayersDistribution = activeDistribution;
            startPlayersDistribution = activeDistribution;
            itsGame->itsApp->DoCommand(kGetReadyToStartCmd);
            isPlaying = true;
            itsGame->ResumeGame();
        } else {
            SDL_Log("  sending kpUnavailableSync\n");
            itsCommManager->SendPacket(activeDistribution, kpUnavailableSynch, originalSender, 0, 0, 0, NULL);
        }
    }
}

void CNetManager::SendResumeCommand(int16_t originalSender) {
    SDL_Log("CNetManager::SendResumeCommand(%d)\n", originalSender);

    activePlayersDistribution = 0;

    if (itsCommManager->myId == 0) {
        // to avoid multiple simultaneous starts, only the server sends the kpResumeLevel requests to everyone
        for (int i = 0; i < kMaxAvaraPlayers; i++) {
            if (playerTable[i]->GetPlayer() && !playerTable[i]->GetPlayer()->isOut) {
                activePlayersDistribution |= 1 << i;
            }
        }
        startingGame = true;
        SDL_Log("  server sending kpResumeLevel to everyone = 0x%02x\n", activePlayersDistribution);
    } else {
        // clients ask the server to forward the kpStartLevel request to everyone
        activePlayersDistribution = kdServerOnly;
        startingGame = false;
        SDL_Log("  client sending kpResumeLevel to server only = 0x01\n");
    }

    itsCommManager->SendPacket(activePlayersDistribution, kpResumeLevel, originalSender, activePlayersDistribution, FRandSeed, 0, 0);
}

void CNetManager::ReceiveResumeCommand(short activeDistribution, short senderSlot, Fixed randomKey, int16_t originalSender) {
    SDL_Log("CNetManager::ReceiveResumeCommand(0x%02x, %d, 0x%08x, %d)\n", activeDistribution, senderSlot, randomKey, originalSender);

    if (senderSlot != 0) {
        // The server will forward clients' kpStartLevel message to kdEveryone,
        // iff readyPlayers hasn't been set, to make sure we aren't sending multiple start commands.
        if (itsCommManager->myId == 0) {
            if (!startingGame) {
                SDL_Log("  server sending kpResumeLevel on behalf of %s\n",
                        playerTable[senderSlot]->GetPlayerName().c_str());
                SendResumeCommand(senderSlot);
            } else {
                SDL_Log("  server NOT sending kpResumeLevel on behalf of %s because it's already trying to resume a game\n",
                        playerTable[senderSlot]->GetPlayerName().c_str());
            }
        }
    } else {
        activePlayersDistribution = activeDistribution;
        if (CanPlay() && randomKey == FRandSeed) {
            itsGame->itsApp->DoCommand(kGetReadyToStartCmd);
            isPlaying = true;
            itsGame->ResumeGame();
        } else {
            itsCommManager->SendPacket(activePlayersDistribution, kpUnavailableSynch, originalSender, 0, 0, 0, NULL);
        }
    }
}

void CNetManager::ReceivedUnavailable(short slot, short fromSlot) {
    unavailablePlayers |= 1 << slot;

    if (slot == itsCommManager->myId) {
        itsGame->itsApp->ParamLine(kmStartFailure, MsgAlignment::Center, playerTable[fromSlot]->PlayerName(), NULL);
    } else {
        itsGame->itsApp->ParamLine(kmUnavailableNote, MsgAlignment::Center, playerTable[slot]->PlayerName(), NULL);
    }
}

Boolean CNetManager::ResumeEnabled() {
    //	Check to see if all the players in 'my' game
    //	have the correct randomKey and are indeed in paused
    //	state. If not, tell the caller that resume is not available.

    short i;

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        if (playerTable[i]->GetPlayer() && !playerTable[i]->GetPlayer()->isOut &&
            !(playerTable[i]->RandomKey() == FRandSeed && playerTable[i]->LoadingStatus() == kLPaused)) {
            return false;
        }
    }

    return true;
}

void CNetManager::StopGame(short newStatus) {
    short playerStatus;
    short slot = itsCommManager->myId;
    CPlayerManager *thePlayerManager;
    CAbstractPlayer *thePlayer;
    long winFrame = 0;

    SDL_Log("CNetManager::StopGame\n");
    isPlaying = false;
    if (newStatus == kPauseStatus) {
        playerStatus = kLPaused;
    } else {
        if (newStatus == kNoVehicleStatus)
            playerStatus = kLNoVehicle;
        else
            playerStatus = isConnected ? kLConnected : kLNotConnected;
    }

    thePlayerManager = playerTable[slot];
    thePlayer = thePlayerManager->GetPlayer();

    if (thePlayer) {
        winFrame = thePlayer->winFrame;

        /*
        GetDateTime(&gameResult.r.when);
        gameResult.levelTag = itsGame->loadedTag;
        gameResult.directoryTag = itsGame->loadedDirectory;
        thePlayer->FillGameResultRecord(&gameResult);

        if(gameResult.r.time >= 0)
        {	itsGame->statusRequest = kWinStatus;
        }
        else
        {	itsGame->statusRequest = kLoseStatus;
        }
        */
    } else { //	Something is seriously wrong here.
        itsGame->statusRequest = kAbortStatus;
    }

    itsCommManager->SendPacket(
        kdEveryone, kpPlayerStatusChange, slot, playerStatus, FRandSeed, sizeof(long), (Ptr)&winFrame);

    itsGame->itsApp->BroadcastCommand(kGameResultAvailableCmd);
}

void CNetManager::ReceivePlayerStatus(short slotId, short newStatus, Fixed randomKey, long winFrame) {
    if (slotId >= 0 && slotId < kMaxAvaraPlayers) {
        if (randomKey != 0) {
            playerTable[slotId]->RandomKey(randomKey);
        }
        playerTable[slotId]->SetPlayerStatus(newStatus, winFrame);
    }
}

void CNetManager::ReceiveJSON(short slotId, Fixed randomKey, long winFrame, std::string json){
    if (slotId >= 0 && slotId < kMaxAvaraPlayers) {
        nlohmann::json message = nlohmann::json::parse(json);
        playerTable[slotId]->RandomKey(randomKey);
        /*
        if(message.type() == nlohmann::json::value_t::object) {
            auto it = message.begin();

            if(it.key() == "xyz") {
                bool xyz = it.value();
            }
        }
        */
    }
}

short CNetManager::PlayerCount() {
    CPlayerManager *thePlayer;
    short playerCount = 0;

    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        thePlayer = playerTable[i];
        const std::string playerName((char *)thePlayer->PlayerName() + 1, thePlayer->PlayerName()[0]);

        if (playerName.size() > 0) {
            playerCount++;
        }
    }

    return playerCount;
}

short CNetManager::SelfDistribution() {
    return (1 << itsCommManager->myId);
}

short CNetManager::AlivePlayersDistribution() {
    return activePlayersDistribution & ~deadOrDonePlayers;
}

bool CNetManager::IAmAlive() {
    return AlivePlayersDistribution() & SelfDistribution();
}

void CNetManager::AttachPlayers(CAbstractPlayer *playerActorList) {
    short i;
    CAbstractPlayer *nextPlayer;
    Boolean changedColors = false;
    char newColors[kMaxAvaraPlayers];

    //	Let active player managers choose actors for themselves.
    for (i = 0; i < kMaxAvaraPlayers; i++) {
        short slot = positionToSlot[i];

        if ((1 << slot) & startPlayersDistribution) {
            playerActorList = playerTable[slot]->ChooseActor(playerActorList, kGreenTeam + teamColors[i]);
        }
    }

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        short slot = positionToSlot[i];
        CPlayerManager *thePlayerMan;

        newColors[slot] = teamColors[i];
        thePlayerMan = playerTable[slot];
        if (((1 << slot) & startPlayersDistribution) && thePlayerMan->GetPlayer() == NULL) {
            if (playerActorList) //	Any actors left?
            {
                playerActorList = thePlayerMan->TakeAnyActor(playerActorList);
                if (itsCommManager->myId == 0 && thePlayerMan->GetPlayer() &&
                    thePlayerMan->PlayerColor() != teamColors[slot]) {
                    changedColors = true;
                    newColors[slot] = thePlayerMan->PlayerColor() - kGreenTeam;
                }
            } else {
                if (thePlayerMan->IncarnateInAnyColor()) {
                    changedColors = true;
                    newColors[slot] = thePlayerMan->PlayerColor() - kGreenTeam;
                }

                if (thePlayerMan->GetPlayer() == NULL && slot == itsCommManager->myId) {
                    long noWin = -1;

                    itsCommManager->SendPacket(
                        kdEveryone, kpPlayerStatusChange, slot, kLNoVehicle, 0, sizeof(long), (Ptr)&noWin);
                }
            }
        }
    }

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        if (playerTable[i]->GetPlayer()) {
            playerTable[i]->SpecialColorControl();
        }
    }

    if (changedColors) {
        itsCommManager->SendPacket(kdEveryone, kpColorChange, 0, 0, 0, kMaxAvaraPlayers, newColors);
    }

    //	Throw away the rest.
    while (playerActorList) {
        nextPlayer = (CAbstractPlayer *)playerActorList->nextActor;
        playerActorList->Dispose();
        playerActorList = nextPlayer;
    }
}

void CNetManager::ConfigPlayer(short senderSlot, Ptr configData) {
    PlayerConfigRecord *config = (PlayerConfigRecord *)configData;
    config->numGrenades = ntohs(config->numGrenades);
    config->numMissiles = ntohs(config->numMissiles);
    config->numBoosters = ntohs(config->numBoosters);
    config->hullType = ntohs(config->hullType);
    config->frameLatency = ntohs(config->frameLatency);
    config->frameTime = ntohs(config->frameTime);
    playerTable[senderSlot]->TheConfiguration() = *config;
}

void CNetManager::DoConfig(short senderSlot) {
    PlayerConfigRecord *theConfig = &playerTable[senderSlot]->TheConfiguration();

    if (playerTable[senderSlot]->GetPlayer()) {
        playerTable[senderSlot]->GetPlayer()->ReceiveConfig(theConfig);
    }

    // any reason for these conditionals?  seems like we should always set frameTime etc.
    if (PermissionQuery(kAllowLatencyBit, 0) || !(activePlayersDistribution & kdServerOnly) || senderSlot == 0) {
        // transmitting latencyTolerance in terms of frameLatency to keep it as a short value on transmission
        itsGame->SetFrameTime(theConfig->frameTime);
        itsGame->SetFrameLatency(theConfig->frameLatency, -1);
        latencyVoteFrame = itsGame->NextFrameForPeriod(AUTOLATENCYPERIOD);
    }
}

void CNetManager::MugShotRequest(short sendTo, long sendFrom) {
    CPlayerManager *myPlayer;
    long mugSize;

    myPlayer = playerTable[itsCommManager->myId];

    if (myPlayer->MugPict() == NULL) {
        itsGame->itsApp->BroadcastCommand(kGiveMugShotCmd);
    }

    mugSize = myPlayer->MugSize();
    if (myPlayer->MugPict() && mugSize > sendFrom) {
        short i;
        long sendPoint;
        long sendLen;

        HLock(myPlayer->MugPict());

        sendPoint = sendFrom;

        for (i = 0; i < kMugShotWindowSize; i++) {
            sendLen = mugSize - sendPoint;
            if (sendLen > PACKETDATABUFFERSIZE)
                sendLen = PACKETDATABUFFERSIZE;

            if (sendLen > 0) {
                itsCommManager->SendPacket(1L << sendTo,
                    kpMugShot,
                    0,
                    sendPoint / PACKETDATABUFFERSIZE,
                    mugSize,
                    sendLen,
                    (*myPlayer->MugPict()) + sendPoint);

                sendPoint += sendLen;
            } else
                break;
        }

        HUnlock(myPlayer->MugPict());
    }
}

void CNetManager::ReceiveMugShot(short fromPlayer, short seqNumber, long totalLength, short dataLen, Ptr dataBuffer) {
    CPlayerManager *thePlayer;

    thePlayer = playerTable[fromPlayer];

    if (seqNumber == 0) {
        if (thePlayer->MugPict()) {
            SetHandleSize(thePlayer->MugPict(), totalLength);
        } else {
            thePlayer->MugPict(NewHandle(totalLength));
        }

        thePlayer->MugState(0);
        thePlayer->MugSize(GetHandleSize(thePlayer->MugPict()));
    }

    if (totalLength == thePlayer->MugSize()) {
        thePlayer->MugState(seqNumber * PACKETDATABUFFERSIZE);

        BlockMoveData(dataBuffer, (*thePlayer->MugPict()) + thePlayer->MugState(), dataLen);

        thePlayer->MugState(thePlayer->MugState() + dataLen);

        if ((seqNumber & (kMugShotWindowSize - 1)) == 0) {
            long nextRequest;

            nextRequest = thePlayer->MugState() + (kMugShotWindowSize - 1) * PACKETDATABUFFERSIZE;
            if (nextRequest < totalLength) {
                itsCommManager->SendPacket(1L << fromPlayer, kpGetMugShot, 0, 0, nextRequest, 0, NULL);
            }
        }

        if (thePlayer->MugState() ==
            totalLength /* && theRoster*/) { // theRoster->InvalidateArea(kFullMapBox, slotToPosition[fromPlayer]);
        }
    }
}

void CNetManager::StoreMugShot(Handle mugPict) {
    short slot = itsCommManager->myId;

    playerTable[slot]->StoreMugShot(mugPict);
}

void CNetManager::ZapMugShot(short slot) {
    CPlayerManager *thePlayer;

    if (slot == -1) {
        slot = itsCommManager->myId;
        itsCommManager->SendPacket(kdEveryone & ~(1 << slot), kpZapMugShot, 0, 0, 0, 0, 0);
    }

    thePlayer = playerTable[slot];
    DisposeHandle(thePlayer->MugPict());

    thePlayer->MugPict(NULL);
    thePlayer->MugSize(-1);
    thePlayer->MugState(0);

    // theRoster->InvalidateArea(kFullMapBox, slotToPosition[slot]);
}

Boolean CNetManager::PermissionQuery(short reason, short index) {
    short slot = itsCommManager->myId;

    if (slot == 0 && reason != kAllowLatencyBit) {
        return reason != kAllowKickBit || positionToSlot[index];
    } else {
        if (reason == kFreeColorBit || reason == kAllowOwnColorBit) {
            return (serverOptions & (1 << kFreeColorBit)) ||
                   ((serverOptions & (1 << kAllowOwnColorBit)) && positionToSlot[index] == slot);
        } else {
            return (serverOptions & (1 << reason)) != 0;
        }
    }
}

void CNetManager::ChangedServerOptions(short curOptions) {
    if (itsCommManager->myId == 0) {
        itsCommManager->SendPacket(kdEveryone, kpServerOptions, 0, curOptions, 0, 0, NULL);
    }
}

void CNetManager::NewArrival(short slot) {
    CPlayerManager *thePlayer = playerTable[slot];
    itsGame->itsApp->NotifyUser();
    SDL_Log("someone has joined in slot #%d\n", slot+1);
    itsGame->scoreKeeper->PlayerJoined();
}

void CNetManager::ResultsReport(Ptr results) {
    itsGame->scoreKeeper->ReceiveResults((int32_t *)results);
}

/*
void	CNetManager::BuildTrackerTags(
    CTracker *tracker)
{
    CAvaraApp		*theApp = (CAvaraApp *)gApplication;
    short			i;
    CPlayerManager	*thePlayer;
    CScoreKeeper	*keeper;
    char			gameStat = ktgsNotLoaded;

    if(itsGame->loadedTag)
    {	if(theApp->directorySpec.name[0])
                tracker->WriteStringTag(ktsLevelDirectory, theApp->directorySpec.name);
        else	tracker->WriteStringTag(ktsLevelDirectory, theApp->appSpec.name);

        tracker->WriteStringTag(ktsLevelName, itsGame->loadedLevel);

        switch(playerTable[0]->loadingStatus)
        {	case kLActive:
                gameStat = ktgsActive;
                break;
            case kLPaused:
                gameStat = ktgsPaused;
                break;
            default:
                gameStat = ktgsLoaded;
                break;
        }
    }

    tracker->WriteCharTag(ktsGameStatus, gameStat);

    keeper = itsGame->scoreKeeper;
    for(i=0;i<kMaxAvaraPlayers;i++)
    {	thePlayer = playerTable[i];

        if(thePlayer->loadingStatus != kLNotConnected)
        {	short	lives;

            tracker->WriteStringTagIndexed(kisPlayerNick, i, thePlayer->playerName);
            tracker->WriteLongTagIndexed(kisPlayerLocation, i, *(long *)&thePlayer->globalLocation);

            if(thePlayer->itsPlayer)
                lives = thePlayer->itsPlayer->lives;
            else
                lives = keeper->netScores.player[i].lives;

            if(lives >= 0)
            {	if(lives > 99)
                    lives = 99;
                tracker->WriteShortTagIndexed(kisPlayerLives, i, lives);
            }
        }
    }
}
*/
void CNetManager::LoginRefused() {
    long thisTime;

    thisTime = TickCount();
    if (((unsigned long)thisTime - lastLoginRefusal) > 60 * 60 * 4) {
        lastLoginRefusal = thisTime;

        itsGame->itsApp->MessageLine(kmRefusedLogin, MsgAlignment::Center);
    }
}
