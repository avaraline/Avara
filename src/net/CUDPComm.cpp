/*
    Copyright ©1996-1998, Juri Munkki
    All rights reserved.

    File: CUDPComm.c
    Created: Monday, January 29, 1996, 13:45
    Modified: Saturday, January 3, 1998, 02:04
*/

#include "CUDPComm.h"

#include "AvaraTCP.h"
#include "CApplication.h"
#include "CCompactTagBase.h"
#include "CRC.h"
#include "CUDPConnection.h"
#include "CharWordLongPointer.h"
#include "CommDefs.h"
#include "CommandList.h"
#include "Preferences.h"
#include "System.h"

// get rid of this
#include "CAvaraApp.h"

#define kUDPProtoHandlerIsAsync true

#define kUDPConnectDialogId 410
#define kUDPServerDialogId 411
#define kUDPPopStrings 410

enum {
    kGrayLineInd = 1,
    kAddToHotListInd,
    kRemoveFromHotListInd,
    kRejectBecauseFullInd,
    kRejectLimitInd,
    kRejectBadPasswordInd,
    kRejectTimeOutInd,
    kDNRFailureInd,
    kConnectionErrorType,
    kModemNetType,
    kLowBandType,
    kModerateBandType,
    kHighBandType,
    kBusyStartingServer,
    kBusyStartingTracker,
    kBusyStartingClient,
    kBusyLookingUpServer,
    kBusyContactingServer
};

//#include "UDPTags.h"

enum {
    kClientConnectItem = 1,
    kClientCloseItem,
    kClientQueryTrackerItem,
    kClientAddressTextItem,
    kClientPortTextItem,
    kClientPasswordTextItem,
    kClientLocalPortTextItem,
    kClientHotPopupItem
};

enum {
    kServerStartItem = 1,
    kServerCancelItem,
    kServerPortTextItem,
    kServerMaxPlayersItem,
    kServerPasswordTextItem,
    kServerTrackerRegItem,
    kServerTrackerLocItem,
    kServerTrackerPopItem,
    kServerTrackerInviteItem
};

static void UDPReadComplete(UDPpacket *packet, void *userData) {
    CUDPComm *theComm = (CUDPComm *)userData;
    theComm->ReadComplete(packet);
}

static void UDPWriteComplete(int result, void *userData) {
    CUDPComm *theComm = (CUDPComm *)userData;
    theComm->WriteComplete(result);
}

/*
static	void	UDPBufferReturnComplete(
    UDPiopbPtr		pb)
{
    CUDPComm	*theComm = (CUDPComm *)pb->csParam.send.userDataPtr;
    if(!theComm->isClosed)
    {	theComm->AsyncRead();
    }
}
*/

static Boolean ImmedProtoHandler(PacketInfo *thePacket, Ptr userData) {
    CUDPComm *theControl = (CUDPComm *)userData;

    return theControl->PacketHandler(thePacket);
}

void CUDPComm::WriteAndSignPacket(PacketInfo *thePacket) {
    CUDPConnection *conn;
    short dummyStackVar;

    thePacket->sender = myId; //	Sign it.

    if (!isClosed) { //	Queue it to be sent out:

        if (thePacket->flags & kpUrgentFlag) {
            for (conn = connections; conn; conn = conn->next) {
                if ((conn->routingMask & thePacket->distribution) && conn->port && !conn->killed) {
                    conn->RoutePacket((UDPPacketInfo *)thePacket);
                }
            }
        }

        for (conn = connections; conn; conn = conn->next) {
            short aMask = 1 << conn->myId;

            if ((aMask & thePacket->distribution) && conn->port && !conn->killed) {
                conn->SendQueuePacket((UDPPacketInfo *)thePacket, aMask);
            }
        }

        //	If the network is available, start sending right now.
        AsyncWrite();
    }

    //	If it was also meant for our own consumption, dispatch it.
    if (thePacket->distribution & (1 << myId)) {
        DispatchPacket(thePacket);
    }

    //	We're all done. Release it.
    ReleasePacket(thePacket);
}

void CUDPComm::ForwardPacket(PacketInfo *thePacket) {
    CUDPConnection *conn;
    short dummyStackVar;

    // SDL_Log("CUDPComm::ForwardPacket cmd=%d sender=%d distribution=%d myId=%d\n", thePacket->command,
    // thePacket->sender, thePacket->distribution, myId);
    if (!isClosed) { //	Queue it to be sent out:

        for (conn = connections; conn; conn = conn->next) {
            short aMask = 1 << conn->myId;

            if ((aMask & thePacket->distribution) && conn->port && !conn->killed) {
                conn->SendQueuePacket((UDPPacketInfo *)thePacket, aMask);
            }
        }

        //	If the network is available, start sending right now.
        AsyncWrite();
    }

    //	If it was also meant for our own consumption, dispatch it.
    if (thePacket->distribution & (1 << myId) ||
        myId ==
            -1) // I ADDED myId == -1, DUNNO IF THIS IS RIGHT, seems like we wont handle some initial packets otherwise
    {
        DispatchPacket(thePacket);
    }

    //	We're all done. Release it.
    ReleasePacket(thePacket);
}

void CUDPComm::ProcessQueue() {
    // short	dummyStackVar;

    // This may need to be called higher up, ProcessQueue might only happen when the game is running, which won't work
    // for connecting
    CheckSockets();

    CCommManager::ProcessQueue();

#if 1
    //	Check to see if there's something we should write.
    if (turboMode) {
        if (nextWriteTime - GetClock() < 0 && !isClosed) {
            AsyncWrite();
        }
    } else {
        Boolean writeAgain;

        if (nextWriteTime - GetClock() < 0) {
            do {
                writeAgain = false;

                if (!isClosed) {
                    writeAgain = AsyncWrite();
                }
            } while (writeAgain);
        }
    }
#endif
    /*
    if(tracker && tracker->isTracking && (specialWakeup || tracker->wakeUp - lastClock < 0))
    {	if(specialWakeup)
        {	specialWakeup = false;
            tracker->wakeUp = lastClock + 2000;
        }
        else
        {	tracker->WakeUp();
        }
    }
    */
}

