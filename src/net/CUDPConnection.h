/*
    Copyright ©1996-1998, Juri Munkki
    All rights reserved.

    File: CUDPConnection.h
    Created: Monday, January 29, 1996, 13:35
    Modified: Saturday, January 3, 1998, 01:07
*/

#pragma once
#include "CCommManager.h"
#include "CDirectObject.h"
#include "CommDefs.h"
#include "RolloverCounter.h"
#include "SlidingHistogram.h"

#define kSerialNumberStepSize 2
#define kNumReceivedOffsets 128

typedef RolloverCounter<uint16_t> SerialNumber;

#define PACKET_DEBUG 0   // set to 1 for packet debug output, 2 for extra detail

#pragma pack(1)
typedef struct {
    PacketInfo packet;

    int32_t birthDate;
    // int32_t lastSendTime;
    int32_t nextSendTime;
    SerialNumber serialNumber;
    uint8_t sendCount;
    uint8_t _spares[5]; // for 8-byte alignment in queues

} UDPPacketInfo;
#pragma pack()

typedef struct {
    ip_addr host;
    port_num port;
} CompleteAddress;

typedef struct {
    long hostIP;

    long estimatedRoundTrip;
    long averageRoundTrip;
    long pessimistRoundTrip;
    long optimistRoundTrip;

    short connectionType;
} UDPConnectionStatus;

#define kDebugBufferSize 1024

#define kPleaseSendAcknowledge ((UDPPacketInfo *)-1L)

enum { kReceiveQ, kTransmitQ, kBusyQ, kQueueCount };

class CUDPConnection : public CDirectObject {
public:
    class CUDPConnection *next = 0;
    class CUDPComm *itsOwner = 0;

    QHdr queues[kQueueCount] = {0};

    int32_t seed = 0;

    ip_addr ipAddr = 0;
    port_num port = 0;

    short myId = 0;

    SerialNumber serialNumber = 0;
    SerialNumber receiveSerial = 0;
    SerialNumber maxValid = 0;

    Boolean haveToSendAck = 0;
    ClockTick nextAckTime = 0;
    ClockTick nextWriteTime = 0;

    ClockTick validTime = 0;

    float meanRoundTripTime = 0;
    float varRoundTripTime = 0;
    float meanSendCount = 0;
    float meanReceiveCount = 0;
    SlidingHistogram<float> *latencyHistogram = {0};

    ClockTick retransmitTime = 0;
    ClockTick urgentRetransmitTime = 0;

    long quota = 0;
    short cramData = 0;
    short busyQLen = 0;

    short routingMask = 0;

#if PACKET_DEBUG
    short dp;
    OSType d[kDebugBufferSize];
#endif

    long totalSent = 0;
    long totalResent = 0;
    long numResendsWithoutReceive = 0;
    double recentResendRate = 0;

    volatile short *offsetBufferBusy = 0;
    int32_t ackBitmap = 0;
    SerialNumber ackBase = 0;

    Boolean killed = 0;

    virtual void IUDPConnection(CUDPComm *theMaster);
    virtual void SendQueuePacket(UDPPacketInfo *thePacket, short theDistribution);
    virtual void RoutePacket(UDPPacketInfo *thePacket);
    virtual UDPPacketInfo *GetOutPacket(int32_t curTime, int32_t cramTime, int32_t urgencyAdjust);
    virtual UDPPacketInfo *FindBestPacket(int32_t curTime, int32_t cramTime, int32_t urgencyAdjust);
    virtual void ProcessBusyQueue(int32_t curTime);

    virtual char *WriteAcks(char *dest);
    virtual void RunValidate();

    virtual void ValidatePacket(UDPPacketInfo *thePacket, int32_t when);
    virtual void ValidateReceivedPacket(UDPPacketInfo *thePacket);
    virtual char *ValidatePackets(char *validateInfo, int32_t curTime);

    virtual size_t ReceivedPacket(UDPPacketInfo *thePacket);
    virtual bool ReceiveQueuedPackets();

    virtual void FlushQueues();
    virtual void Dispose();

    virtual void MarkOpenConnections(CompleteAddress *table);
    virtual void RewriteConnections(CompleteAddress *table, const CompleteAddress &myAddressInTOC);
    virtual void OpenNewConnections(CompleteAddress *table);

    virtual Boolean AreYouDone();

    virtual void FreshClient(ip_addr remoteHost, port_num remotePort, uint16_t firstReceiveSerial);

#if PACKET_DEBUG || LATENCY_DEBUG
    virtual void DebugPacket(char eType, UDPPacketInfo *p);
#endif
    virtual void CloseSlot(short theId);
    virtual void ErrorKill();

    virtual void ReceiveControlPacket(PacketInfo *thePacket);

    virtual void GetConnectionStatus(short slot, UDPConnectionStatus *parms);

private:
    long LatencyEstimate();
};
