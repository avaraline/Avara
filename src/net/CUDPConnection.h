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
    uint8_t _spare; // not sure if needed, just to be safe

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
    class CUDPConnection *next;
    class CUDPComm *itsOwner;

    QHdr queues[kQueueCount];

    long seed;

    ip_addr ipAddr;
    port_num port;

    short myId;

    SerialNumber serialNumber;
    SerialNumber receiveSerial;
    SerialNumber maxValid;

    Boolean haveToSendAck;
    long nextAckTime;
    long nextWriteTime;

    int32_t validTime;

    float meanRoundTripTime;
    float varRoundTripTime;
    float meanSendCount;
    float meanReceiveCount;
    SlidingHistogram<float>* latencyHistogram;

    long retransmitTime;
    long urgentRetransmitTime;

    long quota;
    short cramData;
    short busyQLen;

    short routingMask;

#if PACKET_DEBUG
    short dp;
    OSType d[kDebugBufferSize];
#endif

    long totalSent;
    long totalResent;
    long numResendsWithoutReceive;
    double recentResendRate;

    volatile short *offsetBufferBusy;
    int32_t ackBitmap;
    SerialNumber ackBase;

    Boolean killed;

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
    virtual void ReceivedPacket(UDPPacketInfo *thePacket);

    virtual void FlushQueues();
    virtual void Dispose();

    virtual void MarkOpenConnections(CompleteAddress *table);
    virtual void RewriteConnections(CompleteAddress *table);
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