/*
**	The connection table contains the Id number for the receiver
**	and an IP address + port number for every participating Id.
**	The idea is that if you find a new IP address and port, you
**	have a connection there and if you find that you have an
**	address that is not on the list, the connection was broken and
**	should be removed.
*/
void CUDPComm::SendConnectionTable() {
    PacketInfo *tablePack;
    PacketInfo *theDuplicate;
    CUDPConnection *conn;
    CompleteAddress *table;

    tablePack = GetPacket();
    if (tablePack) {
        table = (CompleteAddress *)tablePack->dataBuffer;
        tablePack->command = kpPacketProtocolTOC;

        //	Fill in the table first:
        for (conn = connections; conn; conn = conn->next) {
            if (conn->port && !conn->killed) {
                table->host = conn->ipAddr;
                table->port = conn->port;
            } else {
                table->host = 0;
                table->port = 0;
            }
            table++;
        }

        tablePack->dataLen = ((Ptr)table) - tablePack->dataBuffer;

        for (conn = connections; conn; conn = conn->next) {
            if (conn->port && !conn->killed) {
                tablePack->p1 = conn->myId;
                tablePack->p2 = 0;
                tablePack->p3 = conn->seed;
                tablePack->distribution = 1 << (conn->myId);
                theDuplicate = DuplicatePacket(tablePack);
                WriteAndSignPacket(theDuplicate);
            }
        }

        ReleasePacket(tablePack);

        SendPacket(kdEveryone, kpPacketProtocolControl, udpCramInfo, cramData, 0, 0, NULL);
    }
}

void CUDPComm::SendRejectPacket(ip_addr remoteHost, short remotePort, OSErr loginErr) {
    SDL_Log("CUDPComm::SendRejectPacket\n");
    /*
    charWordLongP	outData;

    if(rejectPB.ioResult != 1)
    {	rejectDataTable[0].ptr = rejectData;
        rejectDataTable[1].length = 0;

        rejectPB.ioCompletion = NULL;
        rejectPB.udpStream = stream;
        rejectPB.ioCRefNum = gMacTCP;
        rejectPB.csParam.send.reserved = 0;
        rejectPB.csParam.send.checkSum = true;
        rejectPB.csParam.send.wdsPtr = rejectDataTable;
        rejectPB.csParam.send.userDataPtr = (Ptr) this;

        rejectPB.csParam.send.remoteHost = remoteHost;
        rejectPB.csParam.send.remotePort = remotePort;
        rejectPB.csParam.send.localPort = localPort;

        outData.c = rejectData;

        *outData.w++ = softwareVersion;
        *outData.w++ = 0;	//	ACK
        *outData.w++ = 0;	//	Packet serial number
        *outData.c++ = 2;	//	p2 exists.
        *outData.c++ = kpPacketProtocolReject;
        *outData.w++ = loginErr;

        rejectDataTable[0].length = outData.c - rejectData;
        *(short *)rejectData = CRC16((unsigned char *)rejectDataTable[0].ptr, rejectDataTable[0].length);

        if(stream && remotePort)
        {	UDPWrite(&rejectPB, true);
        }
    }
    */
}

CUDPConnection *CUDPComm::DoLogin(PacketInfo *thePacket, UDPpacket *udp) {
    ip_addr remoteHost;
    short remotePort;
    short i;
    OSErr loginErr = noErr;
    CUDPConnection *newConn;
    short newId = 0;

    remoteHost = udp->address.host;
    remotePort = udp->address.port;
    SDL_Log("DoLogin from %u:%d\n", remoteHost, remotePort);
    // remoteHost = receivePB.csParam.receive.remoteHost;
    // remotePort = receivePB.csParam.receive.remotePort;

    i = 0;
    for (newConn = connections; newConn; newConn = newConn->next) {
        if (newConn->port) {
            i++;
        }
    }

    if (i >= clientLimit) {
        loginErr = tmfoErr;
        SendPacket(kdServerOnly, kpPacketProtocolRefusal, 0, 0, 0, 0, NULL);
    } else { //	Look for a free connection:
        for (newConn = connections; newConn; newConn = newConn->next) {
            newId++;
            if (newConn->port == 0) { //	Connection is available
                newConn->myId = newId;
                newConn->seed = thePacket->p3;
                break;
            } else if (newConn->port == remotePort &&
                       newConn->ipAddr == remoteHost) { //	Duplicate login packet, it seems. Just ignore it.
                return NULL;
            }
        }

        if (newConn == NULL) {
            loginErr = mFulErr;
        } else {
            for (i = 0; i <= password[0]; i++) {
                if (password[i] != thePacket->dataBuffer[i]) {
                    loginErr = afpPwdExpiredErr;
                }
            }
        }
    }

    if (loginErr == noErr) { //	Cool, we have a new user.
        newConn->FreshClient(remoteHost, remotePort, kSerialNumberStepSize);
        SendConnectionTable();
    } else { //	We should send a reject packet here..
        SendRejectPacket(remoteHost, remotePort, loginErr);
        newConn = NULL;
    }

    return newConn;
}

void CUDPComm::ReadFromTOC(PacketInfo *thePacket) {
    // SDL_Log("CUDPComm::ReadFromTOC\n");
    CompleteAddress *table;

    myId = thePacket->p1;
    clientReady = true;

    table = (CompleteAddress *)thePacket->dataBuffer;
    table[myId - 1].host = 0;
    table[myId - 1].port = 0;

    connections->MarkOpenConnections(table);
    connections->OpenNewConnections(table);
}

Boolean CUDPComm::PacketHandler(PacketInfo *thePacket) {
    Boolean didHandle = true;

    // SDL_Log("CUDPComm::PacketHandler command=%d p1=%d p2=%d p3=%d\n", thePacket->command, thePacket->p1,
    // thePacket->p2, thePacket->p3);

    switch (thePacket->command) {
        case kpPacketProtocolReject:
            if (!isServing) {
                rejectReason = thePacket->p2;
            }
            break;
        case kpPacketProtocolTOC:
            if (!isServing && thePacket->p3 == seed) {
                ReadFromTOC(thePacket);
                SendPacket(kdEveryone, kpPacketProtocolControl, udpCramInfo, cramData, 0, 0, NULL);
            }
            break;
        case kpPacketProtocolLogout:
            if (!isClosing) {
                specialWakeup = true;

                if (thePacket->p1 == 0) //	The server is gone! All hope is gone...we are lost!
                {
                    SendPacket(1 << myId, kpKillConnection, myId, 0, 0, 0, NULL);
                } else {
                    if (myId == 0) {
                        SendPacket(kdEveryone, kpKillConnection, thePacket->p1, 0, 0, 0, NULL);
                    } else {
                        SendPacket(1 << myId, kpKillConnection, thePacket->p1, 0, 0, 0, NULL);
                    }
                }
            }
            break;
        case kpPacketProtocolControl:
            connections->ReceiveControlPacket(thePacket);
            break;
        case kpKickClient:
            if (!isClosing) {
                SendPacket(kdServerOnly, kpPacketProtocolLogout, myId, 0, 0, 0, 0);
            }
            specialWakeup = true;
            didHandle = false;
            break;

        case kpLogin:
        case kpDisconnect:
        case kpKillConnection:
        case kpNameChange:
        case kpResumeLevel:
        case kpStartLevel:
            specialWakeup = true;
            didHandle = false;
            break;

        default:
            didHandle = false;
            break;
    }

    return didHandle;
}

