/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CCommManager.h
    Created: Monday, February 20, 1995, 23:57
    Modified: Wednesday, August 14, 1996, 19:03
*/

#pragma once
#include "CDirectObject.h"
#include "Memory.h"

#define TALKERSTRINGS 1000
#define PACKETDATABUFFERSIZE 128
#define MINIMUMBUFFERRESERVE 64
#define FRESHALLOCSIZE 64

#pragma pack(1)
typedef struct PacketInfo {
    struct PacketInfo *qLink;

    int16_t sender;
    int16_t distribution;
    char command;
    char p1;
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

class CCommManager : public CDirectObject {
public:
    short myId; //	Required/accessed publicly

    Ptr packetBuffers;
    QHdr freeQ;
    QHdr inQ;
    long freeCount;

    ReceiverRecord *firstReceivers[2]; //	Receiver queues

    short genericInfoTextRes;

    double frameTimeScale = 1.0;     // when time is slowed down, this number is > 1

    //	For method documentation, see .c-file:

    virtual void ICommManager(short packetSpace);
    virtual OSErr AllocatePacketBuffers(short numPackets);
    virtual void AddReceiver(ReceiverRecord *aReceiver, Boolean delayed);
    virtual void RemoveReceiver(ReceiverRecord *aReceiver, Boolean delayed);

    virtual OSErr SendPacket(short distribution, char command, char p1, short p2, long p3, short dataLen, Ptr dataPtr);
    virtual OSErr
    SendUrgentPacket(short distribution, char command, char p1, short p2, long p3, short dataLen, Ptr dataPtr);
    virtual void Dispose();

    virtual PacketInfo *GetPacket();
    virtual PacketInfo *DuplicatePacket(PacketInfo *original);

    virtual void ReleasePacket(PacketInfo *thePacket);

    virtual void DispatchPacket(PacketInfo *thePacket);
    virtual void DispatchAndReleasePacket(PacketInfo *thePacket);

    virtual void WriteAndSignPacket(PacketInfo *thePacket);
    virtual void ProcessQueue();
    virtual void DisconnectSlot(short slotId);

    virtual void OptionCommand(long theCommand);

    virtual short GetStatusInfo(short slotId, Handle leftColumn, Handle rightColumn);

    virtual Boolean ReconfigureAvailable();
    virtual void Reconfigure();

    virtual long GetMaxRoundTrip(short distribution);
};
