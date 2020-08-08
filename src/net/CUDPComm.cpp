/*
    Copyright ©1996-1998, Juri Munkki
    All rights reserved.

    File: CUDPComm.c
    Created: Monday, January 29, 1996, 13:45
    Modified: Saturday, January 3, 1998, 02:04
*/
#define SIMULATE_LATENCY_ON_CLIENTS 0
#define RANDOMLY_DROP_PACKETS 0
#if SIMULATE_LATENCY_ON_CLIENTS || RANDOMLY_DROP_PACKETS
#include <unistd.h> // for usleep()
#define SIMULATE_LATENCY_MEAN   250000
#define SIMULATE_LATENCY_JITTER  20000
// split between receive & send
#define SIMULATE_LATENCY_FORMULA (SIMULATE_LATENCY_MEAN - SIMULATE_LATENCY_JITTER/2 + int((SIMULATE_LATENCY_JITTER)*float(rand())/RAND_MAX)) / 2
#define SIMULATE_LATENCY_DISTRIBUTION  0x02  // bitmask of who gets the latency
#endif
#if RANDOMLY_DROP_PACKETS
int numToDrop = 0;
#endif

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

// A utility function to byte-swap the host & port and create a human-readable format
static std::string FormatAddr(IPaddress addr) {
    std::ostringstream os;
    ip_addr ipaddr = SDL_SwapBE32(addr.host);
    os << (ipaddr >> 24) << "."
       << ((ipaddr >> 16) & 0xff) << "."
       << ((ipaddr >> 8) & 0xff) << "."
       << (ipaddr & 0xff) <<  ":"
       << SDL_SwapBE16(addr.port);
    return os.str();
}
static std::string FormatAddr(CUDPConnection *conn) {
    IPaddress ip;
    ip.host = conn->ipAddr;
    ip.port = conn->port;
    return FormatAddr(ip);
}


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
    
    // SDL_Log("CUDPComm::WriteAndSignPacket   cmd=%d p1=%d p2=%d p3=%d sndr=%d dist=0x%02hx\n",
    //         thePacket->command, thePacket->p1, thePacket->p2, thePacket->p3,
    //         thePacket->sender, thePacket->distribution);

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

#if ROUTE_THRU_SERVER
static void FastForwardComplete(int result, void *userData) {
    CUDPComm *theComm = (CUDPComm *)userData;
    #if PACKET_DEBUG > 1
        SDL_Log("CUDPComm::FastForwardComplete: result = %d\n", result);
    #endif
}

void CUDPComm::FastForwardPacket(UDPpacket *udp, int16_t distribution) {
    if (distribution != 0) {
        for (CUDPConnection *conn = connections; conn != NULL; conn = conn->next) {
            if (distribution & (1 << conn->myId)) {

                #if PACKET_DEBUG
                    if (udp->len == 7) {  // ACK
                        int16_t rsn = *(uint16_t *)(udp->data + 2);
                        int8_t senderId = *(int8_t *)(udp->data + 4);
                        int16_t distribution = *(int16_t *)(udp->data + 5);
                        SDL_Log("CUDPComm::FastForwardPacket    ACK rsn=%d sndr=%d dist=0x%02x, %s --> %s\n",
                                rsn, senderId, distribution,
                                FormatAddr(udp->address).c_str(),
                                FormatAddr(conn).c_str());
                    } else {
                        int16_t rsn = *(uint16_t *)(udp->data + 2);
                        int16_t sn = *(uint16_t *)(udp->data + 4);
                        // if received sn is odd, then have to skip an extra 4 bytes past ackBitmap
                        int8_t cmd = *(int8_t *)(udp->data + ((rsn & 1) ? 11 : 7));
                        SDL_Log("CUDPComm::FastForwardPacket    rsn=%d sn=%d cmd=%d, %s --> %s\n",
                                rsn, sn, cmd,
                                FormatAddr(udp->address).c_str(),
                                FormatAddr(conn).c_str());
                    }
                #endif

                udp->address.host = conn->ipAddr;
                udp->address.port = conn->port;
                UDPWrite(stream, udp, FastForwardComplete, this);
            }
        }
    }
}


