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
#include "Types.h"

#include <string>

#define ROUTER_CAPABLE

#define CRAMTIME 5000 //	About 20 seconds.
#define CRAMPACKSIZE 64
#define kClientConnectTimeoutTicks 600 //(60*30)

enum { udpCramInfo }; //	Selectors for kpPacketProtocolControl packer p1 params.

class CUDPComm : public CCommManager {
public:
    long seed;
    short softwareVersion;
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

    class CTagBase *prefs;
    class CUDPConnection *connections;
    class CUDPConnection *nextSender;
    /*
    class	CTracker			*tracker;
    */

    long retransmitToRoundTripRatio; //	In fixed point 24.8 format

    long nextWriteTime;
    long latencyConvert;
    long urgentResendTime;
    long lastClock;
    long lastQuotaTime;

    ip_addr localIP; //	Just a guess, but that's all we need for the tracker.
    port_num localPort;
    UDPsocket stream;
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

    // Boolean				readIsComplete;
    // Boolean				writeIsComplete;
    // Boolean				blockReadComplete;
    // Boolean				blockWriteComplete;
    Boolean specialWakeup;
    Str255 inviteString;

    virtual void IUDPComm(short clientCount, short bufferCount, short version, long urgentTimePeriod);
    virtual OSErr AllocatePacketBuffers(short numPackets);
    virtual void Disconnect();
    virtual void WritePrefs();
    virtual void Dispose();

    long GetClock();

    virtual void ReadComplete(UDPpacket *packet);
    virtual void WriteComplete(int result);

    virtual void WriteAndSignPacket(PacketInfo *thePacket);
    virtual void ForwardPacket(PacketInfo *thePacket);
    virtual void ProcessQueue();

    virtual void SendConnectionTable();
    virtual void ReadFromTOC(PacketInfo *thePacket);

    virtual void SendRejectPacket(ip_addr remoteHost, port_num remotePort, OSErr loginErr);

    virtual CUDPConnection *DoLogin(PacketInfo *thePacket, UDPpacket *udp);

    virtual Boolean PacketHandler(PacketInfo *thePacket);

    virtual OSErr AsyncRead();
    virtual Boolean AsyncWrite();

    virtual void ReceivedGoodPacket(PacketInfo *thePacket);

    virtual OSErr CreateStream(port_num streamPort);

    virtual void CreateServer();
    virtual OSErr ContactServer(ip_addr serverHost, port_num serverPort);

    virtual Boolean ServerSetupDialog(Boolean disableSome);

    virtual void Connect(std::string address); //	Client
    virtual void Connect(std::string address, std::string passwordStr);

    virtual void StartServing(); //	Server.

    virtual void OptionCommand(long theCommand);
    virtual void DisconnectSlot(short slotId);

    virtual short GetStatusInfo(short slotId, Handle leftColumn, Handle rightColumn);

    virtual Boolean ReconfigureAvailable();
    virtual void Reconfigure();
    virtual long GetMaxRoundTrip(short distribution);

    virtual void BuildServerTags();
};