void CUDPComm::ReadComplete(UDPpacket *packet) {
    if (packet) { //	We actually received some data...let's put it into packets.
        long curTime;
        UDPPacketInfo *thePacket;
        charWordLongP inData;
        char *inEnd;
        short inLen;

        curTime = GetClock();
        inData.c = (char *)packet->data; // receivePB.csParam.receive.rcvBuff;
        inLen = packet->len; // receivePB.csParam.receive.rcvBuffLen;
        inEnd = inData.c + inLen;

        if (inLen >= 2 * sizeof(short)) {
            CUDPConnection *conn;
            unsigned short theCheck;

            theCheck = *inData.uw;
            *inData.w = softwareVersion;

            if (CRC16(inData.uc, inLen) == theCheck) {
                inData.w++;

                for (conn = connections; conn; conn = conn->next) {
                    if (conn->port == packet->address.port /*receivePB.csParam.receive.remotePort*/ &&
                        conn->ipAddr == packet->address.host /*receivePB.csParam.receive.remoteHost*/) {
                        inData.c = conn->ValidatePackets(inData.c, curTime);
                        break;
                    }
                }

                if (conn == NULL) {
                    if (*inData.w++ & 1) {
                        while (*inData.c++)
                            ;
                    }
                }

                while (inEnd > inData.c) {
                    PacketInfo *p;
                    char flags;

                    thePacket = (UDPPacketInfo *)GetPacket();

                    if (thePacket) {
                        p = &thePacket->packet;

                        thePacket->serialNumber = *inData.uw++;
                        flags = *inData.c++;
                        p->command = *inData.c++;

#ifdef ROUTER_CAPABLE
                        if (flags & 64)
                            p->distribution = *inData.w++;
                        else
                            p->distribution = 1 << myId;
#endif
                        p->p3 = (flags & 4) ? *inData.l++ : (flags & 32) ? *inData.uw++ : 0;
                        p->p2 = (flags & 2) ? *inData.w++ : 0;
                        p->dataLen = (flags & 8) ? *inData.w++ : (flags & 16) ? *inData.uc++ : 0;
                        p->p1 = (flags & 1) ? *inData.c++ : 0;

#ifdef ROUTER_CAPABLE
                        if (flags & 128)
                            p->sender = *inData.c++;
                        else
                            p->sender = conn != NULL ? conn->myId : 0;
#endif
                        if (p->dataLen) {
                            BlockMoveData(inData.c, p->dataBuffer, p->dataLen);
                            inData.c += p->dataLen;
                        }

                        // SDL_Log("CUDPComm::ReadComplete cmd=%d p1=%d p2=%d p3=%d flags=%d sender=%d
                        // distribution=%d\n", p->command,
                        //    p->p1, p->p2, p->p3, p->flags, p->sender, p->distribution);

                        if (conn) {
                            conn->ReceivedPacket(thePacket);
                        } else {
                            if (isServing && thePacket->serialNumber == 0 &&
                                thePacket->packet.command == kpPacketProtocolLogin) {
                                conn = DoLogin((PacketInfo *)thePacket, packet);
                            }

                            ReleasePacket((PacketInfo *)thePacket);
                        }
                    } else {
                        inData.c = inEnd;
                    }
                }
            }
        }
    }

    if (!isClosed) {
        AsyncRead();
        AsyncWrite();
    }
}

void CUDPComm::ReceivedGoodPacket(PacketInfo *thePacket) {
    //	thePacket->distribution = kdEveryone;
    if (thePacket->distribution != (1 << myId))
        ForwardPacket(thePacket);
    else
        DispatchAndReleasePacket(thePacket);
}

void CUDPComm::WriteComplete(int result) {
    if (turboMode && !isClosed && result == noErr) {
        AsyncWrite();
    }
}

OSErr CUDPComm::AsyncRead() {
    UDPRead(stream, UDPReadComplete, this);
    return noErr; //	Should be 1, so we'll usually just ignore it.
}

#define kUnitQuota 3
#define kHeaderQuota 12
#define kMaxAllowedQuota (kUnitQuota * 120)
#define kNumConnectionTypes (kFastestConnectionCmd - kSlowestConnectionCmd + 1)

