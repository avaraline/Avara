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

#define kSerialNumberStepSize 2
#define kNumReceivedOffsets 128

#define PACKET_DEBUG 0   // set to 1 for packet debug output, 2 for extra detail

#pragma pack(1)
typedef struct {
    PacketInfo packet;

    int32_t birthDate;
    // int32_t lastSendTime;
    int32_t nextSendTime;
    int16_t serialNumber;
    int16_t sendCount;

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

    short serialNumber;
    short receiveSerial;
    short maxValid;

    Boolean haveToSendAck;
    long nextAckTime;
    long nextWriteTime;

    long validTime;

    float meanRoundTripTime;
    float stableRoundTripTime;
    float varRoundTripTime;
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

    volatile short *offsetBufferBusy;
    int32_t ackBitmap;
    short ackBase;

    Boolean killed;

    virtual void IUDPConnection(CUDPComm *theMaster);
    virtual void SendQueuePacket(UDPPacketInfo *thePacket, short theDistribution);
    virtual void RoutePacket(UDPPacketInfo *thePacket);
    virtual UDPPacketInfo *GetOutPacket(long curTime, long cramTime, long urgencyAdjust);
    virtual UDPPacketInfo *FindBestPacket(long curTime, long cramTime, long urgencyAdjust);
    virtual void ProcessBusyQueue(long curTime);

    virtual char *WriteAcks(char *dest);
    virtual void RunValidate();

    virtual void ValidatePacket(UDPPacketInfo *thePacket, long when);
    virtual char *ValidateReceivedPackets(char *validateInfo, long curTime);
    virtual void ReceivedPacket(UDPPacketInfo *thePacket);

    virtual void FlushQueues();
    virtual void Dispose();

    virtual void MarkOpenConnections(CompleteAddress *table);
    virtual void OpenNewConnections(CompleteAddress *table);

    virtual Boolean AreYouDone();

    virtual void FreshClient(ip_addr remoteHost, port_num remotePort, long firstReceiveSerial);

#if PACKET_DEBUG
    virtual void DebugPacket(char eType, UDPPacketInfo *p);
#endif
    virtual void CloseSlot(short theId);
    virtual void ErrorKill();

    virtual void ReceiveControlPacket(PacketInfo *thePacket);

    virtual void GetConnectionStatus(short slot, UDPConnectionStatus *parms);
};
