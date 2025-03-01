/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CNetManager.c
    Created: Monday, May 15, 1995, 22:25
    Modified: Tuesday, September 17, 1996, 03:21
*/
#include <algorithm> // std::max
#include <bitset>

#include "CNetManager.h"

#include "AssetManager.h"
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
#define LOWERLATENCYCOUNT   (0.01 * AUTOLATENCYPERIOD / itsGame->frameTime)       // waited on >1% of frames
#define HIGHERLATENCYCOUNT  (0.18 * AUTOLATENCYPERIOD / itsGame->frameTime)       // waited on >18% of frames

#define kMessageBufferMaxAge 90
#define kMessageBufferMinAge 30
#define kMessageWaitTime 12

extern Fixed FRandSeed;

void CNetManager::INetManager(CAvaraGame *theGame) {
    short i;

    itsGame = theGame;
    readyPlayers = 0;

    netStatus = kNullNet;
    itsCommManager = std::make_unique<CCommManager>();
    itsCommManager->ICommManager(NULLNETPACKETS);

    itsProtoControl = new CProtoControl;
    itsProtoControl->IProtoControl(itsCommManager.get(), itsGame);

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

std::shared_ptr<CPlayerManager> CNetManager::CreatePlayerManager(short id) {
    std::shared_ptr<CPlayerManagerImpl> pm = std::make_shared<CPlayerManagerImpl>();
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
    std::unique_ptr<CCommManager> newManager = nullptr;
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
                    newManager = std::make_unique<CCommManager>();
                    newManager->ICommManager(NULLNETPACKETS);
                    break;
                case kServerNet: {
                    newManager = std::make_unique<CUDPComm>();
                    CUDPComm *theServer = (CUDPComm*)(newManager.get());
                    theServer->IUDPComm(kMaxAvaraPlayers - 1, TCPNETPACKETS, kAvaraNetVersion, itsGame->frameTime);
                    theServer->StartServing();
                    confirm = theServer->isConnected;
                }   break;
                case kClientNet:
                    newManager = std::make_unique<CUDPComm>();
                    CUDPComm *theClient = (CUDPComm *)(newManager.get());
                    theClient->IUDPComm(kMaxAvaraPlayers - 1, TCPNETPACKETS, kAvaraNetVersion, itsGame->frameTime);
                    theClient->Connect(address, password);
                    confirm = theClient->isConnected;
                    break;
            }
        }

        if (confirm && newManager) {
            PresenceType keepPresence = playerTable[itsCommManager->myId]->Presence();
            itsCommManager->Dispose();        // send kpPacketProtocolLogout message before being destroyed
            itsProtoControl->Detach();
            itsCommManager.swap(newManager);  // newManager takes place existing CCommManager which gets deleted when out of scope
            playerTable[itsCommManager->myId]->SetPresence(keepPresence);
            itsProtoControl->Attach(itsCommManager.get());
            netStatus = netKind;
            isConnected = true;
            DisconnectSome(kdEveryone);

            totalDistribution = 0;
            DBG_Log("login", "sending kpLogin to server with presence=%d\n", keepPresence);
            itsCommManager->SendPacket(kdServerOnly, kpLogin, 0, keepPresence, 0, 0L, NULL);
            if (itsGame->loadedFilename.length() > 0) {
                itsGame->LevelReset(true);
                // theRoster->InvalidateArea(kBottomBox, 0);
            }
            itsGame->itsApp->BroadcastCommand(kNetChangedCmd);
        }
    } else {
        playerTable[itsCommManager->myId]->NetDisconnect();
        itsCommManager->SendPacket(kdServerOnly, kpLogin, 0, playerTable[itsCommManager->myId]->Presence(), 0, 0L, NULL);
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

    thePlayer = playerTable[slotId].get();
    if (regStatus < 0) {
        thePlayer->IsRegistered(true);
        BlockMoveData(realName, thePlayer->PlayerRegName(), realName[0] + 1);
    } else {
        thePlayer->IsRegistered(false);
        thePlayer->PlayerRegName()[0] = 0;
        // GetIndString(thePlayer->PlayerRegName(), 133, 5 + regStatus);
    }

    // theRoster->InvalidateArea(kRealNameBox, slotToPosition[slotId]);
}