CUDPConnection * CUDPComm::ConnectionForPacket(UDPpacket *udp) {
    int8_t senderId;
    int16_t distribution;

    int16_t rsn, sn;
    int flags_offset;
    uint8_t flags;

    if (udp->len == 7) {
        // only ACK is 7 bytes (with ROUTE_THRU_SERVER enabled)
        rsn = *(int8_t *)(udp->data + 2);
        senderId = *(int8_t *)(udp->data + 4);
        distribution = *(int16_t *)(udp->data + 5);
        #if PACKET_DEBUG
            SDL_Log("CUDPComm::ConnectionForPacket  ACK received - rsn=%d, sndr=%d, dist=0x%02x\n",
                    rsn, senderId, distribution);
        #endif
    } else {
        rsn = *(uint16_t *)(udp->data + 2);
        sn = *(uint16_t *)(udp->data + 4);
        flags_offset = (rsn & 1) ? 10 : 6; // skip past ackBitmap if received sn is odd
        flags = *(int8_t *)(udp->data + flags_offset);

        senderId = 0;  // default to server if senderId not in packet 
        if (flags & 128) {
            // jump past the other fields to extract sender
            int sender_offset = flags_offset +
            2 +                                           // flags + cmd
            ((flags & 64) ? 2 : 0) +                      // distribution
            ((flags & 4) ? 4 : ((flags & 32) ? 2 : 0)) +  // p3
            ((flags & 2) ? 2 : 0) +                       // p2
            ((flags & 8) ? 2 : ((flags & 16) ? 1 : 0)) +  // dataLen
            ((flags & 1) ? 1 : 0);                        // p1
            senderId = *(int8_t *)(udp->data + sender_offset);
        }

        distribution = kdServerOnly;
        if (flags & 64) {
            distribution = *(int16_t *)(udp->data + flags_offset + 2);
        }
        #if PACKET_DEBUG
            SDL_Log("CUDPComm::ConnectionForPacket  rsn=%d sn=%d, flags=0x%x, sndr=%d, dist=0x%02x\n",
                    rsn, sn, flags, senderId, distribution);
        #endif
    }

    CUDPConnection *conn;
    for (conn = connections; conn; conn = conn->next) {
        if (isServing) {
            // server must match host/port because senderId not always included in the packets to the server
            if (conn->ipAddr == udp->address.host && conn->port == udp->address.port) {
                senderId = conn->myId;
                break;
            }
        } else {
            // clients can rely on senderId because we default to id of server above
            if (conn->myId == senderId) {
                break;
            }
        }
    }
    #if PACKET_DEBUG > 1
        SDL_Log("CUDPComm::ConnectionForPacket  isServing = %s, connId=%d\n", isServing ? "true" : "false", conn ? conn->myId : -1);
    #endif

    if (isServing) {
        // fast forward to everyone who's on the distribution (except server)
        FastForwardPacket(udp, distribution & ~kdServerOnly);
        if ((distribution & kdServerOnly) == 0) {
            // don't process this packet any further if server isn't included in distribution
            conn = NULL;
            #if PACKET_DEBUG
                SDL_Log("CUDPComm::ConnectionForPacket  NOT processing packet, not intended for server\n");
            #endif
        }
    }
    return conn;
}
#endif  // ROUTE_THRU_SERVER

