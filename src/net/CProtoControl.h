/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CProtoControl.h
    Created: Thursday, March 2, 1995, 22:32
    Modified: Wednesday, May 17, 1995, 21:34
*/

#pragma once
#include "CCommManager.h"

class CProtoControl {
public:
    CCommManager *itsManager;
    class CAvaraGame *theGame;
    ReceiverRecord immedReceiverRecord;
    ReceiverRecord delayedReceiverRecord;

    CProtoControl(CCommManager *aManager, CAvaraGame *aGame);
    virtual ~CProtoControl() {}
    
    virtual Boolean PacketHandler(PacketInfo *thePacket);
    virtual Boolean DelayedPacketHandler(PacketInfo *thePacket);

    virtual void Attach(CCommManager *aManager);
    virtual void Detach();
};
