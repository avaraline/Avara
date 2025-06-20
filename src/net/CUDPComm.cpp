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
#define SIMULATE_LATENCY_LT 0.2
#define SIMULATE_LATENCY_MSEC_PER_LT 15  // less than 64 because of overhead of running multiple clients
#define SIMULATE_LATENCY_MEAN   (SIMULATE_LATENCY_LT*SIMULATE_LATENCY_MSEC_PER_LT*1000)
#define SIMULATE_LATENCY_JITTER  4
#define SIMULATE_LATENCY_DISTRIBUTION  0x02  // bitmask of who gets the latency

#define SIMULATE_LATENCY_CODE(text) \
if ((1 << myId) & SIMULATE_LATENCY_DISTRIBUTION) { \
    useconds_t zzz = 2*(SIMULATE_LATENCY_MEAN - SIMULATE_LATENCY_JITTER/2 + int((SIMULATE_LATENCY_JITTER)*float(rand())/RAND_MAX)); \
    SDL_Log("sleeping for %d msec on %s\n", zzz/1000, text); \
    usleep(zzz); \
}

#endif
#if RANDOMLY_DROP_PACKETS
#define RANDOM_DROP_DISTRIBUTION 0x02  // which clients drop packets
int numToDrop = 0;
#endif

#include "CUDPComm.h"

#include "AvaraTCP.h"
#include "CApplication.h"
#include "CRC.h"
#include "CUDPConnection.h"
#include "CharWordLongPointer.h"
#include "CommDefs.h"
#include "CommandList.h"
#include "Preferences.h"
#include "System.h"
#include "Debug.h"

#include <thread>


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

static std::string FormatAddr(IPaddress addr) {
    return FormatHostPort(addr.host, addr.port);
}
static std::string FormatAddr(CUDPConnection *conn) {
    return FormatHostPort(conn->ipAddr, conn->port);
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

std::string CUDPComm::FormatConnectionTable(CompleteAddress *table) {
    std::ostringstream oss;
    oss << " myId | Host:Port\n";
    oss << "------+---------------\n";
    for (int id = 1; id <= maxClients; id++, table++) {
        oss << "   " << id << "  | " << FormatHostPort(table->host, table->port) << "\n";
    }
    return oss.str();
}

std::string CUDPComm::FormatConnectionsList() {
    std::ostringstream oss;
    oss << " myId | Host:Port             | SN       | RSN\n";
    oss << "------+-----------------------+----------+----------\n";
    for (auto conn = connections; conn; conn = conn->next) {
        oss << "   " << ((conn->myId < 0) ? " " : std::to_string(conn->myId)) << "  | "
            << std::setw(21) << FormatHostPort(conn->ipAddr, conn->port) << " | "
            << std::setw(8) << conn->serialNumber << " | "
            << conn->receiveSerial << "\n";
    }
    return oss.str();
}


// Is the first part of the IP address "non-routable" (maybe a double-NAT or a LAN)?
// https://www.geeksforgeeks.org/non-routable-address-space/
//   10.0.0.0/8 ( Range: 10.0.0.0 – 10.255.255.255 )
//   172.16.0.0/12 ( Range: 172.16.0.0 – 172.31.255.255 )
//   192.168.0.0/16 ( Range: 192.168.0.0 – 192.168.255.255 )
bool CUDPComm::IsLAN(uint32_t host) {
    ip_addr ipaddr = SDL_SwapBE32(host);
    uint8_t ip1 = (ipaddr >> 24);
    uint8_t ip2 = ((ipaddr >> 16) & 0xff);
    return ((ip1 == 127) ||  // localhost loopback
            (ip1 == 10) ||
            (ip1 == 172 && ip2 >= 16 && ip2 <= 31) ||
            (ip1 == 192 && ip2 == 168));
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
                // server sends external host/IP because we don't want to mess with our already connected IP (which might be a LAN address)
                table->host = conn->ipAddrExt;
                table->port = conn->port;
            } else {
                table->host = 0;
                table->port = 0;
            }
            table++;
        }

        DBG_Log("login", "Sending Connection Table ...\n%s", FormatConnectionTable((CompleteAddress *)tablePack->dataBuffer).c_str());
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

        DBG_Log("login+", "sending  kpPacketProtocolControl from %d with cramData = %d to kdEveryone\n", myId, cramData);
        for (auto conn = connections; conn; conn = conn->next) {
            if (conn->port && !conn->killed) {
                DBG_Log("login+", "   kdEveryone includes %d (%s)\n", conn->myId, FormatHostPort(conn->ipAddrExt, conn->port).c_str());
            }
        }
        SendPacket(kdEveryone, kpPacketProtocolControl, udpCramInfo, cramData, 0, 0, NULL);

    }
}