Boolean CUDPComm::AsyncWrite() {
    OSErr theErr;
    CUDPConnection *theConnection;
    CUDPConnection *firstSender;
    CUDPConnection *conn;
    UDPPacketInfo *thePacket = NULL;
    long curTime = GetClock();
    long deltaQuotas[kNumConnectionTypes];
    long delta, acc;
    short i;
    short longestBusyQ;
    long highestRoundTrip;
    Boolean result = false;

    delta = (curTime - lastQuotaTime) * kUnitQuota;
    lastQuotaTime = curTime;

    acc = 0;

    for (i = 0; i <= cramData; i++) {
        acc += delta;
        deltaQuotas[i] = acc;
    }

    while (i < kNumConnectionTypes) {
        deltaQuotas[i++] = acc;
    }

    conn = connections;
    theConnection = NULL;
    longestBusyQ = 1;
    highestRoundTrip = 0;

    while (conn) {
        if (conn->port) {
            conn->quota += deltaQuotas[conn->cramData];

            if (conn->quota > 0) {
                if (conn->nextWriteTime - curTime < 0 &&
                    (conn->queues[kTransmitQ].qHead ||
                        conn->haveToSendAck)) { /*	It has been a long time since we last had
                                                **	a chance to resend data or an ack should
                                                **	be sent. Make it more likely that we get
                                                **	the chance to do it.
                                                */

                    conn->busyQLen++;
                }

                if (conn->busyQLen > longestBusyQ) {
                    theConnection = conn;
                    highestRoundTrip = conn->realRoundTrip;
                    longestBusyQ = conn->busyQLen;
                } else if (conn->busyQLen == longestBusyQ && conn->realRoundTrip > highestRoundTrip) {
                    theConnection = conn;
                    highestRoundTrip = conn->realRoundTrip;
                }
            }
        }

        conn = conn->next;
    }

    if (theConnection) {
        thePacket = theConnection->GetOutPacket(curTime, 0, 0);
    }

    if (thePacket == NULL) {
        firstSender = nextSender;
        do {
            theConnection = nextSender;
            nextSender = theConnection->next;
            if (nextSender == NULL) {
                nextSender = connections;
            }

            if (theConnection->port && theConnection->quota >= 0) {
                thePacket = theConnection->GetOutPacket(curTime, 0, 0);
            }
        } while (nextSender != firstSender && thePacket == NULL);
    }

    if (thePacket) {
        unsigned short *versionCheck;
        charWordLongP outData;
        UDPPacketInfo *packetList = NULL;
        short cramCount;
        short origCramCount;

        if (theConnection->quota > kMaxAllowedQuota) {
            theConnection->quota = kMaxAllowedQuota;
        }

        cramCount = cramData;
        if (theConnection->cramData < cramCount) {
            cramCount = theConnection->cramData;
        }
        origCramCount = cramCount--;

        UDPpacket *udp = SDLNet_AllocPacket(UDPSENDBUFFERSIZE);
        udp->channel = -1;

        outData.c = (char *)udp->data;
        versionCheck = outData.uw++;
        *versionCheck = softwareVersion;

        outData.c = theConnection->WriteAcks(outData.c);

        if (thePacket == kPleaseSendAcknowledge) {
            thePacket = theConnection->GetOutPacket(curTime, (cramCount-- > 0) ? CRAMTIME : 0, CRAMTIME);
        }

        while (thePacket && thePacket != kPleaseSendAcknowledge) {
            PacketInfo *p;
            char *fp;
            char flags = 0;
            short len;

            p = &thePacket->packet;

            *outData.w++ = thePacket->serialNumber;
            fp = outData.c++;

            *outData.c++ = p->command;
#ifdef ROUTER_CAPABLE
            if (p->distribution != 1 << theConnection->myId) {
                *outData.w++ = p->distribution;
                flags |= 64;
            }
#endif
            if (p->p3 & ~0xFFFFL) {
                flags |= 4;
                *outData.l++ = p->p3;
            } else if (p->p3) {
                flags |= 32;
                *outData.uw++ = p->p3;
            }

            if (p->p2) {
                flags |= 2;
                *outData.w++ = p->p2;
            }

            if (p->dataLen > 255) {
                flags |= 8;
                *outData.w++ = p->dataLen;
            } else if (p->dataLen) {
                flags |= 16;
                *outData.uc++ = p->dataLen;
            }

            if (p->p1) {
                flags |= 1;
                *outData.c++ = p->p1;
            }

#ifdef ROUTER_CAPABLE
            if (p->sender != myId) {
                *outData.c++ = p->sender;
                flags |= 128;
            }
#endif
            *fp = flags;

            if (p->dataLen) {
                BlockMoveData(p->dataBuffer, outData.c, p->dataLen);
                outData.c += p->dataLen;
            }

            thePacket->packet.qLink = (PacketInfo *)packetList;
            packetList = thePacket;

            udp->len = outData.c - (char *)udp->data;
            if (udp->len < CRAMPACKSIZE && udp->len < theConnection->quota) {
                thePacket = theConnection->GetOutPacket(curTime, (cramCount-- > 0) ? CRAMTIME : 0, CRAMTIME);
            } else {
                thePacket = NULL;
            }

            // SDL_Log("Sending packet cmd=%d p1=%d p2=%d p3=%d flags=%d dist=%d sender=%d\n", p->command, p->p1, p->p2,
            // p->p3, p->flags,
            //    p->distribution, p->sender);
        }

        theConnection->quota -= kHeaderQuota;

        udp->address.host = theConnection->ipAddr;
        udp->address.port = theConnection->port;
        udp->len = outData.c - (char *)udp->data;

        *versionCheck = CRC16(udp->data, udp->len);

        theConnection->quota -= udp->len;

        curTime = GetClock();
        while (packetList) {
            thePacket = (UDPPacketInfo *)packetList->packet.qLink;

            if (packetList->birthDate ==
                packetList->nextSendTime) { //	This was the first time the packet was ever sent out.

                packetList->birthDate = curTime;

                if (origCramCount) {
                    if ((packetList->packet.flags & kpUrgentFlag) &&
                        urgentResendTime <= theConnection->retransmitTime) {
                        packetList->nextSendTime = curTime + urgentResendTime;
                    } else {
                        packetList->packet.flags &= ~kpUrgentFlag;
                        packetList->nextSendTime = curTime + theConnection->retransmitTime;
                    }
                } else {
                    packetList->packet.flags &= ~kpUrgentFlag;
                    packetList->nextSendTime = curTime + theConnection->retransmitTime;
                }
            } else { //	This packet has been sent before.
                packetList->packet.flags &= ~kpUrgentFlag;
                packetList->nextSendTime = curTime + theConnection->retransmitTime;
            }

            Enqueue((QElemPtr)packetList, &theConnection->queues[kTransmitQ]);
            packetList = thePacket;
        }

        if (stream && theConnection->port) {
            UDPWrite(stream, udp, UDPWriteComplete, this);
            result = true;
        }
    } else {
        nextWriteTime = curTime + (urgentResendTime >> 2) + 1;
    }

    return result;
}

long CUDPComm::GetClock() {
    // Apparently this clock is about 240/second?
    // return SDL_GetTicks() >> 2;
    return (long)((double)SDL_GetTicks() / 4.16666666667);
}

