/*
    Copyright ©1996-1998, Juri Munkki
    All rights reserved.

    File: CUDPComm.h
    Created: Monday, January 29, 1996, 13:32
    Modified: Saturday, January 3, 1998, 01:06
*/

#pragma once
#include "AvaraTCP.h"
#include "CCommManager.h"
#include "CommDefs.h"
#include "Types.h"
#include "CUDPConnection.h"

#include <string>

#define INITIAL_SERIAL_NUMBER     SerialNumber(0)  // must be even

#define ROUTE_THRU_SERVER 0  // non-zero to route all messages through the server

#define CRAMTIME 5000 //	About 20 seconds.
#define CRAMPACKSIZE 64
#define kClientConnectTimeoutTicks 600 //(60*30)

// Should be at most half of the lowest frameTime.  In classic game this was 4.096 ms.
#define MSEC_PER_GET_CLOCK (1)
#define CLASSICFRAMECLOCK (CLASSICFRAMETIME / MSEC_PER_GET_CLOCK)  // classic frameTime (64ms) in units of GetClock()

enum { udpCramInfo }; //	Selectors for kpPacketProtocolControl packer p1 params.

class CUDPComm : public CCommManager {
public:
    int32_t seed;
    uint16_t softwareVersion;
    short maxClients;
    short clientLimit;
    Str255 password;

    /*
    UDPiopb				sendPB;
    UDPiopb				receivePB;
    UDPiopb				rejectPB;
    char				rejectData[32];
    wdsEntry			rejectDataTable[2];

    wdsEntry            writeDataTable[2];

    UDPIOCompletionUPP	readComplete;
    UDPIOCompletionUPP	writeComplete;
    UDPIOCompletionUPP	bufferReturnComplete;
    */

    ReceiverRecord receiverRecord;

    // OSErr				writeErr;
    // OSErr				readErr;

    // class CTagBase *prefs;
    class CUDPConnection *connections;
    class CUDPConnection *nextSender;
    /*
    class	CTracker			*tracker;
    */

    long retransmitToRoundTripRatio; //	In fixed point 24.8 format

    ClockTick nextWriteTime;
    long latencyConvert;
    ClockTick urgentResendTime;
    ClockTick lastClock;
    ClockTick lastQuotaTime;

    ip_addr localIP; //	Just a guess, but that's all we need for the tracker.
    port_num localPort;
    int stream;
    // char				streamBuffer[UDPSTREAMBUFFERSIZE];
    // char				sendBuffer[UDPSENDBUFFERSIZE];

    short cramData;

    volatile OSErr rejectReason;
    volatile Boolean clientReady;
    Boolean isConnected;
    Boolean isServing;
    Boolean isClosed;
    Boolean isClosing;
    Boolean turboMode;
    long turboCount;

    // Boolean				readIsComplete;
    // Boolean				writeIsComplete;
    // Boolean				blockReadComplete;
    // Boolean				blockWriteComplete;
    Boolean specialWakeup;
    Str255 inviteString;

    virtual void IUDPComm(short clientCount, short bufferCount, uint16_t version, ClockTick urgentTimePeriod);

    virtual void Disconnect();
    virtual void WritePrefs();
    virtual void Dispose();

    ClockTick GetClock();

    virtual void ReadComplete(UDPpacket *packet);
    virtual void WriteComplete(int result);

    virtual void WriteAndSignPacket(PacketInfo *thePacket);
    #if ROUTE_THRU_SERVER
    virtual void FastForwardPacket(UDPpacket *udp, int16_t distribution);
    virtual CUDPConnection * ConnectionForPacket(UDPpacket *udp);
    #endif
    virtual void ForwardPacket(PacketInfo *thePacket);
    virtual void ProcessQueue();

    virtual std::string FormatConnectionTable(CompleteAddress *table);
    virtual bool IsDoubleNAT(uint32_t host);
    virtual void SendConnectionTable();
    virtual void ReplaceMatchingNAT(const IPaddress &addr);
    virtual void ReadFromTOC(PacketInfo *thePacket);

    virtual void SendRejectPacket(ip_addr remoteHost, port_num remotePort, OSErr loginErr);

    virtual CUDPConnection *DoLogin(PacketInfo *thePacket, UDPpacket *udp);

    virtual Boolean PacketHandler(PacketInfo *thePacket);

    virtual OSErr AsyncRead();
    virtual Boolean AsyncWrite();

    virtual void ReceivedGoodPacket(PacketInfo *thePacket);
    virtual size_t SkipLostPackets(int16_t dist);

    virtual OSErr CreateStream(port_num streamPort);

    virtual void CreateServer();
    virtual OSErr ContactServer(IPaddress &serverAddr);

    virtual Boolean ServerSetupDialog(Boolean disableSome);

    virtual void Connect(std::string address); //	Client
    virtual void Connect(std::string address, std::string passwordStr);

    virtual void StartServing(); //	Server.

    virtual void OptionCommand(long theCommand);
    virtual void DisconnectSlot(short slotId);

    virtual short GetStatusInfo(short slotId, Handle leftColumn, Handle rightColumn);

    virtual Boolean ReconfigureAvailable();
    virtual void Reconfigure();
    virtual long GetMaxRoundTrip(short distribution, float multiplier = 0.0, short *slowPlayerId = nullptr);
    virtual float GetMaxMeanSendCount(short distribution);
    virtual float GetMaxMeanReceiveCount(short distribution);

    virtual void BuildServerTags();
};