// if a double NAT situation, replace IP address (192.x.x.x:PORT) in the connection table with
// the external IP address matching the port
void CUDPComm::ReplaceMatchingNAT(const IPaddress &addr) {
    CUDPConnection *conn;
    for (conn = connections; conn; conn = conn->next) {
        if (conn->port == addr.port && IsLAN(conn->ipAddrExt) && conn->ipAddrExt != addr.host) {
            // set external host to the host returned by punch and try sending the connection table again
            DBG_Log("login", "replacing NAT'd %s with punch server value %s\n",
                    FormatHostPort(conn->ipAddr, conn->port).c_str(),
                    FormatAddr(addr).c_str());
            conn->ipAddrExt = addr.host;
            SendConnectionTable();
            break;
        }
    }
    return;
}

void CUDPComm::PunchHandler(const PunchType cmd, const IPaddress &addr, const int8_t connId) {
    DBG_Log("punch", "CUDPComm::PunchHandler(%u, %s, %d})\n", cmd, FormatAddr(addr).c_str(), connId);
    switch (cmd) {
        case kPunch:
            PunchHole(addr, myId);
            if (isServing) {
                ReplaceMatchingNAT(addr);
            }
            break;
        case kHolePunch:
            // reset connection address for matching ID if different, then resend all messages
            for (CUDPConnection *conn = connections; conn; conn = conn->next) {
                if (conn->myId == connId) {
                    if (addr.host != conn->ipAddr || addr.port != conn->port) {
                        DBG_Log("punch", "   incoming address is different, updating connection from %s to %s\n",
                                FormatAddr(conn).c_str(), FormatAddr(addr).c_str());
                        conn->ipAddr = addr.host;
                        conn->port = addr.port;
                        conn->ResendNonValidatedPackets();
                        AsyncWrite();
                    }
                    break;
                }
            }
            break;
        default:
            break;
    }
}