/*
**	urgentTimePeriod is a time in milliseconds during which you expect to send
**	more urgent data. If not, any data marked urgent will be resent even if there
**	no other data to send within twice that period.
*/
void CUDPComm::IUDPComm(short clientCount, short bufferCount, short version, long urgentTimePeriod) {
    OSErr theErr;

    ICommManager(bufferCount);

    inviteString[0] = 0;

    softwareVersion = version;

    //	Convert time from milliseconds to 2*in our time units.
    urgentResendTime = (urgentTimePeriod * 125) >> 8;
    latencyConvert = urgentTimePeriod * 2;

    /*
    prefs = new CTagBase;
    prefs->ITagBase();
    prefs->ConvertFromHandle(gApplication->prefsBase->ReadHandle(kUDPCommPrefsTag));
    */
    retransmitToRoundTripRatio = 192 + (gApplication->Number(kUDPResendPrefTag) << 5);

    isConnected = false;
    isClosed = false;
    isServing = false;
    clientReady = false;
    stream = 0;
    cramData = gApplication->Number(kUDPConnectionSpeedTag);

    myId = -1;

    connections = NULL;

    clientLimit = clientCount;
    maxClients = clientCount;

    while (clientCount--) {
        CUDPConnection *newConn;

        newConn = new CUDPConnection;
        newConn->IUDPConnection(this);
        newConn->next = connections;
        connections = newConn;
    }

    nextSender = connections;

    /*
    writeComplete = NewUDPIOCompletionProc(UDPWriteComplete);
    readComplete = NewUDPIOCompletionProc(UDPReadComplete);
    bufferReturnComplete = NewUDPIOCompletionProc(UDPBufferReturnComplete);
    */

    receiverRecord.handler = ImmedProtoHandler;
    receiverRecord.userData = (Ptr)this;
    AddReceiver(&receiverRecord, kUDPProtoHandlerIsAsync);

    // rejectPB.ioResult = noErr;
    rejectReason = noErr;

    // tracker = NULL;

    lastQuotaTime = GetClock();
    nextWriteTime = lastQuotaTime;
    isClosing = false;
    turboMode = true; // was set to true

    localPort = 0;
    localIP = 0;
    password[0] = 0;
}

OSErr CUDPComm::AllocatePacketBuffers(short packetSpace) {
    OSErr theErr;
    Ptr mem;
    UDPPacketInfo *pp;

    mem = NewPtr(sizeof(Ptr) + sizeof(UDPPacketInfo) * packetSpace);
    theErr = MemError();

    if (theErr == noErr) {
        *(Ptr *)mem = packetBuffers;
        packetBuffers = mem;

        pp = (UDPPacketInfo *)(packetBuffers + sizeof(Ptr));

        while (packetSpace--) {
            Enqueue((QElemPtr)pp, &freeQ);
            freeCount++;
            pp++;
        }
    }

    return theErr;
}

void CUDPComm::Disconnect() {
    long startTicks;
    long delta;
    Boolean changedCursor = false;

    isClosing = true;

    if (isConnected) {
        startTicks = TickCount();

        do {
            ProcessQueue();

            delta = TickCount() - startTicks;
            if (delta > 120) { // SetCursor(&gWatchCursor);
                changedCursor = true;
            }

            if (delta > 60) {
                if (connections->AreYouDone()) {
                    isClosed = true;
                }
            }
        } while (delta < 1200 && !isClosed);

        if (changedCursor)
            ; // SetCursor(&qd.arrow);

        isConnected = false;
    }

    if (stream) {
        isClosed = true;
        isConnected = false;
        DestroySocket(stream);
        stream = 0;
    }

    //	Just in case...
    isClosed = true;
}

void CUDPComm::WritePrefs() {
    /*
    Handle	compactPrefs;

    compactPrefs = prefs->ConvertToHandle();
    gApplication->prefsBase->WriteHandle(kUDPCommPrefsTag, compactPrefs);

    DisposeHandle(compactPrefs);
    */
}

void CUDPComm::Dispose() {
    CUDPConnection *nextConn;

    /*
    if(tracker)
    {	tracker->StopTracking();
    }
    */

    if (isConnected && !isClosed) {
        if (myId == 0)
            SendPacket(kdEveryone, kpPacketProtocolLogout, 0, 0, 0, 0, 0);
        else
            SendPacket(kdServerOnly, kpPacketProtocolLogout, myId, 0, 0, 0, 0);

        Disconnect();
    }

    RemoveReceiver(&receiverRecord, kUDPProtoHandlerIsAsync);

    while (connections) {
        nextConn = connections->next;

        connections->Dispose();
        connections = nextConn;
    }

    /*
    DisposeUDPIOCompletionProc(readComplete);
    DisposeUDPIOCompletionProc(writeComplete);
    DisposeUDPIOCompletionProc(bufferReturnComplete);

    if(tracker)
    {	tracker->Dispose();
    }

    if(prefs)
    {	prefs->Dispose();
        prefs = NULL;
    }
    */

    CCommManager::Dispose();
}

void CUDPComm::CreateServer() {
    OSErr theErr;

    localPort = gApplication->Number(kDefaultUDPPort);

    OpenAvaraTCP();

    if (noErr == CreateStream(localPort)) {
        isConnected = true;
        isServing = true;
        myId = 0;
        AsyncRead();
    }

    /*
    tracker = new CTracker;
    tracker->ITracker(this, prefs);
    tracker->OpenStream();
    if(prefs->ReadShort(kUDPRegAtTrackerTag, true))
    {	tracker->StartTracking();
    }
    */

    //	And that's all there is to it!
}