void CUDPComm::ForwardPacket(PacketInfo *thePacket) {
    CUDPConnection *conn;
    short dummyStackVar;

    // SDL_Log("CUDPComm::ForwardPacket cmd=%d sndr=%d dist=0x%02x myId=%d\n", thePacket->command,
    //         thePacket->sender, thePacket->distribution, myId);
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
            turboCount = 3; // max messages to send in turboMode
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

void CUDPComm::SendRejectPacket(ip_addr remoteHost, port_num remotePort, OSErr loginErr) {
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
    port_num remotePort;
    short i;
    OSErr loginErr = noErr;
    CUDPConnection *newConn;
    short newId = 0;

    remoteHost = udp->address.host;
    remotePort = udp->address.port;
    SDL_Log("DoLogin from %s\n", FormatAddr(udp->address).c_str());
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
            std::string passwordStr =  gApplication->String(kServerPassword);
            password[0] = passwordStr.length();
            BlockMoveData(passwordStr.c_str(), password + 1, passwordStr.length());
            
            for (i = 0; i <= password[0]; i++) {
                if (password[i] != thePacket->dataBuffer[i]) {
                    loginErr = afpPwdExpiredErr;
                    SDL_Log("Password mismatch");
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
    table[myId - 1].host = 0; // don't want to connect to myself
    table[myId - 1].port = 0;

    connections->MarkOpenConnections(table);
    connections->OpenNewConnections(table);
}

Boolean CUDPComm::PacketHandler(PacketInfo *thePacket) {
    Boolean didHandle = true;

    // SDL_Log("CUDPComm::PacketHandler command=%d p1=%d p2=%d p3=%d\n", thePacket->command, thePacket->p1,
    // thePacket->p2, thePacket->p3);
    // SDL_Log("   CUDPComm::PacketHandler    <<<<  cmd=%d sender=%d  p1=%d p2=%d p3=%d\n", thePacket->command, thePacket->sender,
    //         thePacket->p1, thePacket->p2, thePacket->p3);

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

        #if SIMULATE_LATENCY_ON_CLIENTS
            if ((1 << myId) & SIMULATE_LATENCY_DISTRIBUTION) {
                usleep(SIMULATE_LATENCY_FORMULA); // simulate network latencies
            }
        #endif

        curTime = GetClock();
        inData.c = (char *)packet->data; // receivePB.csParam.receive.rcvBuff;
        inLen = packet->len; // receivePB.csParam.receive.rcvBuffLen;
        inEnd = inData.c + inLen;

        if (inLen >= 2 * sizeof(short)) {
            CUDPConnection *conn;
            unsigned short theCheck;

            theCheck = *inData.uw;
            *inData.w = softwareVersion;

            if (CRC16(inData.uc, inLen) != theCheck) {
                SDL_Log("CUDPComm::ReadComplete packet FAILED CRC check, 0x%04x != 0x%04x!!\n", CRC16(inData.uc, inLen), theCheck);
            } else {
                #if ROUTE_THRU_SERVER
                    // restore the CRC in case we have to fast forward the packet unchanged
                    *inData.w = theCheck;
                #endif
                inData.w++;

                #if ROUTE_THRU_SERVER
                    conn = ConnectionForPacket(packet);
                    // NULL conn suggests this packet needs no further processing
                    if (conn) {
                        inData.c = conn->ValidateReceivedPackets(inData.c, curTime);
                        if (inLen == 7) {
                            // done processing ACK
                            inData.c = inEnd;
                        }
                    }
                #else
                    for (conn = connections; conn; conn = conn->next) {
                        if (conn->port == packet->address.port /*receivePB.csParam.receive.remotePort*/ &&
                            conn->ipAddr == packet->address.host /*receivePB.csParam.receive.remoteHost*/) {
                            inData.c = conn->ValidateReceivedPackets(inData.c, curTime);
                            break;
                        }
                    }
                #endif

                if (conn == NULL) {
                    if (*inData.w++ & 1) {
                        while (*inData.c++)
                            ;
                    }
                } else if (inData.c == inEnd) {
                    #if PACKET_DEBUG
                        SDL_Log("     CUDPComm::ReadComplete(R) <ACK> cn=%d rsn=%d\n", conn->myId, conn->maxValid);
                    #endif
                }

                while (inEnd > inData.c) {
                    PacketInfo *p;
                    uint8_t flags;

                    thePacket = (UDPPacketInfo *)GetPacket();

                    if (thePacket) {
                        p = &thePacket->packet;

                        thePacket->serialNumber = *inData.uw++;
                        flags = *inData.uc++;
                        p->flags = flags;
                        p->command = *inData.c++;

                        if (flags & 64)
                            p->distribution = *inData.w++;
                        else
                            p->distribution = 1 << myId;

                        p->p3 = (flags & 4) ? *inData.l++ : (flags & 32) ? *inData.uw++ : 0;
                        p->p2 = (flags & 2) ? *inData.w++ : 0;
                        p->dataLen = (flags & 8) ? *inData.w++ : (flags & 16) ? *inData.uc++ : 0;
                        p->p1 = (flags & 1) ? *inData.c++ : 0;

                        if (flags & 128)
                            p->sender = *inData.c++;
                        else
                            p->sender = conn != NULL ? conn->myId : 0;

                        #if PACKET_DEBUG > 1
                            SDL_Log("        CUDPComm::ReadComplete sn=%d cmd=%d flags=0x%02x sndr=%d dist=0x%02x\n",
                                    thePacket->serialNumber, p->command, p->flags, p->sender, p->distribution);
                        #endif

                        if (p->dataLen) {
                            BlockMoveData(inData.c, p->dataBuffer, p->dataLen);
                            inData.c += p->dataLen;
                        }

                        // SDL_Log("  CUDPComm::ReadComplete  << num=%d cmd=%d p1=%d p2=%d p3=%d flags=%d sender=%d dist=0x%02x from %s\n",
                        //         thePacket->serialNumber, p->command, p->p1, p->p2, p->p3, p->flags, p->sender, p->distribution,
                        //         FormatAddr(packet->address).c_str());

                        if (conn) {
                            #if ROUTE_THRU_SERVER
                                if (isServing) {
                                    if (p->distribution & (1 << myId)) {
                                        conn->ReceivedPacket(thePacket);
                                    }
                                } else {
                                    // since we're routing through the server, receiving client needs to assign this packet
                                    // to the connection associated with the sending client
                                    if (conn->myId < 0) {
                                        // initially client has no ID (myId = -1)
                                        SDL_Log("CUDPComm::ReadComplete: myId == -1\n");
                                        conn->ReceivedPacket(thePacket);
                                    } else {
                                        for (conn = connections; conn; conn = conn->next) {
                                            if (conn->myId == p->sender) {
                                                conn->ReceivedPacket(thePacket) ;
                                                // SDL_Log("CUDPComm::ReadComplete: Found connection to assign received packet to!\n");
                                                break;
                                            }
                                        }
                                    }
                                    if (conn == NULL) {
                                        SDL_Log("CUDPComm::ReadComplete: Couldn't find connection to assign received packet to!\n");
                                    }
                                }

                            #else // !ROUTE_THRU_SERVER
                                conn->ReceivedPacket(thePacket);
                            #endif

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
        AsyncWrite();
        AsyncRead();
    }
}

void CUDPComm::ReceivedGoodPacket(PacketInfo *thePacket) {
    #if PACKET_DEBUG > 1
        SDL_Log("CUDPComm::ReceivedGoodPacket cmd=%d sndr=%d dist=0x%02x myId=%d\n", thePacket->command,
                thePacket->sender, thePacket->distribution, myId);
    #endif
    //	thePacket->distribution = kdEveryone;
    if (thePacket->distribution != (1 << myId))
        ForwardPacket(thePacket);
    else
        DispatchAndReleasePacket(thePacket);
}

void CUDPComm::WriteComplete(int result) {
    if (turboMode && turboCount-- > 0 && !isClosed && result == noErr) {
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

    theConnection = NULL;
    longestBusyQ = 1;
    highestRoundTrip = 0;

    for (CUDPConnection *conn = connections; conn != NULL; conn = conn->next) {
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
                    highestRoundTrip = conn->meanRoundTripTime;
                    longestBusyQ = conn->busyQLen;
                } else if (conn->busyQLen == longestBusyQ && conn->meanRoundTripTime > highestRoundTrip) {
                    theConnection = conn;
                    highestRoundTrip = conn->meanRoundTripTime;
                }
            }
        }
    }

    if (theConnection) {
        thePacket = theConnection->GetOutPacket(curTime, 0, 0);
    }

    if (thePacket == NULL) {  // haven't found anything to send yet
        firstSender = nextSender;
        do { // loop thru all connections looking for something to send
            theConnection = nextSender;
            nextSender = theConnection->next;
            if (nextSender == NULL) {
                nextSender = connections; // back to top to the connection list
            }

            if (theConnection->port && theConnection->quota >= 0) {
                thePacket = theConnection->GetOutPacket(curTime, 0, 0);
            }
        } while (nextSender != firstSender && thePacket == NULL);
    }

    if (thePacket) {  // did we find something to send?
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
            packetList = theConnection->GetOutPacket(curTime, (cramCount-- > 0) ? CRAMTIME : 0, CRAMTIME);
            SDL_Log("Sending ACK to %s\n", FormatAddr(theConnection).c_str());
            #if ROUTE_THRU_SERVER
                // if going through server, add the sender/dist so the server can figure out where to forward it
                *outData.c++ = myId;                        // sender
                *outData.w++ = (1 << theConnection->myId);  // distribution
            #endif
        }

        while (thePacket && thePacket != kPleaseSendAcknowledge) {
            PacketInfo *p;
            char *fp;
            uint8_t flags = 0;
            short len;

            p = &thePacket->packet;

            *outData.w++ = thePacket->serialNumber;
            fp = outData.c++;

            *outData.c++ = p->command;

            #if ROUTE_THRU_SERVER
                // if a client tries to send to anyone other than just the server
                if (!isServing && p->distribution != kdServerOnly) {
                    *outData.w++ = p->distribution;
                    flags |= 64;
                }
            #else
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

            #if ROUTE_THRU_SERVER
                if (!isServing && p->distribution != kdServerOnly) {  // any client to any other client
                    *outData.c++ = p->sender;
                    flags |= 128;
                }
            #else
                if (p->sender != myId) {
                    *outData.c++ = p->sender;
                    flags |= 128;
                }
            #endif

            *fp = flags;
            p->flags = flags;  // stick flags in the packet structure

            if (p->dataLen) {
                BlockMoveData(p->dataBuffer, outData.c, p->dataLen);
                outData.c += p->dataLen;
            }

            thePacket->packet.qLink = (PacketInfo *)packetList;
            packetList = thePacket;

            #if PACKET_DEBUG > 1 || ROUTE_THRU_SERVER // to see how flags get changed on ROUTE_THRU_SERVER
                SDL_Log("          preparing packet >>> cn=%d rsn=%d sn=%d cmd=%d p1=%d p2=%d p3=%d flags=0x%02x sndr=%d dist=0x%02x\n",
                        myId, *(short*)&udp->data[2], thePacket->serialNumber, p->command, p->p1, p->p2, p->p3, p->flags, p->sender, p->distribution);
            #endif

            // See if there are other messages that could be sent in this packet payload
            udp->len = outData.c - (char *)udp->data;
            if (udp->len < CRAMPACKSIZE && udp->len < theConnection->quota) {
                thePacket = theConnection->GetOutPacket(curTime, (cramCount-- > 0) ? CRAMTIME : 0, CRAMTIME);
            } else {
                thePacket = NULL;
            }
        }

        theConnection->quota -= kHeaderQuota;

        udp->address.host = theConnection->ipAddr;
        udp->address.port = theConnection->port;
        #if ROUTE_THRU_SERVER
            if (!isServing) {
                // if routing through the server, client uses the server host/port instead of direct-to-client
                udp->address.host = connections->ipAddr;
                udp->address.port = connections->port;
            }
            SDL_Log("           destination host is %s\n", FormatAddr(theConnection).c_str());
        #endif
        #if PACKET_DEBUG
            SDL_Log("     transmitting packet(s) to %s\n", FormatAddr(udp->address).c_str());
        #endif

        udp->len = outData.c - (char *)udp->data;

        *versionCheck = CRC16(udp->data, udp->len);
        // SDL_Log("... len=%d, CRC=0x%04x\n", udp->len, *versionCheck);

        theConnection->quota -= udp->len;

        curTime = GetClock();
        while (packetList && packetList != kPleaseSendAcknowledge) {
            thePacket = (UDPPacketInfo *)packetList->packet.qLink;

            if (packetList->sendCount++ == 0) {  //	This is the first time to send the packet

                packetList->birthDate = curTime;

                if (origCramCount) {
                    if (packetList->packet.flags & kpUrgentFlag) {
                        packetList->nextSendTime = curTime + theConnection->urgentRetransmitTime;
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

            #if PACKET_DEBUG > 1
                SDL_Log("queueing packet (sn=%d) for possible re-transmit in %dms\n", packetList->serialNumber, int(theConnection->retransmitTime/2.4));
            #endif
            Enqueue((QElemPtr)packetList, &theConnection->queues[kTransmitQ]);
            packetList = thePacket;
        }

        if (stream && theConnection->port) {
            #if PACKET_DEBUG > 1
                SDL_Log("   WRITING UDP packet to %s using stream %p\n",
                        FormatAddr(udp->address).c_str(), stream);
            #endif
            
            #if RANDOMLY_DROP_PACKETS
                if (rand() < RAND_MAX/256 || numToDrop > 0) {          // drop frequency = (1/N)
                    numToDrop = (numToDrop <= 0) ? 2 : numToDrop - 1;  // how many more to drop
                    SDL_Log("           ---------> DROPPING PACKET <---------\n");
                } else {
            #endif
            #if SIMULATE_LATENCY_ON_CLIENTS
                if ((1 << myId) & SIMULATE_LATENCY_DISTRIBUTION) {
                    usleep(SIMULATE_LATENCY_FORMULA); // simulate network latencies
                }
            #endif
            UDPWrite(stream, udp, UDPWriteComplete, this);
            #if RANDOMLY_DROP_PACKETS
                }
            #endif
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
    turboCount = 0;

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

OSErr CUDPComm::ContactServer(ip_addr serverHost, port_num serverPort) {
    if (noErr == CreateStream(localPort)) {
        long startTime;
        SDL_Event theEvent;

        seed = TickCount();
        connections->myId = 0;
        connections->port = serverPort;
        connections->ipAddr = serverHost;

        SDL_Log("Connecting to %s (seed=%ld) from port %d\n", FormatAddr(connections).c_str(), seed, localPort);

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
    Connect(address, "");
}

void CUDPComm::Connect(std::string address, std::string passwordStr) {
    SDL_Log("Connect address = %s pw length=%lu %s", address.c_str(), passwordStr.size(), passwordStr.c_str());

    OpenAvaraTCP();

    long serverPort = gApplication->Number(kDefaultUDPPort);
    // if kDefaultClientUDPPort not specified, fall back to kDefaultUDPPort
    localPort = gApplication->Number(kDefaultClientUDPPort, serverPort);

    IPaddress addr;
    CAvaraApp *app = (CAvaraAppImpl *)gApplication;
    SDLNet_ResolveHost(&addr, address.c_str(), serverPort);

    password[0] = passwordStr.length();
    BlockMoveData(passwordStr.c_str(), password + 1, passwordStr.length());
    
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

OSErr CUDPComm::CreateStream(port_num streamPort) {
    localPort = streamPort;
    if (stream) {
        DestroySocket(stream);
    }
    // SDL_Log("   CREATING SOCKET on port %d\n", localPort);
    stream = CreateSocket(localPort);
    // SDL_Log("   CREATED STREAM addr = %p\n", stream);

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
            if (conn->meanRoundTripTime > maxTrip) {
                maxTrip = conn->meanRoundTripTime;
            }
        }
    }

    maxTrip = maxTrip*1000/240;

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
