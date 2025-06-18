/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CCommManager.h
    Created: Monday, February 20, 1995, 23:57
    Modified: Wednesday, August 14, 1996, 19:03
*/

#pragma once
#include "Memory.h"
#include <list>
#include <vector>
#include "RolloverCounter.h"

#define TALKERSTRINGS 1000
#define PACKETDATABUFFERSIZE (1024-40)          // -40 to get UDPPacketInfo to 1024 bytes
#define MINIMUMBUFFERRESERVE 64*8
#define FRESHALLOCSIZE 64*8

#pragma pack(1)
typedef struct PacketInfo {
    struct PacketInfo *qLink;

    int16_t sender;
    int16_t distribution;
    int8_t command;
    int8_t p1;
    int16_t p2;
    int32_t p3;
    int16_t dataLen;
    int16_t flags;
    char dataBuffer[PACKETDATABUFFERSIZE];
} PacketInfo;
#pragma pack()

#define kpUrgentFlag (short)1

typedef Boolean PacketFuncType(PacketInfo *thePacket, Ptr userData);

struct ReceiverRecord {
    PacketFuncType *handler;
    Ptr userData;
    struct ReceiverRecord *nextReceiver;
};

typedef struct ReceiverRecord ReceiverRecord;

/*
**	CCommManager is an abstract class that manages packet buffers and
**	queues for its subclasses. Both clients and servers need its
**	functionality.
*/

class CCommManager {
public:
    short myId; //	Required/accessed publicly

    std::size_t packetSize;
    std::list<std::vector<std::byte>> packetBuffers;
    QHdr freeQ;
    QHdr inQ;

    ReceiverRecord *firstReceivers[2]; //	Receiver queues

    short genericInfoTextRes;

    RolloverCounter<uint32_t> totalPacketsSent = 0;

    //	For method documentation, see .c-file:

    CCommManager() : CCommManager(MINIMUMBUFFERRESERVE, sizeof(PacketInfo)) {}
    CCommManager(short packetSpace, size_t pSize);
    virtual ~CCommManager() {}

    void AllocatePacketBuffers(int numPackets);
    virtual void AddReceiver(ReceiverRecord *aReceiver, Boolean delayed);
    virtual void RemoveReceiver(ReceiverRecord *aReceiver, Boolean delayed);

    virtual OSErr SendPacket(short distribution, int8_t command, int8_t p1, int16_t p2, int32_t p3, int16_t dataLen, Ptr dataPtr, int16_t flags = 0);
    virtual OSErr SendUrgentPacket(short distribution, int8_t command, int8_t p1, int16_t p2, int32_t p3, int16_t dataLen, Ptr dataPtr);

    virtual PacketInfo *GetPacket();
    virtual PacketInfo *DuplicatePacket(PacketInfo *original);

    virtual void ReleasePacket(PacketInfo *thePacket);

    virtual void DispatchPacket(PacketInfo *thePacket);
    virtual void DispatchAndReleasePacket(PacketInfo *thePacket);

    virtual void WriteAndSignPacket(PacketInfo *thePacket);
    virtual void ProcessQueue();
    virtual void DisconnectSlot(short slotId);

    virtual void OptionCommand(long theCommand);

    virtual long GetMaxRoundTrip(short distribution, float stdMult = 0.0, short *slowPlayerId = nullptr);
    virtual float GetMaxMeanSendCount(short distribution);
    virtual float GetMaxMeanReceiveCount(short distribution);

    virtual const RolloverCounter<uint32_t>& TotalPacketsSent();
};