OSErr CUDPComm::ContactServer(ip_addr serverHost, short serverPort) {
    if (noErr == CreateStream(localPort)) {
        long startTime;
        SDL_Event theEvent;

        seed = TickCount();
        connections->myId = 0;
        connections->port = serverPort;
        connections->ipAddr = serverHost;

        SDL_Log("Connecting to %u:%d (seed=%ld)\n", serverHost, serverPort, seed);

        AsyncRead();
        SendPacket(kdServerOnly, kpPacketProtocolLogin, 0, 0, seed, password[0] + 1, (Ptr)password);

        startTime = TickCount();

        /* TODO: connecting to server dialog
        gApplication->SetCommandParams(kUDPPopStrings, kBusyContactingServer, true, 0);
        gApplication->BroadcastCommand(kBusyStartCmd);
        */

        while (!clientReady && rejectReason == noErr) { // give the system some time...

            /* TODO: cancel connect dialog
            gApplication->BroadcastCommand(kBusyTimeCmd);
            if(gApplication->commandResult)
            {	rejectReason = 1;
                break;
            }
            */

            if (SDL_WaitEventTimeout(&theEvent, 1)) {
                if (theEvent.type == SDL_KEYDOWN && theEvent.key.keysym.sym == SDLK_ESCAPE) {
                    rejectReason = 1;
                }
            }

            if ((TickCount() - startTime) > kClientConnectTimeoutTicks) {
                rejectReason = 2;
            }

            ProcessQueue();
        }

        gApplication->BroadcastCommand(kBusyEndCmd);
        gApplication->BroadcastCommand(kBusyHideCmd);

        if (clientReady) {
            isConnected = true;
            rejectReason = noErr;
        } else {
            Disconnect();

            if (rejectReason != 1) {
                SDL_Log("REJECTED!!! %d\n", rejectReason);
            }
        }
    }

    return rejectReason;
    return noErr;
}

void CUDPComm::Connect(std::string address) {
    OpenAvaraTCP();

    localPort = gApplication->Number(kDefaultUDPPort);

    IPaddress addr;
    CAvaraApp *app = (CAvaraAppImpl *)gApplication;
    SDLNet_ResolveHost(&addr, address.c_str(), localPort);

    ContactServer(addr.host, addr.port);
    /*
    DialogPtr		clientDialog;
    short			itemHit;
    ModalFilterUPP	myFilter;
    Str255			tempString;
    Str255			hostName;
    Rect			iRect,popRect;
    Handle			iHandle;
    Handle			hostHandle;
    Handle			portHandle;
    Handle			passHandle;
    Handle			localPortHandle;
    short			iType;
    long			thePortNumber;
    long			localPortLong;
    OSErr			theErr;

    if(connections)
    {	myFilter = NewModalFilterProc(UDPClientDialogFilter);

        clientDialog = GetNewDialog(kUDPConnectDialogId, 0, (WindowPtr)-1);
        SetPort(clientDialog);

        GetDItem(clientDialog, kClientHotPopupItem, &iType, &iHandle, &popRect);
        GetDItem(clientDialog, kClientAddressTextItem, &iType, &hostHandle, &iRect);
        GetDItem(clientDialog, kClientPortTextItem, &iType, &portHandle, &iRect);
        GetDItem(clientDialog, kClientPasswordTextItem, &iType, &passHandle, &iRect);
        GetDItem(clientDialog, kClientLocalPortTextItem, &iType, &localPortHandle, &iRect);

        prefs->ReadString(kDefaultHostTag, tempString);
        if(tempString[0])
        {	SetIText(hostHandle, tempString);
        }

        NumToString(prefs->ReadLong(kDefaultPortTag, kDefaultUDPPort), tempString);
        SetIText(portHandle, tempString);

        NumToString(prefs->ReadLong(kDefaultClientPortTag, 0), tempString);
        SetIText(localPortHandle, tempString);

        do
        {	ModalDialog(myFilter, &itemHit);
            switch(itemHit)
            {	case kClientConnectItem:
                case kClientCloseItem:
                case kClientAddressTextItem:
                case kClientPortTextItem:
                case kClientPasswordTextItem:
                case kClientLocalPortTextItem:
                    break;
                case kClientHotPopupItem:
                    {	short	newPort;

                        GetIText(hostHandle, hostName);
                        GetIText(portHandle, tempString);
                        if(tempString[0])	StringToNum(tempString, &thePortNumber);
                        else				thePortNumber = kDefaultUDPPort;

                        newPort = thePortNumber;
                        SetPort(clientDialog);
                        if(DoHotPop(&popRect, hostName, &newPort, kHotListTag))
                        {	SelIText(clientDialog, kClientPasswordTextItem, 0, 32767);
                            SetIText(hostHandle, hostName);
                            NumToString(newPort, tempString);
                            SetIText(portHandle, tempString);
                        }
                    }
                    break;
                case kClientQueryTrackerItem:
                    {	short	newPort;

                        tracker = new CTracker;
                        tracker->ITracker(this, prefs);

                        if(tracker->QueryTracker(hostName, &newPort))
                        {	SelIText(clientDialog, kClientPasswordTextItem, 0, 32767);
                            SetIText(hostHandle, hostName);
                            NumToString(newPort, tempString);
                            SetIText(portHandle, tempString);
                        }

                        tracker->Dispose();
                        tracker = NULL;
                    }
                    break;
            }
        } while(itemHit != kClientConnectItem && itemHit != kClientCloseItem);

        GetIText(hostHandle, hostName);
        prefs->WriteString(kDefaultHostTag, hostName);

        GetIText(portHandle, tempString);
        if(tempString[0])
        {	StringToNum(tempString, &thePortNumber);
        }
        else
        {	thePortNumber = kDefaultUDPPort;
        }
        prefs->WriteLong(kDefaultPortTag, thePortNumber);

        GetIText(localPortHandle, tempString);
        if(tempString[0])
        {	StringToNum(tempString, &localPortLong);
        }
        else
        {	localPortLong = 0;
        }
        prefs->WriteLong(kDefaultClientPortTag, localPortLong);

        if(itemHit == kClientConnectItem)
        {	ip_addr		addr;

            gApplication->SetCommandParams(kUDPPopStrings, kBusyStartingClient, false, 0);
            gApplication->BroadcastCommand(kBusyStartCmd);

            theErr = OpenAvaraTCP();
            if(theErr == noErr)
            {
                gApplication->SetCommandParams(kUDPPopStrings, kBusyLookingUpServer, false, 0);
                gApplication->BroadcastCommand(kBusyStartCmd);
                theErr = PascalStringToAddress(hostName, &addr);
                gApplication->BroadcastCommand(kBusyEndCmd);

                if(theErr)
                {	gApplication->BroadcastCommand(kBusyHideCmd);
                    GetIndString(hostName, kUDPPopStrings, kDNRFailureInd);
                    ParamText(hostName, 0,0,0);
                    Alert(412, 0);
                }
                else
                {
                    localPort = localPortLong;
                    GetIText(passHandle, password);
                    if(password[0] > 64)
                        password[0] = 64;

                    theErr = ContactServer(addr, thePortNumber);
                }
            }
            gApplication->BroadcastCommand(kBusyEndCmd);
        }

        WritePrefs();
        DisposeDialog(clientDialog);
        DisposeRoutineDescriptor(myFilter);
    }
    else
    {	SysBeep(10);
    }
    */
}