void CUDPComm::SendRejectPacket(ip_addr remoteHost, port_num remotePort, OSErr loginErr) {
    SDL_Log("CUDPComm::SendRejectPacket\n");
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
            BlockMoveData(passwordStr.c_str(), password + 1, password[0]);

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

CUDPConnection *CUDPComm::UpdateConnectionMatchingSender(const UDPPacketInfo &thePacket, const IPaddress &newAddress) {
    CUDPConnection *conn = nullptr;
    const PacketInfo &pp = thePacket.packet;
    const uint16_t pSerial = uint16_t(thePacket.serialNumber);
    std::string formattedAddr = FormatAddr(newAddress);
    const char *addrStr = formattedAddr.c_str();

    // for now, only update for the first "several" packets (let the UDP connection settle)
    if (pSerial < SERIAL_NUMBER_UDP_SETTLE) {
        // unmatched early packets will include sender which helps us figure out which connection to match with
        for (conn = connections; conn; conn = conn->next) {
            // is this the matching connection number?
            if (pp.sender == conn->myId) break;
        }
        if (conn) {
            // don't change the connection address if we've received newer packets than this one (ignore old resends)
            if (pSerial >= conn->receiveSerial) {
                DBG_Log("login", "Setting connection address from an early packet, sndr=%d cmd=%d sn=%d addr: %s\n",
                        pp.sender, pp.command, pSerial, addrStr);
                // we found the connection, reset its IP address from the received packet
                conn->ipAddr = newAddress.host;
                conn->port = newAddress.port;
            }
        } else {
            SDL_Log("Got an EARLY packet with UNKNOWN SENDER, sndr=%d cmd=%d sn=%d addr: %s",
                    pp.sender, pp.command, pSerial, addrStr);
        }
        DBG_Log("login+", "Current connections list: \n%s\n", FormatConnectionsList().c_str());
//    } else {
//        SDL_Log("Got a packet from UNMATCHED ADDRESS, sndr=%d, cmd=%d, sn=%d addr: %s",
//                pp.sender, pp.command, pSerial, addrStr);
    }

    return conn;
}



void CUDPComm::ReadFromTOC(PacketInfo *thePacket) {
    // SDL_Log("CUDPComm::ReadFromTOC\n");
    CompleteAddress *table;

    myId = thePacket->p1;
    clientReady = true;

    table = (CompleteAddress *)thePacket->dataBuffer;
    DBG_Log("login", "Received Connection Table ...\n%s", FormatConnectionTable(table).c_str());

    CompleteAddress myAddressFromTOC = table[myId - 1];
    table[myId - 1] = {}; // don't want to connect to myself

    connections->MarkOpenConnections(table);
    // DBG_Log("login", "After removing open connections ...\n%s", FormatConnectionTable(table).c_str());

    connections->RewriteConnections(table, myAddressFromTOC);
    DBG_Log("login", "Final Connection Table, opening remaining connections:\n%s", FormatConnectionTable(table).c_str());

    connections->OpenNewConnections(table);
    // DBG_Log("login", "After opening connections ...\n%s", FormatConnectionTable(table).c_str());

    DBG_Log("login+", "After processing, connections list:\n%s", FormatConnectionsList().c_str());
}

Boolean CUDPComm::PacketHandler(PacketInfo *thePacket) {
    Boolean didHandle = true;

    // SDL_Log("CUDPComm::PacketHandler <<<<   cmd=%d p1=%d p2=%d p3=%d sndr=%d dist=0x%02hx\n",
    //         thePacket->command, thePacket->p1, thePacket->p2, thePacket->p3,
    //         thePacket->sender, thePacket->distribution);

    switch (thePacket->command) {
        case kpPacketProtocolReject:
            if (!isServing) {
                rejectReason = thePacket->p2;
            }
            break;
        case kpPacketProtocolTOC:
            if (!isServing && thePacket->p3 == seed) {
                ReadFromTOC(thePacket);
                DBG_Log("login+", "sending  kpPacketProtocolControl from %d with cramData = %d to kdEveryone\n", myId, cramData);
                for (auto conn = connections; conn; conn = conn->next) {
                    if (conn->port && !conn->killed) {
                        DBG_Log("login+", "   kdEveryone includes %d (%s)\n", conn->myId, FormatHostPort(conn->ipAddrExt, conn->port).c_str());
                    }
                }

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
            DBG_Log("login+", "received kpPacketProtocolControl from %d with cramData = %d\n", thePacket->sender, thePacket->p2);
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
        ClockTick curTime;
        UDPPacketInfo *thePacket;
        charWordLongP inData;
        char *inEnd;
        short inLen;

        #if PACKET_DEBUG > 2
            SDL_Log("CUDPComm::ReadComplete raw packet from=%s\n", FormatAddr(packet->address).c_str());
        #endif

        #if SIMULATE_LATENCY_ON_CLIENTS
            SIMULATE_LATENCY_CODE("read")
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
                        inData.c = conn->ValidatePackets(inData.c, curTime);
                        if (inLen == 7) {
                            // done processing ACK
                            inData.c = inEnd;
                        }
                    }
                #else
                    for (conn = connections; conn; conn = conn->next) {
                        if (conn->port == packet->address.port /*receivePB.csParam.receive.remotePort*/ &&
                            conn->ipAddr == packet->address.host /*receivePB.csParam.receive.remoteHost*/) {
                            inData.c = conn->ValidatePackets(inData.c, curTime);
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
                        thePacket->sendCount = *inData.uc++;
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

                        bool gotSender = false;
                        if (flags & 128) {
                            p->sender = *inData.c++;
                            gotSender = true;
                        } else {
                            p->sender = conn != nullptr ? conn->myId : 0;
                        }

                        #if PACKET_DEBUG > 1
                            SDL_Log("        CUDPComm::ReadComplete sn=%d-%hd cmd=%d flags=0x%02x sndr=%d dist=0x%02x\n",
                                    thePacket->serialNumber, thePacket->sendCount, p->command, p->flags, p->sender, p->distribution);
                        #endif

                        if (p->dataLen) {
                            if (p->dataLen > PACKETDATABUFFERSIZE || p->dataLen < 0) {
                                SDL_Log("CUDPComm::ReadComplete BUFFER SIZE ERROR (ignoring packet)!! sn=%d-%hd cmd=%d, sndr=%d dataLen = %d\n",
                                        int(thePacket->serialNumber), thePacket->sendCount, p->command, p->sender, p->dataLen);
                                ReleasePacket((PacketInfo *)thePacket);
                                break;
                            }
                            BlockMoveData(inData.c, p->dataBuffer, p->dataLen);
                            inData.c += p->dataLen;
                        }

                        // SDL_Log("  CUDPComm::ReadComplete  << num=%d cmd=%d p1=%d p2=%d p3=%d flags=%d sender=%d dist=0x%02x from %s\n",
                        //         thePacket->serialNumber, p->command, p->p1, p->p2, p->p3, p->flags, p->sender, p->distribution,
                        //         FormatAddr(packet->address).c_str());

                        if (conn && (!gotSender || p->sender == conn->myId)) {
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
                            // if we didn't find a connection OR the connection we found doesn't match the sender
                            if (isServing && thePacket->serialNumber == INITIAL_SERIAL_NUMBER &&
                                thePacket->packet.command == kpPacketProtocolLogin) {
                                // process a client Login packet
                                conn = DoLogin((PacketInfo *)thePacket, packet);

                                ReleasePacket((PacketInfo *)thePacket);
                            } else {
                                // look for a connection with myId==p->sender and update it to the packet's address
                                conn = UpdateConnectionMatchingSender(*thePacket, packet->address);

                                if (conn) {
                                    conn->ReceivedPacket(thePacket);
                                } else {
                                    ReleasePacket((PacketInfo *)thePacket);
                                }
                            }

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

size_t CUDPComm::SkipLostPackets(int16_t dist) {
    size_t sumRecvQs = 0;
    for (CUDPConnection *conn = connections; conn; conn = conn->next) {
        if ((1 << conn->myId) & dist) {
            sumRecvQs += conn->ReceivedPacket(nullptr);
        }
    }
    return sumRecvQs;
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
    CUDPConnection *theConnection;
    CUDPConnection *firstSender;
    UDPPacketInfo *thePacket = NULL;
    ClockTick curTime = GetClock();
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

        UDPpacket *udp = CreatePacket(UDPSENDBUFFERSIZE);

        outData.c = (char *)udp->data;
        versionCheck = outData.uw++;
        *versionCheck = softwareVersion;

        outData.c = theConnection->WriteAcks(outData.c);

        if (thePacket == kPleaseSendAcknowledge) {
            packetList = theConnection->GetOutPacket(curTime, (cramCount-- > 0) ? CRAMTIME : 0, CRAMTIME);
            #if PACKET_DEBUG > 1
                SDL_Log("Sending ACK to %s\n", FormatAddr(theConnection).c_str());
            #endif
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

            // increment global packet counter
            totalPacketsSent++;

            p = &thePacket->packet;

            *outData.uw++ = thePacket->serialNumber;
            *outData.uc++ = thePacket->sendCount;
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
                // transmit sender ID on initial packet in case they need that info to match up the connection
                if (p->sender != myId || thePacket->serialNumber <= SERIAL_NUMBER_UDP_SETTLE) {
                    *outData.c++ = p->sender;
                    flags |= 128;
                }
            #endif

            *fp = flags;
            p->flags = flags;  // stick flags in the packet structure

            if (p->dataLen) {
                if (p->dataLen > PACKETDATABUFFERSIZE) {
                    SDL_Log("CUDPComm::AsyncWrite BUFFER TOO BIG ERROR!! cmd=%d, sndr=%d dataLen = %d\n",
                            p->command, p->sender, p->dataLen);
                    p->dataLen = PACKETDATABUFFERSIZE;
                }
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
            udp->len = static_cast<uint32_t>(outData.c - (char *)udp->data);
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

        udp->len = static_cast<uint32_t>(outData.c - (char *)udp->data);

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
                if (((1 << myId) & RANDOM_DROP_DISTRIBUTION) && rand() < RAND_MAX/40 || numToDrop > 0) {          // drop frequency = (1/N)
                    numToDrop = (numToDrop <= 0) ? 2 : numToDrop - 1;  // how many more to drop
                    SDL_Log("           ---------> DROPPING PACKET <---------\n");
                } else {
            #endif
            // #if SIMULATE_LATENCY_ON_CLIENTS
            //     SIMULATE_LATENCY_CODE("write")
            // #endif
            UDPWrite(stream, udp, UDPWriteComplete, this);
            #if RANDOMLY_DROP_PACKETS
                }
            #endif
            result = true;
        }

        FreePacket(udp);
    } else {
        nextWriteTime = curTime + (urgentResendTime >> 2) + 1;
    }

    return result;
}

ClockTick CUDPComm::GetClock() {
    // Apparently this clock is about 240/second?
    // Upon further investigation, the original code,
    // 	  return lastClock = (microTime[0] << 20) | (microTime[1] >> 12);
    // shifted a usec time >> 12, effectively dividing by 4096.
    // So technically this was a 4.096 msec (4096 usec) clock tick (~244/sec).
    // To accomodate higher frame rates this will be adjusted down to 1 msec so that
    // the ratio of frameRate/clock is still approximately 16 with the 16 msec frameTime.
    return SDL_GetTicks() / MSEC_PER_GET_CLOCK;
}

/*
**	urgentTimePeriod is a time in milliseconds during which you expect to send
**	more urgent data. If not, any data marked urgent will be resent even if there
**	no other data to send within twice that period.
*/
void CUDPComm::IUDPComm(short clientCount, short bufferCount, uint16_t version, ClockTick urgentTimePeriod) {
    // create queues big enough to hold UDPPacketInfo packets
    InitializePacketQueues(bufferCount, sizeof(UDPPacketInfo));

    inviteString[0] = 0;

    softwareVersion = version;

    //	Convert time from milliseconds to 2*in our time units.
    // urgentResendTime = (urgentTimePeriod * 125) >> 8;
    //                  = (2 * urgentTimePeriod * 1000) >> 12;  // convert to usec then 2X GetClock interval
    // urgentResendTime represents 2 frames of time since urgentTimePeriod is passed as frameTime
    urgentResendTime = (urgentTimePeriod * 2 / MSEC_PER_GET_CLOCK);

    // latencyConvert = urgentTimePeriod * 2;  // no longer being used

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

    if (gApplication->Boolean(kPunchHoles)) {
        // this lambda will be called after a punch message (kPunch or kJab) is received
        SetPunchMessageHandler([this](PunchType cmd, const IPaddress &addr, int8_t connId) -> void {
            this->PunchHandler(cmd, addr, connId);
        });
    } else {
        SetPunchMessageHandler({});
    }


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

void CUDPComm::Dispose() {
    CUDPConnection *nextConn;

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

    CCommManager::Dispose();
}

void CUDPComm::CreateServer() {
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
}

OSErr CUDPComm::ContactServer(IPaddress &serverAddr) {
    if (noErr == CreateStream(localPort)) {
        long startTime;
        SDL_Event theEvent;

        // Before we "connect", notify the punch server so it can tell the host to open a hole to us
        if (!CUDPComm::IsLAN(serverAddr.host)) {
            RequestPunch(serverAddr);
        }

        seed = static_cast<int32_t>(TickCount());
        connections->myId = 0;
        connections->port = serverAddr.port;
        connections->ipAddr = serverAddr.host;

        SDL_Log("Connecting to %s (seed=%d) from port %d\n", FormatAddr(connections).c_str(), seed, localPort);

        AsyncRead();
        SendPacket(kdServerOnly, kpPacketProtocolLogin, 0, 0, seed, password[0] + 1, (Ptr)password);

        startTime = SDL_GetTicks();

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

            if ((SDL_GetTicks() - startTime) > kClientConnectTimeoutMsec) {
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
    SDL_Log("Connect address = %s pw length=%zu %s", address.c_str(), passwordStr.size(), passwordStr.c_str());

    OpenAvaraTCP();

    long serverPort = gApplication->Number(kDefaultUDPPort);
    // if kDefaultClientUDPPort not specified, fall back to 0 (random port) for the client
    localPort = gApplication->Number(kDefaultClientUDPPort, 0);

    IPaddress addr;
    // CAvaraApp *app = (CAvaraAppImpl *)gApplication;
    ResolveHost(&addr, address.c_str(), serverPort);

    password[0] = passwordStr.length();
    BlockMoveData(passwordStr.c_str(), password + 1, password[0]);

    ContactServer(addr);
}

void CUDPComm::StartServing() {
    CreateServer();
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
    ResolveHost(&addr, NULL, localPort);
    localIP = addr.host;

    // Register servers AND clients with punch server
    if (gApplication->Boolean(kPunchHoles)) {
        IPaddress localAddr = {htonl(localIP), htons(localPort)};
        RegisterPunchServer(localAddr);
    }

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

long CUDPComm::GetMaxRoundTrip(short distribution, float mult, short *slowPlayerId) {
    float maxTrip = 0;
    CUDPConnection *conn;
    // 1.3*stdev = 90.3% prob, 1.4=91.9%, 1.5=93.3%, 1.6=94.5
    if (mult < 0) {
        mult = 1.5;  // default
    }

    for (conn = connections; conn; conn = conn->next) {
        if (conn->port && (distribution & (1 << conn->myId))) {
            float stdev = sqrt(conn->varRoundTripTime);
            // add in mult*stdev but max it at CLASSICFRAMETIME (so we don't add more than 0.5 to LT)
            // note: is this really a erlang distribution?  if so, what's the proper equation?
            float rtt = conn->meanRoundTripTime + std::min<float>(mult*stdev, (1.0*CLASSICFRAMECLOCK));
            if (slowPlayerId != nullptr) {
                DBG_Log("rtt", "RTT[%d] = %.1f(%.2f) + min(%.1f * %.1f(%.2f), 64(0.5)) = %.1f(%.2fLT)\n",
                        conn->myId,
                        conn->meanRoundTripTime*MSEC_PER_GET_CLOCK,
                        conn->meanRoundTripTime*MSEC_PER_GET_CLOCK / (2*CLASSICFRAMETIME),
                        mult,
                        stdev*MSEC_PER_GET_CLOCK,
                        stdev*MSEC_PER_GET_CLOCK / (2*CLASSICFRAMETIME),
                        rtt*MSEC_PER_GET_CLOCK,
                        rtt*MSEC_PER_GET_CLOCK / (2*CLASSICFRAMETIME));
            }
            if (rtt > maxTrip) {
                maxTrip = rtt;
                if (slowPlayerId != nullptr) {
                    *slowPlayerId = conn->myId;
                }
            }
        }
    }

    // convert from GetClock() ticks to msec
    maxTrip = maxTrip*MSEC_PER_GET_CLOCK;

    return maxTrip;
}

float CUDPComm::GetMaxMeanSendCount(short distribution) {
    float maxCount = 0;
    for (CUDPConnection *conn = connections; conn; conn = conn->next) {
        if (conn->port && (distribution & (1 << conn->myId))) {
            maxCount = std::max(maxCount, conn->meanSendCount);
        }
    }
    return maxCount;
}

float CUDPComm::GetMaxMeanReceiveCount(short distribution) {
    float maxCount = 0;
    for (CUDPConnection *conn = connections; conn; conn = conn->next) {
        if (conn->port && (distribution & (1 << conn->myId))) {
            maxCount = std::max(maxCount, conn->meanReceiveCount);
        }
    }
    return maxCount;
}
