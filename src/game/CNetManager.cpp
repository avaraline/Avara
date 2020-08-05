/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CNetManager.c
    Created: Monday, May 15, 1995, 22:25
    Modified: Tuesday, September 17, 1996, 03:21
*/

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
//#include "CAsyncBeeper.h"
//#include "Ambrosia_Reg.h"
//#include "CTracker.h"
#include "Preferences.h"
#include "System.h"
#include "Beeper.h"

#include <string.h>

#define AUTOLATENCYPERIOD 64
#define AUTOLATENCYDELAY 8
#define LOWERLATENCYCOUNT 3
#define HIGHERLATENCYCOUNT 8

#if ROUTE_THRU_SERVER
    #define kAvaraNetVersion 7
#else
    #define kAvaraNetVersion 666
#endif

#define kMessageBufferMaxAge 90
#define kMessageBufferMinAge 30
#define kMessageWaitTime 12

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
    CAvaraApp *theApp = itsGame->itsApp;

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
            if (itsGame->loadedTag) {
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
    }
}

void CNetManager::SendLoadLevel(std::string theSet, OSType theLevelTag) {
    CAvaraApp *theApp;
    PacketInfo *aPacket;

    ProcessQueue();

    aPacket = itsCommManager->GetPacket();

    theApp = itsGame->itsApp;

    aPacket->command = kpLoadLevel;
    aPacket->p1 = 0;
    aPacket->p2 = 0;
    aPacket->p3 = theLevelTag;
    aPacket->distribution = kdEveryone;
    aPacket->dataLen = theSet.length() + 1;
    BlockMoveData(theSet.c_str(), aPacket->dataBuffer, theSet.length() + 1);

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

void CNetManager::ReceiveLoadLevel(short senderSlot, void *theDir, OSType theTag) {
    CAvaraApp *theApp;
    OSErr iErr;
    short crc = 0;

    if (!isPlaying) {
        std::string set((char *)theDir);
        theApp = itsGame->itsApp;
        iErr = theApp->LoadLevel(set, theTag);
        if (iErr) {
            itsCommManager->SendPacket(kdEveryone, kpLevelLoadErr, 0, iErr, theTag, 0, 0);
        } else {
            itsCommManager->SendPacket(kdEveryone, kpLevelLoaded, 0, crc, theTag, 0, 0);
        }
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

void CNetManager::LevelLoadStatus(short senderSlot, short crc, OSErr err, OSType theTag) {
    short i;

    CPlayerManager *thePlayer;

    thePlayer = playerTable[senderSlot];
    thePlayer->LevelCRC(crc);
    thePlayer->LevelTag(theTag);
    thePlayer->LevelErr(err);

    if (senderSlot == loaderSlot) {
        for (i = 0; i < kMaxAvaraPlayers; i++) {
            playerTable[i]->LoadStatusChange(crc, err, theTag);
        }
    } else {
        thePlayer->LoadStatusChange(
            playerTable[loaderSlot]->LevelCRC(), playerTable[loaderSlot]->LevelErr(), playerTable[loaderSlot]->LevelTag());
    }
}

Boolean CNetManager::GatherPlayers(Boolean isFreshMission) {
    short i;
    Boolean goAhead;
    long lastTime;

    totalDistribution = 0;
    for (i = 0; i < kMaxAvaraPlayers; i++) {
        playerTable[i]->ResumeGame();
    }

    // itsCommManager->SendUrgentPacket(kdEveryone, kpFastTrack, 0, 0, fastTrack.addr.value, 0,0);

    itsCommManager->SendUrgentPacket(activePlayersDistribution, kpReadySynch, 0, 0, 0, 0, 0);
    lastTime = TickCount();
    do {
        SDL_Log("CNetManager::GatherPlayers loop\n");
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
    Boolean allOk = false;

    SDL_Log("CNetManager::ResumeGame\n");
    config.latencyTolerance = gApplication->Number(kLatencyToleranceTag);
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

    fragmentDetected = false;

    maxRoundTripLatency = 0;
    addOneLatency = 0;
    localLatencyVote = 0;
    autoLatencyVote = 0;
    autoLatencyVoteCount = 0;

    thePlayerManager = playerTable[itsCommManager->myId];
    if (thePlayerManager->GetPlayer()) {
        thePlayerManager->DoMouseControl(&tempPoint, true);

        PlayerConfigRecord copy;
        BlockMoveData(&config, &copy, sizeof(PlayerConfigRecord));
        copy.numGrenades = ntohs(config.numGrenades);
        copy.numMissiles = ntohs(config.numMissiles);
        copy.numBoosters = ntohs(config.numBoosters);
        copy.hullType = ntohs(config.hullType);
        copy.latencyTolerance = ntohs(config.latencyTolerance);

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

    if (frameNumber >= AUTOLATENCYPERIOD) {
        if ((frameNumber & (AUTOLATENCYPERIOD - 1)) == 0) {
            long maxRoundLatency;

            maxRoundLatency = itsCommManager->GetMaxRoundTrip(activePlayersDistribution);
            itsCommManager->SendUrgentPacket(
                activePlayersDistribution, kpLatencyVote, localLatencyVote, maxRoundLatency, FRandSeed, 0, NULL);
            localLatencyVote = 0;
        } else if (((frameNumber + AUTOLATENCYDELAY) & (AUTOLATENCYPERIOD - 1)) == 0) {
            if (fragmentDetected) {
                itsGame->itsApp->MessageLine(kmFragmentAlert, centerAlign);
                fragmentDetected = false;
            }

            if ((serverOptions & (1 << kUseAutoLatencyBit)) && autoLatencyVoteCount) {
                Boolean didChange = false;
                long curLatency = itsGame->latencyTolerance;
                long maxFrameLatency;

                autoLatencyVote /= autoLatencyVoteCount;

                if (autoLatencyVote > HIGHERLATENCYCOUNT) {
                    addOneLatency = 1;
                }

                maxFrameLatency = addOneLatency + (maxRoundTripLatency + itsGame->frameTime) /
                                                      (itsGame->frameTime + itsGame->frameTime);

                if (maxFrameLatency > 8)
                    maxFrameLatency = 8;

                if (maxFrameLatency < curLatency) {
                    addOneLatency = 0;
                    itsGame->latencyTolerance--;
                    gApplication->Set(kLatencyToleranceTag, itsGame->latencyTolerance);
                    didChange = true;
                } else if (maxFrameLatency > curLatency && autoLatencyVote > LOWERLATENCYCOUNT) {
                    itsGame->latencyTolerance++;
                    itsGame->itsApp->Set(kLatencyToleranceTag, itsGame->latencyTolerance);
                    didChange = true;
                }

                if (didChange) {
                    SDL_Log("*** LT set to %ld\n", itsGame->latencyTolerance);
                    itsGame->latencyFrameTime = itsGame->frameTime * (4 + itsGame->latencyTolerance) / 4;
                    SDL_Log("*** latencyFrameTime = %ld\n", itsGame->latencyFrameTime);
                    /*
                    if(itsGame->latencyTolerance > 1) {
                        itsGame->latencyTolerance = 1;
                        SDL_Log("*** Capping at LT 1\n");
                    }*/
                }
            }

            autoLatencyVote = 0;
            autoLatencyVoteCount = 0;
            maxRoundTripLatency = 0;
        }
    }
}

void CNetManager::ViewControl() {
    playerTable[itsCommManager->myId]->ViewControl();
}

void CNetManager::SendStartCommand() {
    short i;

    activePlayersDistribution = 0;
    startPlayersDistribution = 0;
    readyPlayers = 0;
    unavailablePlayers = 0;

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        if (playerTable[i]->LoadingStatus() == kLLoaded) {
            activePlayersDistribution |= 1 << i;
        }
    }

    SDL_Log("SENDING START PACKET\n");
    itsCommManager->SendPacket(activePlayersDistribution, kpStartLevel, 0, activePlayersDistribution, 0, 0, 0);
}

void CNetManager::ReceiveStartCommand(short activeDistribution, short fromSlot) {
    SDL_Log("CNetManager::ReceiveStartCommand\n");
    if (/*gApplication->modelessLevel == 0 && */ !isPlaying) {
        deadOrDonePlayers = 0;
        activePlayersDistribution = activeDistribution;
        startPlayersDistribution = activeDistribution;
        itsGame->itsApp->DoCommand(kGetReadyToStartCmd);
        isPlaying = true;
        itsGame->ResumeGame();
    } else {
        itsCommManager->SendPacket(activeDistribution, kpUnavailableSynch, fromSlot, 0, 0, 0, NULL);
    }
}

void CNetManager::ReceiveResumeCommand(short activeDistribution, short fromSlot, Fixed randomKey) {
    short i;
    activePlayersDistribution = activeDistribution;

    if (/*gApplication->modelessLevel == 0 &&*/
        !isPlaying && randomKey == FRandSeed) { // itsGame->itsApp->DoUpdate();

        itsGame->itsApp->DoCommand(kGetReadyToStartCmd);

        isPlaying = true;
        itsGame->ResumeGame();
    } else {
        itsCommManager->SendPacket(activeDistribution, kpUnavailableSynch, fromSlot, 0, 0, 0, NULL);
    }
}

void CNetManager::ReceivedUnavailable(short slot, short fromSlot) {
    unavailablePlayers |= 1 << slot;

    if (slot == itsCommManager->myId) {
        itsGame->itsApp->ParamLine(kmStartFailure, centerAlign, playerTable[fromSlot]->PlayerName(), NULL);
    } else {
        itsGame->itsApp->ParamLine(kmUnavailableNote, centerAlign, playerTable[slot]->PlayerName(), NULL);
    }
}

void CNetManager::SendResumeCommand() {
    short i;
    Fixed myKey;

    activePlayersDistribution = 0;

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        if (playerTable[i]->GetPlayer() && !playerTable[i]->GetPlayer()->isOut) {
            activePlayersDistribution |= 1 << i;
        }
    }

    itsCommManager->SendPacket(activePlayersDistribution, kpResumeLevel, 0, activePlayersDistribution, FRandSeed, 0, 0);
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
        kdEveryone, kpPlayerStatusChange, 0, playerStatus, FRandSeed, sizeof(long), (Ptr)&winFrame);

    itsGame->itsApp->BroadcastCommand(kGameResultAvailableCmd);
}

void CNetManager::ReceivePlayerStatus(short slotId, short newStatus, Fixed randomKey, long winFrame) {
    if (slotId >= 0 && slotId < kMaxAvaraPlayers) {
        playerTable[slotId]->RandomKey(randomKey);
        playerTable[slotId]->SetPlayerStatus(newStatus, winFrame);
    }
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
                        kdEveryone, kpPlayerStatusChange, 0, kLNoVehicle, 0, sizeof(long), (Ptr)&noWin);
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
    config->latencyTolerance = ntohs(config->latencyTolerance);
    playerTable[senderSlot]->TheConfiguration() = *config;
}

void CNetManager::DoConfig(short senderSlot) {
    PlayerConfigRecord *theConfig = &playerTable[senderSlot]->TheConfiguration();

    if (playerTable[senderSlot]->GetPlayer()) {
        playerTable[senderSlot]->GetPlayer()->ReceiveConfig(theConfig);
    }

    if (PermissionQuery(kAllowLatencyBit, 0) || !(activePlayersDistribution & kdServerOnly)) {
        if (itsGame->latencyTolerance < theConfig->latencyTolerance)
            itsGame->latencyTolerance = theConfig->latencyTolerance;
    } else {
        if (senderSlot == 0) {
            itsGame->latencyTolerance = theConfig->latencyTolerance;
        }
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
    std::string name((char *)thePlayer->PlayerName() + 1, thePlayer->PlayerName()[0]);
    SDL_Log("%s has joined!!\n", name.c_str());
}

void CNetManager::ResultsReport(Ptr results) {
    itsGame->scoreKeeper->ReceiveResults((int32_t *)results);
}

void CNetManager::Beep() {
    Beep();
    /*
    if(gAsyncBeeper)
    {	Str255		bellSoundName;
        Handle		theSound;

        gApplication->ReadStringPref(kBellSoundTag, bellSoundName);
        gAsyncBeeper->PlayNamedBeep(bellSoundName);
    }
    */
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

        itsGame->itsApp->MessageLine(kmRefusedLogin, centerAlign);
    }
}