Boolean CUDPComm::ServerSetupDialog(Boolean disableSome) {
    /*
    DialogPtr		serverDialog;
    short			itemHit;
    ModalFilterUPP	myFilter;
    Str255			tempString;
    Rect			iRect,popRect;
    Handle			portHandle;
    Handle			maxPlayerHandle;
    Handle			passHandle;
    Handle			trackerHandle;
    Handle			inviteHandle;
    short			iType;
    long			thePortNumber;
    ControlHandle	trackerRegBox;
    Boolean			doRegister;
    Handle			iHandle;
    long			maxPlayers;

    myFilter = NewModalFilterProc(UDPServerDialogFilter);

    serverDialog = GetNewDialog(kUDPServerDialogId, 0, (WindowPtr)-1);
    SetPort(serverDialog);

    GetDItem(serverDialog, kServerTrackerPopItem, &iType, &iHandle, &popRect);
    if(disableSome)
        HideDItem(serverDialog, kServerTrackerPopItem);

    GetDItem(serverDialog, kServerPortTextItem, &iType, &portHandle, &iRect);

    if(disableSome)
        SetDItem(serverDialog, kServerPortTextItem, itemDisable | statText, portHandle, &iRect);

    GetDItem(serverDialog, kServerMaxPlayersItem, &iType, &maxPlayerHandle, &iRect);
    GetDItem(serverDialog, kServerPasswordTextItem, &iType, &passHandle, &iRect);
    SetIText(passHandle, password);

    GetDItem(serverDialog, kServerTrackerLocItem, &iType, &trackerHandle, &iRect);

    if(disableSome)
        SetDItem(serverDialog, kServerTrackerLocItem, itemDisable | statText, trackerHandle, &iRect);

    GetDItem(serverDialog, kServerTrackerInviteItem, &iType, &inviteHandle, &iRect);
    GetDItem(serverDialog, kServerTrackerRegItem, &iType, (Handle *)&trackerRegBox, &iRect);
    SetCtlValue(trackerRegBox, prefs->ReadShort(kUDPRegAtTrackerTag, false));

    prefs->ReadString(kUDPTrackerNameTag, tempString);
    if(tempString[0])
    {	SetIText(trackerHandle, tempString);
    }

    prefs->ReadString(kUDPLastInviteStringTag, tempString);
    if(tempString[0])
    {	SetIText(inviteHandle, tempString);
    }

    NumToString(prefs->ReadLong(kDefaultServerPortTag, kDefaultUDPPort), tempString);
    SetIText(portHandle, tempString);

    maxPlayers = prefs->ReadLong(kDefaultPlayersTag, maxClients+1);
    if(maxPlayers < 1)						maxPlayers = 1;
    else if(maxPlayers > maxClients + 1)	maxPlayers = maxClients + 1;

    NumToString(maxPlayers, tempString);
    SetIText(maxPlayerHandle, tempString);

    SelIText(serverDialog, kServerMaxPlayersItem, 0, 32767);

    do
    {	ModalDialog(myFilter, &itemHit);

        switch(itemHit)
        {	case kServerStartItem:
            case kServerCancelItem:
            case kServerPortTextItem:
            case kServerPasswordTextItem:
            case kServerTrackerLocItem:
            case kServerTrackerInviteItem:
                break;
            case kServerTrackerRegItem:
                SetCtlValue(trackerRegBox, !GetCtlValue(trackerRegBox));
                break;
            case kServerTrackerPopItem:
                {	short	newPort;

                    GetIText(trackerHandle, tempString);

                    newPort = kTrackerPortNumber;
                    SetPort(serverDialog);
                    if(DoHotPop(&popRect, tempString, &newPort, kUDPTrackerHotListTag))
                    {	SelIText(serverDialog, kServerTrackerInviteItem, 0, 32767);
                        SetIText(trackerHandle, tempString);
                    }
                }
                break;
        }

    } while(itemHit != kServerStartItem && itemHit != kServerCancelItem);


    if(itemHit == kServerStartItem)
    {

        doRegister = GetCtlValue(trackerRegBox);
        prefs->WriteShort(kUDPRegAtTrackerTag, doRegister);

        GetIText(portHandle, tempString);
        if(tempString[0])
        {	StringToNum(tempString, &thePortNumber);
        }
        else
        {	thePortNumber = kDefaultUDPPort;
        }

        if(thePortNumber > 0 && thePortNumber <= 65535)
        {	localPort = thePortNumber;
        }
        else
        {	localPort = 0;
            thePortNumber = 0;
        }

        GetIText(trackerHandle, tempString);
        prefs->WriteString(kUDPTrackerNameTag, tempString);

        GetIText(inviteHandle, inviteString);
        prefs->WriteString(kUDPLastInviteStringTag, inviteString);
        prefs->WriteLong(kDefaultServerPortTag, thePortNumber);

        GetIText(maxPlayerHandle, tempString);
        if(tempString[0])
        {	StringToNum(tempString, &maxPlayers);
        }
        else
        {	maxPlayers = maxClients + 1;
        }

        if(maxPlayers < 1)						maxPlayers = 1;
        else if(maxPlayers > maxClients + 1)	maxPlayers = maxClients + 1;
        prefs->WriteLong(kDefaultPlayersTag, maxPlayers);

        clientLimit = maxPlayers - 1;

        GetIText(passHandle, password);
        if(password[0] > 64)
            password[0] = 64;
    }

    WritePrefs();
    DisposeDialog(serverDialog);
    DisposeRoutineDescriptor(myFilter);

    return itemHit == kServerStartItem;
    */
    return false;
}

void CUDPComm::StartServing() {
    CreateServer();
    /*
    if(ServerSetupDialog(false))
    {	CreateServer();
    }
    */
}

OSErr CUDPComm::CreateStream(short streamPort) {
    localPort = streamPort;
    if (stream) {
        DestroySocket(stream);
    }
    stream = CreateSocket(localPort);

    // This is almost certainly useless - should make it a preference
    IPaddress addr;
    SDLNet_ResolveHost(&addr, NULL, localPort);
    localIP = addr.host;

    return noErr;
}