void CNetManager::NameChange(StringPtr newName) {
    /*
    MachineLocation		myLocation;
    Point				loc;

    ReadLocation(&myLocation);
    loc.h = myLocation.longitude >> 16;
    loc.v = myLocation.latitude >> 16;
    */
    LoadingState status = playerTable[itsCommManager->myId]->LoadingStatus();
    PresenceType presence = playerTable[itsCommManager->myId]->Presence();
    itsCommManager->SendPacket(kdEveryone, kpNameChange, 0, status, presence, newName[0] + 1, (Ptr)newName);
}

void CNetManager::ValueChange(short slot, std::string attributeName, bool value) {
    std::string json = "{\"" + attributeName + "\":" + (value == true ? "true" : "false") + "}";
    char* c = const_cast<char*>(json.c_str());
    SDL_Log("sending json %s", c);
    itsCommManager->SendPacket(kdEveryone, kpJSON, slot, 0, 0, strlen(c), c);
}

void CNetManager::RecordNameAndState(short theId, StringPtr theName, LoadingState status, PresenceType presence) {
    if (theId >= 0 && theId < kMaxAvaraPlayers) {
        totalDistribution |= 1 << theId;
        if (status != 0)
            playerTable[theId]->SetPlayerStatus(status, presence, -1);

        playerTable[theId]->ChangeName(theName);
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

void CNetManager::BufferMessage(size_t len, char *c) {
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

void CNetManager::SendRosterMessage(size_t len, char *c) {
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
        thePlayer = playerTable[slotId].get();
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
    for (short i = 0; i < kMaxAvaraPlayers; i++) {
        if ((1L << i) & mask) {
            playerTable[i]->NetDisconnect();
        }
    }
    totalDistribution &= ~mask;
}

void CNetManager::HandleDisconnect(short slotId, short why) {
    DBG_Log("login+", "HandleDisconnect called for slot #%d, why=%d", slotId, why);
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
    PacketInfo *aPacket;
    SDL_Log("SendLoadLevel(%s, %s, %d)\n", theSet.c_str(), levelTag.c_str(), originalSender);

    ProcessQueue();

    aPacket = itsCommManager->GetPacket();

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

    aPacket->dataLen = std::min<int16_t>(setAndLevel.length() + 1, PACKETDATABUFFERSIZE);
    BlockMoveData(setAndLevel.c_str(), aPacket->dataBuffer, aPacket->dataLen);

    /* TODO: implement
     itsGame->itsApp->GetDirectoryLocator((DirectoryLocator *)aPacket->dataBuffer);

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
        CPlayerManager *sendingPlayer = playerTable[originalSender].get();

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

        aPacket->dataLen = std::min<int16_t>(tag.length() + 1, PACKETDATABUFFERSIZE);
        BlockMoveData(tag.c_str(), aPacket->dataBuffer, aPacket->dataLen);
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

    thePlayer = playerTable[senderSlot].get();
    thePlayer->LevelCRC(crc);
    thePlayer->LevelTag(theTag);
    thePlayer->LevelErr(err);

    if (senderSlot == loaderSlot) {
        for (i = 0; i < kMaxAvaraPlayers; i++) {
            playerTable[i]->LoadStatusChange(crc, err, theTag);

        }

        SDL_Log("CNetManager::LevelLoadStatus loop x%zu\n", i);
    } else {
        thePlayer->LoadStatusChange(
            playerTable[loaderSlot]->LevelCRC(), playerTable[loaderSlot]->LevelErr(), playerTable[loaderSlot]->LevelTag());
    }

    // reset startingGame flag whenever a new level is loaded
    startingGame = false;
}

size_t CNetManager::SkipLostPackets(int16_t dist) {
    CUDPComm* theComm = dynamic_cast<CUDPComm*>(itsCommManager.get());
    size_t remaining = theComm->SkipLostPackets(dist);
    DBG_Log("q", "SkipLostPackets: remaining recv queues for 0x%02x = %zu", dist, remaining);
    return remaining;
}

Boolean CNetManager::GatherPlayers(Boolean isFreshMission) {
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        // reset frame buffers
        playerTable[i]->ResumeGame();
    }

    readyPlayers = 0;
    readyPlayersConsensus = 0;

    Boolean goAhead = false;
    uint64_t currentTime = TickCount();
    int loopCount = 0;
    uint64_t resendTime = currentTime;
    static int READY_SYNC_PERIOD = MSEC_TO_TICK_COUNT(4.0*CLASSICFRAMETIME);  // rounds to 250ms
    uint64_t timeLimit = currentTime + READY_SYNC_PERIOD*24;  // ~6sec
    // wait to hear from everyone, within time limit
    while (activePlayersDistribution != readyPlayersConsensus &&
           currentTime < timeLimit) {
        if (currentTime >= resendTime) {
            uint16_t distrib = activePlayersDistribution;
//#define DONT_SEND_FIRST_READY  // debug what happens when slot 0 doesn't get packet from slot 1
#ifdef DONT_SEND_FIRST_READY
            if (itsCommManager->myId == 1 && loopCount == 0) {
                distrib &= ~1; // don't send kpReadySynch from player 2 to player 1
                // fake incrementing the serialNumber on the connection we aren't sending to
                dynamic_cast<CUDPComm*>(itsCommManager.get())->connections[0].serialNumber += kSerialNumberStepSize;
                SDL_Log("NOT SENDING kpReadySync to slot 0!!!\n");
            }
#endif
            SDL_Log("CNetManager::GatherPlayers sending kpReadySynch to 0x%02x with readyPlayers = 0x%02x\n", distrib, readyPlayers);
            itsCommManager->SendUrgentPacket(distrib, kpReadySynch, 0, readyPlayers, 0, 0, 0);
            resendTime += READY_SYNC_PERIOD;
            if ((++loopCount % 5) == 0) {
                // occasionally, skip lost packets on the connections who don't agree with consensus yet
                // (don't do this too often because it can cause you to lose frame 0... i think)
                SkipLostPackets(distrib & ~readyPlayersConsensus);
            }
        }
        // processes kpReadySynch messages coming from other players
        ProcessQueue();
        currentTime = TickCount();
    }

    if (activePlayersDistribution == readyPlayersConsensus) {
        // everybody agrees, all active players are ready (normal/good case)
        goAhead = true;
    } else {
        // if less than all active players are ready, see if we have a majority
        std::bitset<kMaxAvaraPlayers> activeBits{activePlayersDistribution};
        std::bitset<kMaxAvaraPlayers> readyBits{readyPlayersConsensus};
        if (readyBits.count() > activeBits.count() / 2) {
            SDL_Log("CNetManager::GatherPlayers using majority decision to start playing with 0x%02x\n", readyPlayersConsensus);
            // if the players in readyPlayers represent at least half of all active players
            activePlayersDistribution = readyPlayersConsensus;
            goAhead = true;
        } else {
            // i must be in the minority
            activePlayersDistribution = 0;
        }
    }

    if (isFreshMission) {
        startPlayersDistribution = activePlayersDistribution;
    }
    SDL_Log("CNetManager::GatherPlayers activePlayers = 0x%02x startPlayers = 0x%02x\n", activePlayersDistribution, startPlayersDistribution);

    startingGame = false;

    return goAhead;
}

void CNetManager::ReceiveReady(short senderSlot, uint32_t senderReadyPlayers) {
    // if sender not already in readyPlayers mask add them
    readyPlayers |= (1 << senderSlot);
    // combine all players' readyPlayer bits to form consensus quickly
    readyPlayersConsensus |= senderReadyPlayers;
    SDL_Log("CNetManager::ReceiveReady(%d, 0x%02x) -> readyPlayers / readyPlayersConsensus = 0x%02x / 0x%02x\n",
            senderSlot, senderReadyPlayers, readyPlayers, readyPlayersConsensus);
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

    UpdateLocalConfig();

    // Pull grenade/missile/booster counts from HULL resource
    long hullRes = ReadLongVar(iHull01 + config.hullType);
    HullConfigRecord hull = **AssetManager::GetHull(hullRes);
    config.numGrenades = hull.maxGrenades;
    config.numMissiles = hull.maxMissiles;
    config.numBoosters = hull.maxBoosters;

    // This is what pulled the counts from CLevelListWind
    itsGame->itsApp->BroadcastCommand(kConfigurePlayerCmd);

    ResetLatencyVote();
    addOneLatency = 0;
    reduceLatencyCounter = 0;
    localLatencyVote = 0;
    latencyVoteFrame = itsGame->NextFrameForPeriod(AUTOLATENCYPERIOD);

    thePlayerManager = playerTable[itsCommManager->myId].get();
    if (thePlayerManager->GetPlayer()) {
        thePlayerManager->DoMouseControl(&tempPoint, !(itsGame->moJoOptions & kJoystickMode));

        PlayerConfigRecord copy {};
        BlockMoveData(&config, &copy, sizeof(PlayerConfigRecord));
        copy.numGrenades = ntohs(config.numGrenades);
        copy.numMissiles = ntohs(config.numMissiles);
        copy.numBoosters = ntohs(config.numBoosters);
        copy.hullType = ntohs(config.hullType);
        copy.frameLatency = ntohs(config.frameLatency);
        copy.frameTime = ntohs(config.frameTime);
        copy.spawnOrder = ntohs(config.spawnOrder);

        copy.hullColor = ntohl(config.hullColor.GetRaw());
        copy.trimColor = ntohl(config.trimColor.GetRaw());
        copy.cockpitColor = ntohl(config.cockpitColor.GetRaw());
        copy.gunColor = ntohl(config.gunColor.GetRaw());

        // everyone sends this packet which eventually sets the LoadingStatus() to kLActive
        itsCommManager->SendUrgentPacket(
            kdEveryone, kpStartSynch, 0, kLActive, FRandSeed, sizeof(PlayerConfigRecord), (Ptr)&copy);

        //	Synchronize players:
        lastTime = TickCount();
        notReady = true;
        do {
            short statusTest;

            statusTest = 0;
            for (i = 0; i < kMaxAvaraPlayers; i++) {
                // mark which players we have received kLActive from
                if (playerTable[i]->LoadingStatus() == kLActive) {
                    statusTest |= 1 << i;
                }
            }

            // have we received kLActive from everyone who's playing?
            if ((statusTest & activePlayersDistribution) == activePlayersDistribution) {
                notReady = false;
            }

            // process packets until we receive all the kpStartSynch messages (with kLActive status)
            ProcessQueue();
            // Do we need this?
            // itsGame->itsApp->DoUpdate();

            // TODO: waiting for players dialog with cancel
        } while (TickCount() - lastTime < 1800 && notReady);

        bool ignoreCustomColors = itsGame->itsApp->Get(kIgnoreCustomColorsTag);
        for (i = 0; i < kMaxAvaraPlayers; i++) {
            if (activePlayersDistribution & (1 << i)) {
                DoConfig(i);

                if (!ignoreCustomColors &&
                    itsGame->frameNumber == 0 &&
                    playerTable[i]->GetPlayer() &&
                    !playerTable[i]->GetPlayer()->didIncarnateMasked &&
                    !playerTable[i]->GetPlayer()->hasTeammates) {
                    playerTable[i]->SpecialColorControl();
                }
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

void CNetManager::AutoLatencyControl(FrameNumber frameNumber, Boolean didWait) {
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
                maxPlayer = playerTable[maxId].get();
                uint8_t llv = std::min(long(UINT8_MAX), localLatencyVote);  // just in case, p1 can only accept a max of 256

                itsCommManager->SendUrgentPacket(
                    activePlayersDistribution, kpLatencyVote, llv, maxRoundLatency, FRandSeed, 0, NULL);
                DBG_Log("lt+", "*** fn=%d Sending kpLatencyVote to 0x%2hx, localLatencyVote=%ld, maxRoundLatency=%ld FRandSeed=%d\n",
                        frameNumber, activePlayersDistribution, localLatencyVote, maxRoundLatency, FRandSeed);
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
                DBG_Log("lt+", "====autoLatencyVote = %ld\n", autoLatencyVote);

                short curFrameLatency = itsGame->FrameLatency();
                short rttFrameLatency = itsGame->RoundTripToFrameLatency(maxRoundTripLatency);

                // Usually maxFrameLatency is determined primarily by maxRoundTripLatency...
                // but addOneLatency helps account for deficiencies in the calculation by looking at
                // how often clients had to wait for packets to arrive (autoLatencyVote)
                short addOneLimit = (1.0/itsGame->fpsScale);
                if (autoLatencyVote >= HIGHERLATENCYCOUNT) {
                    // players had to wait too much, LT needs to go up, either via RTT or addOneLatency
                    // Examples scenarios:
                    //    LT=2.25 , rtt = 2.00, add1 = 0.25 --> add1 = 0.50 (LT -> 2.50)
                    //    LT=2.25 , rtt = 1.75, add1 = 0.25 --> add1 = 0.75 (LT -> 2.50)
                    //    LT=2.25 , rtt = 2.50, add1 = 0.25 --> add1 = 0.25 (LT -> 2.75)
                    addOneLatency = std::max<short>(addOneLatency, (curFrameLatency + 1) - rttFrameLatency);
                    addOneLatency = std::min<short>(addOneLatency, addOneLimit);
                    DBG_Log("lt+", "  ++addOneLatency = %hd\n", addOneLatency);
                    reduceLatencyCounter = 0;
                } else if (autoLatencyVote > LOWERLATENCYCOUNT || reduceLatencyCounter < 2) {
                    // keep the LT from going lower (but can still go higher on RTT)
                    // Examples scenarios:
                    //    LT=2.25 , rtt = 2.00, add1 = 0.25 --> add1 = 0.25 (LT -> 2.25)
                    //    LT=2.25 , rtt = 1.75, add1 = 0.25 --> add1 = 0.50 (LT -> 2.25)
                    //    LT=2.25 , rtt = 2.50, add1 = 0.25 --> add1 = 0.25 (LT -> 2.75)
                    addOneLatency = std::max<short>(addOneLatency, (curFrameLatency - rttFrameLatency));
                    addOneLatency = std::min<short>(addOneLatency, addOneLimit);
                    if (autoLatencyVote <= LOWERLATENCYCOUNT) {
                        reduceLatencyCounter++;
                    } else {
                        reduceLatencyCounter = 0;
                    }
                    DBG_Log("lt+", "  ==addOneLatency = %hd, reduceLatencyCounter = %d\n", addOneLatency, reduceLatencyCounter);
                } else /* autoLatencyVote <= LOWERLATENCYCOUNT && reduceLatencyCounter >= 2 */ {
                    // we can now allow LT to decrease
                    // Examples scenarios:
                    //    LT=2.25 , rtt = 2.00, add1 = 0.25 --> add1 = 0.00 (LT -> 2.00)
                    //    LT=2.25 , rtt = 1.50, add1 = 0.25 --> add1 = 0.25 (LT -> 1.75)
                    //    LT=2.25 , rtt = 2.50, add1 = 0.25 --> add1 = 0.00 (LT -> 2.50)
                    addOneLatency = std::max<short>(addOneLatency - 1,
                                                    std::min<short>(addOneLatency, (curFrameLatency - 1) - rttFrameLatency));
                    addOneLatency = std::max<short>(addOneLatency, 0);
                    DBG_Log("lt+", "  --addOneLatency = %hd\n", addOneLatency);
                    if (curFrameLatency != rttFrameLatency + addOneLatency) {
                        reduceLatencyCounter = 0;
                    }
                }

                short newFrameLatency = rttFrameLatency + addOneLatency;

                // limit LT unless it's set to zero
                if (maxAutoLatency > 0) {
                    newFrameLatency = std::min<short>(newFrameLatency, maxAutoLatency);
                }

                DBG_Log("lt", "  fn=%d RTT=%d, Classic LT=%.2lf, add=%.2lf --> FL=%d\n",
                        frameNumber, maxRoundTripLatency,
                        (maxRoundTripLatency) / (2.0*CLASSICFRAMETIME), addOneLatency*itsGame->fpsScale, newFrameLatency);

                if (newFrameLatency < curFrameLatency) {
                    // max reduction of 1.0LT
                    newFrameLatency = std::max<short>(newFrameLatency, curFrameLatency - 1.0/itsGame->fpsScale);
                    itsGame->SetFrameLatency(newFrameLatency, maxPlayer);
                } else if (newFrameLatency > curFrameLatency) {
                    // max increase of 2.0LT
                    newFrameLatency = std::min<short>(newFrameLatency, curFrameLatency + 2.0/itsGame->fpsScale);
                    itsGame->SetFrameLatency(newFrameLatency, maxPlayer);
                }
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

    DBG_Log("lt+", "CNetManager::ReceiveLatencyVote(%d, %d, %hd, %d)\n", sender, p1, p2, p3);
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
                if (IAmAlive()) {
                    SDL_Log("FRAGMENTATION %d != %d in frameNumber %u", fragmentCheck, p3, itsGame->frameNumber);
                }
                fragmentDetected = true;
            // } else {
            //     SDL_Log("No frags detected so far %d == %d in frameNumber %u", fragmentCheck, p3, itsGame->frameNumber);
            }
        }
    } else {
        if (IAmAlive()) {
            SDL_Log("LatencyVote with checksum=%d received outside of the normal voting window not used for fragment check. fn=%u",
                    p3, itsGame->frameNumber);
        }
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

void CNetManager::SendPingCommand(int pingTrips) {
    // there & back = 2 trips... send less to/from players in game
    // a "poke" is a one-way ping, for just keeping the connection open with less traffic
    int pokeTrips = 1;
    int pokeDist = itsGame->IsPlaying() ? activePlayersDistribution : 0;

    // send periodic poke to those who have NOT finished logging in in hopes that it will help get their connection going
    pokeDist |= ~totalDistribution;

    if (isPlaying) {
        // if I'm playing, ONLY send pokes
        pokeDist = kdEveryone;
    }

    // normal pings for everyone else
    int pingDist = ~pokeDist;

    // but don't ping/poke myself
    pokeDist &= ~(1 << itsCommManager->myId);
    pingDist &= ~(1 << itsCommManager->myId);

//    SDL_Log("pokeDist = %x, pingDist = %x\n", pokeDist, pingDist);

    itsCommManager->SendPacket(pokeDist,
                               kpPing, 0, 0, pokeTrips-1, 0, NULL);
    itsCommManager->SendPacket(pingDist,
                               kpPing, 0, 0, pingTrips-1, 0, NULL);
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
    readyPlayersConsensus = 0;

    if (itsCommManager->myId == 0) {
        // to avoid multiple simultaneous starts, only the server sends the kpStartLevel requests to everyone
        for (int i = 0; i < kMaxAvaraPlayers; i++) {
            SDL_Log("  loadingStatus[%d] = %d\n", i, playerTable[i]->LoadingStatus());
            if ((playerTable[i]->LoadingStatus() == kLLoaded ||
                 playerTable[i]->LoadingStatus() == kLReady) &&
                playerTable[i]->Presence() != kzAway)
            {
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
        }
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
    FrameNumber winFrame = 0;

    thePlayerManager = playerTable[slot].get();
    if (newStatus == kAbortStatus) {
        thePlayerManager->RemoveFromGame();
    }

    SDL_Log("CNetManager::StopGame(%d)\n", newStatus);
    isPlaying = false;
    if (newStatus == kPauseStatus) {
        playerStatus = kLPaused;
    } else {
        if (newStatus == kNoVehicleStatus)
            playerStatus = kLNoVehicle;
        else
            playerStatus = isConnected ? kLConnected : kLNotConnected;
    }

    thePlayer = thePlayerManager->GetPlayer();

    if (thePlayer) {
        winFrame = thePlayer->winFrame;

        /*
        GetDateTime(&gameResult.r.when);
        gameResult.levelTag = itsGame->loadedFilename;
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
        kdEveryone, kpPlayerStatusChange, slot, playerStatus, thePlayerManager->Presence(), sizeof(winFrame), (Ptr)&winFrame);

    itsGame->itsApp->BroadcastCommand(kGameResultAvailableCmd);
}

void CNetManager::ReceivePlayerStatus(short slotId, LoadingState newStatus, PresenceType newPresence, Fixed randomKey, FrameNumber winFrame) {
    if (slotId >= 0 && slotId < kMaxAvaraPlayers) {
        if (randomKey != 0) {
            playerTable[slotId]->RandomKey(randomKey);
        }
        if (newPresence == kzUnknown) {
            // don't change presence
            newPresence =  playerTable[slotId]->Presence();
        }
        playerTable[slotId]->SetPlayerStatus(newStatus, newPresence, winFrame);
    }
}

void CNetManager::ReceiveJSON(short slotId, Fixed randomKey, FrameNumber winFrame, std::string json){
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
        thePlayer = playerTable[i].get();
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
    return startPlayersDistribution & ~deadOrDonePlayers;
}

bool CNetManager::IAmAlive() {
    return AlivePlayersDistribution() & SelfDistribution();
}

void CNetManager::AttachPlayers(CAbstractPlayer *playerActorList) {
    short i;
    CAbstractPlayer *nextPlayer;
    Boolean changedColors = false;
    char newColors[kMaxAvaraPlayers];

    UpdateLocalConfig();

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
        thePlayerMan = playerTable[slot].get();
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
                    FrameNumber noWin = -1;

                    itsCommManager->SendPacket(
                        kdEveryone, kpPlayerStatusChange, slot, kLNoVehicle, 0, sizeof(noWin), (Ptr)&noWin);
                }
            }
        }
    }

    for (i = 0; i < kMaxAvaraPlayers; i++) {
        short slot = positionToSlot[i];
        short teammateCount = 0;
        CPlayerManager *thePlayerMan = playerTable[slot].get();
        if (thePlayerMan->Presence() == kzAvailable && thePlayerMan->GetPlayer()) {
            for (short j = 0; j < kMaxAvaraPlayers; j++) {
                if (i != j) {
                    short otherSlot = positionToSlot[j];
                    CPlayerManager *otherPlayerMan = playerTable[otherSlot].get();
                    if (otherPlayerMan->Presence() == kzAvailable &&
                        otherPlayerMan->GetPlayer() &&
                        thePlayerMan->PlayerColor() == otherPlayerMan->PlayerColor()) {
                        teammateCount++;
                    }
                }
            }
            if (teammateCount > 0) {
                thePlayerMan->GetPlayer()->hasTeammates = true;
            }
        }
    }

    if (changedColors) {
        itsCommManager->SendPacket(kdEveryone, kpColorChange, 0, 0, 0, kMaxAvaraPlayers, newColors);
    }

    //	Throw away the rest.
    while (playerActorList) {
        nextPlayer = (CAbstractPlayer *)playerActorList->nextActor;
        delete playerActorList;
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
    config->spawnOrder = ntohs(config->spawnOrder);
    config->hullColor = ntohl(config->hullColor.GetRaw());
    config->trimColor = ntohl(config->trimColor.GetRaw());
    config->cockpitColor = ntohl(config->cockpitColor.GetRaw());
    config->gunColor = ntohl(config->gunColor.GetRaw());
    playerTable[senderSlot]->TheConfiguration() = *config;
}

void CNetManager::DoConfig(short senderSlot) {
    PlayerConfigRecord *theConfig = &playerTable[senderSlot]->TheConfiguration();

    if (playerTable[senderSlot]->GetPlayer()) {
        playerTable[senderSlot]->GetPlayer()->ReceiveConfig(theConfig);
    }

    // gets the frame info from the server (senderSlot==0) if server is playing OR anyone else if server not playing (seems bad)
    // ... but what about the kAllowLatencyBit?
    if (PermissionQuery(kAllowLatencyBit, 0) || !(activePlayersDistribution & kdServerOnly) || senderSlot == 0) {
        // save the frameTime, latency, maxLatency sent by the server (normally)

        // transmitting latencyTolerance in terms of frameLatency to keep it as a short value on transmission
        itsGame->SetFrameTime(theConfig->frameTime);
        maxAutoLatency = theConfig->maxFrameLatency;
        itsGame->SetFrameLatency(theConfig->frameLatency);
        SDL_Log("DoConfig LT = %lf, maxFL = %d\n", itsGame->latencyTolerance, maxAutoLatency);
        itsGame->SetSpawnOrder((SpawnOrder)theConfig->spawnOrder);
        latencyVoteFrame = itsGame->NextFrameForPeriod(AUTOLATENCYPERIOD);
    }
}

void CNetManager::UpdateLocalConfig() {
    CPlayerManager *thePlayerManager = playerTable[itsCommManager->myId].get();

    config.maxFrameLatency = gApplication
        ? gApplication->Get<float>(kLatencyToleranceTag) / itsGame->fpsScale : 0;
    if (IsAutoLatencyEnabled()) {
        // start with the max RTT value
        config.frameLatency = itsGame->RoundTripToFrameLatency(itsCommManager->GetMaxRoundTrip(AlivePlayersDistribution()));
        config.frameLatency += 0.25/itsGame->fpsScale;  // a wee buffer on startup LT (adds 0.25LT for 62.5 fps)
        config.frameLatency = std::min(config.frameLatency, config.maxFrameLatency);
    } else {
        // fix LT to kLatencyToleranceTag
        config.frameLatency = config.maxFrameLatency;
    }
    config.frameTime = itsGame->frameTime;
    config.spawnOrder = gApplication ? gApplication->Get<short>(kSpawnOrder) : ksHybrid;
    config.hullType = gApplication ? gApplication->Number(kHullTypeTag) : 0;
    config.hullColor = gApplication
        ? ARGBColor::Parse(gApplication->String(kPlayerHullColorTag))
            .value_or(
                ColorManager::getTeamColor(thePlayerManager->PlayerColor())
                    .value_or((*ColorManager::getMarkerColor(0)).WithA(0xff))
            )
        : ColorManager::getTeamColor(thePlayerManager->PlayerColor())
            .value_or((*ColorManager::getMarkerColor(0)).WithA(0xff));
    config.trimColor = gApplication
        ? ARGBColor::Parse(gApplication->String(kPlayerHullTrimColorTag))
            .value_or((*ColorManager::getMarkerColor(1)).WithA(0xff))
        : (*ColorManager::getMarkerColor(1)).WithA(0xff);
    config.cockpitColor = gApplication
        ? ARGBColor::Parse(gApplication->String(kPlayerCockpitColorTag))
            .value_or((*ColorManager::getMarkerColor(2)).WithA(0xff))
        : (*ColorManager::getMarkerColor(2)).WithA(0xff);
    config.gunColor = gApplication
        ? ARGBColor::Parse(gApplication->String(kPlayerGunColorTag))
            .value_or((*ColorManager::getMarkerColor(3)).WithA(0xff))
        : (*ColorManager::getMarkerColor(3)).WithA(0xff);

    thePlayerManager->TheConfiguration() = config;
}

void CNetManager::MugShotRequest(short sendTo, long sendFrom) {
    CPlayerManager *myPlayer;
    long mugSize;

    myPlayer = playerTable[itsCommManager->myId].get();

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
                    static_cast<int32_t>(mugSize),
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

    thePlayer = playerTable[fromPlayer].get();

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
                itsCommManager->SendPacket(1L << fromPlayer, kpGetMugShot, 0, 0, static_cast<int32_t>(nextRequest), 0, NULL);
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

    thePlayer = playerTable[slot].get();
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

void CNetManager::NewArrival(short slot, PresenceType presence) {
    playerTable[slot]->SetPresence(presence);
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

    if(itsGame->loadedFilename)
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

std::vector<CPlayerManager*> CNetManager::PlayersWithPresence(PresenceType presence) {
    std::vector<CPlayerManager*> players;
    for (auto player: playerTable) {
        if (player->LoadingStatus() != kLNotConnected && player->Presence() == presence) {
            players.push_back(player.get());
        }
    }
    return players;
}

std::vector<CPlayerManager*> CNetManager::AvailablePlayers() {
    return PlayersWithPresence(kzAvailable);
}

std::vector<CPlayerManager*> CNetManager::ActivePlayers() {
    std::vector<CPlayerManager*> players;
    for (auto player: playerTable) {
        // players who are actually playing in a game
        if (player->LoadingStatus() != kLNotConnected && player->Presence() == kzAvailable && player->GetPlayer() != NULL) {
            players.push_back(player.get());
        }
    }
    return players;
}

std::vector<CPlayerManager*> CNetManager::AllPlayers() {
    std::vector<CPlayerManager*> players;
    for (auto player: playerTable) {
        // All players connected to the server regardless of status
        if (player->LoadingStatus() != kLNotConnected && player->Presence() != kzUnknown) {
            players.push_back(player.get());
        }
    }
    return players;
}

// return a lowercase copy of string
inline std::string lowerStr(std::string s)
{
    std::string low(s);
    std::transform(low.begin(), low.end(), low.begin(), ::tolower);
    return low;
}

int CNetManager::PlayerSlot(std::string playerName) {
    std::string lowerName1(lowerStr(playerName));
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        std::string lowerName2(lowerStr(playerTable[i].get()->GetPlayerName()));
        // case-insensitive name comparison ("FreD" == "fred")
        if (lowerName1 == lowerName2) {
            return i;
        }
    }
    return -1;
}

void CNetManager::ChangeTeamColors(std::map<int, std::vector<std::string>> colorTeamMap) {
    // first set everyone to black then overwrite below
    for (int i = 0; i < kMaxAvaraPlayers; i++) {
        teamColors[i] = kBlackTeam - kGreenTeam;
    }
    for (auto [color, team]: colorTeamMap) {
        for (auto playerName: team) {
            int slot = PlayerSlot(playerName);
            if (slot >= 0) {
                teamColors[slot] = color - kGreenTeam;
            }
        }
    }
    
    SendColorChange();
}

void CNetManager::SetTeamColor(int slot, int color) {
    if (color > kNeutralTeam) {
        teamColors[slot] = color - kGreenTeam;
    } else {
        // pick first color not being used by other slots
        for (int newColor = 0; newColor < kMaxAvaraPlayers; newColor++) {
            bool colorUsed = false;
            for (int i = 0; i < kMaxAvaraPlayers && !colorUsed; i++) {
                if (i != slot && teamColors[i] == newColor) {
                    colorUsed = true;
                }
            }
            if (!colorUsed) {
                teamColors[slot] = newColor;
                break;
            }
        }
    }
    SendColorChange();
}