void CUDPComm::OptionCommand(long theCommand) {
    if (theCommand >= kRetransmitMin && theCommand <= kRetransmitMax) {
        retransmitToRoundTripRatio = 192 + ((theCommand - kRetransmitMin) << 5);
    } else if (theCommand >= kSlowestConnectionCmd && theCommand <= kFastestConnectionCmd) {
        short newCram;

        newCram = theCommand - kSlowestConnectionCmd;
        if (cramData != newCram) {
            cramData = newCram;
            SendPacket(kdEveryone, kpPacketProtocolControl, udpCramInfo, cramData, 0, 0, NULL);
        }
    } else {
        CCommManager::OptionCommand(theCommand);
    }
}

void CUDPComm::DisconnectSlot(short slotId) {
    if (slotId == myId || slotId == 0) {
        Disconnect();
    } else {
        connections->CloseSlot(slotId);
        if (myId == 0 && isServing) {
            SendConnectionTable();
        }
    }
}

short CUDPComm::GetStatusInfo(short slot, Handle leftColumn, Handle rightColumn) {
    /*
    Handle					leftInfo;
    Handle					rightInfo;
    UDPConnectionStatus		parms;
    Str255					tempStr;
    StringPtr				cName;
    char					searchFor[2];
    short					len;

    leftInfo = GetResource('TEXT', 410);
    HLock(leftInfo);
    PtrToXHand(*leftInfo, leftColumn, GetHandleSize(leftInfo));
    HUnlock(leftInfo);

    rightInfo = GetResource('TEXT', 411);
    HLock(rightInfo);
    PtrToXHand(*rightInfo, rightColumn, GetHandleSize(rightInfo));
    HUnlock(rightInfo);

    parms.hostIP = 0;
    parms.averageRoundTrip = 0;
    parms.pessimistRoundTrip = 0;
    parms.optimistRoundTrip = 0;
    parms.estimatedRoundTrip = 0;
    parms.connectionType = -1;

    if(slot == myId)
    {	GetIPParamBlock ip;

        ip.ioCRefNum=gMacTCP; // the TCP driver refnum global...
        GetMyIPAddr(&ip, false);
        parms.hostIP = ip.ourAddress;
        parms.connectionType = cramData;
    }
    else
    {	connections->GetConnectionStatus(slot, &parms);
    }

    searchFor[0] = '‚Ä¢';
    searchFor[1] = 'o';
    NumToString(parms.optimistRoundTrip, tempStr);
    Munger(rightColumn, 0, searchFor, 2, tempStr+1, tempStr[0]);

    searchFor[1] = 'r';
    NumToString(parms.averageRoundTrip, tempStr);
    Munger(rightColumn, 0, searchFor, 2, tempStr+1, tempStr[0]);

    searchFor[1] = 'p';
    NumToString(parms.pessimistRoundTrip, tempStr);
    Munger(rightColumn, 0, searchFor, 2, tempStr+1, tempStr[0]);

    searchFor[1] = 'e';
    NumToString(parms.estimatedRoundTrip, tempStr);
    Munger(rightColumn, 0, searchFor, 2, tempStr+1, tempStr[0]);

    searchFor[1] = '1';
    NumToString((parms.optimistRoundTrip+latencyConvert-1)/latencyConvert, tempStr);
    Munger(rightColumn, 0, searchFor, 2, tempStr+1, tempStr[0]);

    searchFor[1] = '2';
    NumToString((parms.averageRoundTrip+latencyConvert-1)/latencyConvert, tempStr);
    Munger(rightColumn, 0, searchFor, 2, tempStr+1, tempStr[0]);

    searchFor[1] = '3';
    NumToString((parms.pessimistRoundTrip+latencyConvert-1)/latencyConvert, tempStr);
    Munger(rightColumn, 0, searchFor, 2, tempStr+1, tempStr[0]);

    searchFor[1] = '4';
    NumToString((parms.estimatedRoundTrip+latencyConvert-1)/latencyConvert, tempStr);
    Munger(rightColumn, 0, searchFor, 2, tempStr+1, tempStr[0]);

    searchFor[1] = 'c';
    GetIndString(tempStr, kUDPPopStrings, kModemNetType + parms.connectionType);
    Munger(rightColumn, 0, searchFor, 2, tempStr+1, tempStr[0]);

    searchFor[1] = 'i';
    AddrToStr(parms.hostIP, (char *)tempStr);
    len = 0;
    for(cName = tempStr;*cName++;len++);
    Munger(rightColumn, 0, searchFor, 2, tempStr, len);
    */
    return 32;
}

Boolean CUDPComm::ReconfigureAvailable() {
    return isServing;
}

void CUDPComm::Reconfigure() {
    /*
    CCommander		*saved;

    saved = gApplication->BeginDialog();
    ServerSetupDialog(true);

    if(prefs->ReadShort(kUDPRegAtTrackerTag, true))
    {	tracker->StartTracking();
    }
    else
    {	tracker->StopTracking();
    }

    gApplication->EndDialog(saved);
    */
}

long CUDPComm::GetMaxRoundTrip(short distribution) {
    long maxTrip = 0;
    CUDPConnection *conn;

    for (conn = connections; conn; conn = conn->next) {
        if (conn->port && (distribution & (1 << conn->myId))) {
            if (conn->realRoundTrip > maxTrip)
                maxTrip = conn->optimistTime;
        }
    }

    { maxTrip = ((maxTrip << 9) + 256) / 125; }

    return maxTrip;
}

void CUDPComm::BuildServerTags() {
    /*
    CUDPConnection	*theConn;
    charWordLongP	p;

    tracker->WriteCharTag(ktsPlayerLimit, clientLimit+1);
    tracker->WriteStringTag(ktsInvitation, inviteString);

    if(password[0])	tracker->WriteNullTag(ktsHasPassword);

    p.c = tracker->WriteBufferIndexed(kisPlayerIPPort, 0, 6);

    *p.l++ = localIP;
    *p.w++ = localPort;

    theConn = connections;

    while(theConn)
    {	if(theConn->port && theConn->myId >= 0)
        {	p.c = tracker->WriteBufferIndexed(kisPlayerIPPort, theConn->myId, 6);
            *p.l++ = theConn->ipAddr;
            *p.w++ = theConn->port;
        }

        theConn = theConn->next;
    }
    */
}
