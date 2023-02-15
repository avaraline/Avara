/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CCommManager.c
    Created: Tuesday, February 21, 1995, 00:03
    Modified: Saturday, September 14, 1996, 05:24
*/

#include "CCommManager.h"

#include "Memory.h"
#include "Resource.h"
#include "System.h"

#include <SDL.h>

/*
**	Initialize the packet buffer queues and allocate space
**	for buffers.
*/
void CCommManager::ICommManager(short packetSpace) {
    myId = 0; //	Default to server.
    packetBuffers = nullptr;

    freeQ.qFlags = 0;
    freeQ.qHead = 0;
    freeQ.qTail = 0;
    freeCount = 0;

    inQ.qFlags = 0;
    inQ.qHead = 0;
    inQ.qTail = 0;

    firstReceivers[0] = NULL;
    firstReceivers[1] = NULL;

    genericInfoTextRes = 400;

    AllocatePacketBuffers(packetSpace);
}

OSErr CCommManager::AllocatePacketBuffers(short packetSpace) {
    OSErr theErr;
    Ptr mem;
    PacketInfo *pp;

    mem = NewPtr(sizeof(Ptr) + sizeof(PacketInfo) * packetSpace);
    theErr = MemError();

    if (theErr == noErr) {
        *(Ptr *)mem = packetBuffers;  // point to the previous chunk of packet buffers, if any, so they are all removed in Dispose()
        packetBuffers = mem;

        pp = (PacketInfo *)(packetBuffers + sizeof(Ptr));

        while (packetSpace--) {
            Enqueue((QElemPtr)pp, &freeQ);
            freeCount++;
            pp++;
        }
    }

    return theErr;
}

/*
**	Release allocated packet buffer storage and then dispose of self.
*/
void CCommManager::Dispose() {
    Ptr nextDispose;

    while (packetBuffers) {
        nextDispose = *(Ptr *)packetBuffers;
        DisposePtr(packetBuffers);
        packetBuffers = nextDispose;
    }
    
    // SDL_Log("  - called Dispose with &inQ = %lx, &freeQ = %lx\n", &inQ, &freeQ);
    DisposeQueue(&freeQ);
    freeCount = 0;
    DisposeQueue(&inQ);
}

/*
**	Move parameters to a new packet buffer and use WriteAndSignPacket method to
**	send the packet.
*/
OSErr CCommManager::SendPacket(short distribution,
                               char command,
                               char p1,
                               int16_t p2,
                               int32_t p3,
                               int16_t dataLen,
                               Ptr dataPtr) {
    Ptr ps, pd;

    PacketInfo *thePacket;

    if (distribution) {
        thePacket = GetPacket();
        if (thePacket) {
            thePacket->flags = 0;
            thePacket->distribution = distribution;
            thePacket->command = command;
            thePacket->p1 = p1;
            thePacket->p2 = p2;
            thePacket->p3 = p3;

            if (dataLen > PACKETDATABUFFERSIZE)
                dataLen = PACKETDATABUFFERSIZE;
            thePacket->dataLen = dataLen;

            ps = dataPtr;
            pd = thePacket->dataBuffer;

            while (dataLen--) {
                *pd++ = *ps++;
            }

            WriteAndSignPacket(thePacket);
        } else {
            return tmfoErr;
        }
    }

    return noErr;
}

/*
**	Move parameters to a new packet buffer and use WriteAndSignPacket method to
**	send the packet.
*/
OSErr CCommManager::SendUrgentPacket(short distribution,
    char command,
    char p1,
    int16_t p2,
    int32_t p3,
    int16_t dataLen,
    Ptr dataPtr) {
    Ptr ps, pd;

    PacketInfo *thePacket;

    if (distribution) {
        thePacket = GetPacket();
        if (thePacket) {
            thePacket->flags = kpUrgentFlag;
            thePacket->distribution = distribution;
            thePacket->command = command;
            thePacket->p1 = p1;
            thePacket->p2 = p2;
            thePacket->p3 = p3;

            if (dataLen > PACKETDATABUFFERSIZE)
                dataLen = PACKETDATABUFFERSIZE;
            thePacket->dataLen = dataLen;

            ps = dataPtr;
            pd = thePacket->dataBuffer;

            while (dataLen--) {
                *pd++ = *ps++;
            }

            WriteAndSignPacket(thePacket);
        } else {
            return tmfoErr;
        }
    }

    return noErr;
}

/*
**	This default method is for the server, so
**	the way to handle it is to sign with id 0 and
**	then call the packet dispatch method. Clients
**	will override this method.
*/
void CCommManager::WriteAndSignPacket(PacketInfo *thePacket) {
    thePacket->sender = 0;
    if (thePacket->distribution)
        DispatchPacket(thePacket);
    ReleasePacket(thePacket);
}

/*
**	Register a packet receiver (a handler)
**
**	When a packet is received, it is first given to
**	the registered interrupt level receivers. If none
**	of them handle the packet, the next time ProcessQueue
**	is called, the delayed handlers get a chance to try
**	to handle the packet.
*/
void CCommManager::AddReceiver(ReceiverRecord *theReceiver, Boolean delayed) {
    theReceiver->nextReceiver = firstReceivers[delayed];
    firstReceivers[delayed] = theReceiver;
}

/*
**	Remove a registered receiver function.
*/
void CCommManager::RemoveReceiver(ReceiverRecord *theReceiver, Boolean delayed) {
    ReceiverRecord *ep, **px;

    ep = firstReceivers[delayed];
    px = &(firstReceivers[delayed]);

    while (ep != NULL) {
        if (ep == theReceiver) {
            *px = theReceiver->nextReceiver;
            ep = NULL;
        } else {
            px = &ep->nextReceiver;
            ep = ep->nextReceiver;
        }
    }
}

/*
**	Get a free packet buffer from the queue.
*/
PacketInfo *CCommManager::GetPacket() {
    PacketInfo *thePacket;
    OSErr iErr;

    do {
        iErr = noErr;
        thePacket = (PacketInfo *)freeQ.qHead;

        if (thePacket) {
            iErr = Dequeue((QElemPtr)thePacket, &freeQ);
        }
    } while (iErr == qErr);

    {
        freeCount--;
        thePacket->flags = 0;
    }

    return thePacket;
}

/*
**	Duplicate the contents of a packet buffer into
**	another one.
*/
PacketInfo *CCommManager::DuplicatePacket(PacketInfo *original) {
    PacketInfo *duplicate;

    duplicate = GetPacket();
    if (duplicate) {
        duplicate->flags = original->flags;
        duplicate->command = original->command;
        duplicate->sender = original->sender;
        duplicate->distribution = original->distribution;
        duplicate->p1 = original->p1;
        duplicate->p2 = original->p2;
        duplicate->p3 = original->p3;
        duplicate->dataLen = original->dataLen;
        if (duplicate->dataLen) {
            if (duplicate->dataLen > PACKETDATABUFFERSIZE) {
                SDL_Log("CCommManager::DuplicatePacket BUFFER TOO BIG ERROR!! cmd=%d, sndr=%d dataLen = %d\n",
                        duplicate->command, duplicate->sender, duplicate->dataLen);
                duplicate->dataLen = PACKETDATABUFFERSIZE;
            }
            BlockMoveData(original->dataBuffer, duplicate->dataBuffer, duplicate->dataLen);
        }
    }

    return duplicate;
}

/*
**	After a packet buffer is no longer needed, it is
**	returned to the free queue.
*/
void CCommManager::ReleasePacket(PacketInfo *thePacket) {
    if (thePacket) {
        freeCount++;
        Enqueue((QElemPtr)thePacket, &freeQ);
    }
}

/*
**	This is the dispatcher routine that can be called from
**	within an interrupt. It offers the packet to immediate packet
**	handlers. If it is not handled, it is queued so that the
**	next time ProcessQueue is called, the delayed handlers
**	can have a look at it.
*/
void CCommManager::DispatchPacket(PacketInfo *thePacket) {
    ReceiverRecord *receiver;
    Boolean didHandle = false;

    if (thePacket->distribution & (1 << myId) || myId == -1) {
        for (receiver = firstReceivers[1]; receiver; receiver = receiver->nextReceiver) {
            didHandle = receiver->handler(thePacket, receiver->userData);
            if (didHandle)
                break;
        }

        if (!didHandle && firstReceivers[0]) {
            thePacket = DuplicatePacket(thePacket);
            if (thePacket) {
                Enqueue((QElemPtr)thePacket, &inQ);
            }
        }
    }
}

/*
**	This is the dispatcher routine that can be called from
**	within an interrupt. It offers the packet to immediate packet
**	handlers. If it is not handled, it is queued so that the
**	next time ProcessQueue is called, the delayed handlers
**	can have a look at it.
*/
void CCommManager::DispatchAndReleasePacket(PacketInfo *thePacket) {
    ReceiverRecord *receiver;
    Boolean didHandle = false;

    // SDL_Log("CCommManager::DispatchAndReleasePacket cmd=%d sender=%d distribution=%d\n", thePacket->command,
    // thePacket->sender, thePacket->distribution);
    if (thePacket->distribution & (1 << myId) || myId == -1) {
        for (receiver = firstReceivers[1]; receiver; receiver = receiver->nextReceiver) {
            didHandle = receiver->handler(thePacket, receiver->userData);
            if (didHandle)
                break;
        }

        if (!didHandle && firstReceivers[0]) {
            if (thePacket) {
                Enqueue((QElemPtr)thePacket, &inQ);
                return;
            }
        }
    }

    ReleasePacket(thePacket);
}

/*
**	Go through the incoming queue and feed the packets to
**	the delayed packet handlers. Note that more packets
**	can arrive while this method is still executing and
**	that they will be handled within this same call.
*/
void CCommManager::ProcessQueue() {
    PacketInfo *thePacket;
    ReceiverRecord *receiver;
    Boolean didHandle;

    if (freeCount < MINIMUMBUFFERRESERVE) {
        AllocatePacketBuffers(FRESHALLOCSIZE);
    }

    do {
        thePacket = (PacketInfo *)inQ.qHead;
        if (thePacket) {
            short iErr;

            iErr = Dequeue((QElemPtr)thePacket, &inQ);

            if (iErr == noErr) {
                for (receiver = firstReceivers[0]; receiver; receiver = receiver->nextReceiver) {
                    didHandle = receiver->handler(thePacket, receiver->userData);
                    if (didHandle)
                        break;
                }
                ReleasePacket(thePacket);
            }
        }
    } while (thePacket);
}

/*
**	Abstract routine that is used to clean up after
**	a slot has been disconnected. The default action
**	is to do nothing.
*/
void CCommManager::DisconnectSlot(short slotId) {}

void CCommManager::OptionCommand(long theCommand) {}

short CCommManager::GetStatusInfo(short slot, Handle leftColumn, Handle rightColumn) {
    /*
    Handle		genericInfo;

    genericInfo = GetResource('TEXT', genericInfoTextRes);
    HLock(genericInfo);

    PtrToXHand(*genericInfo, leftColumn, GetHandleSize(genericInfo));

    HUnlock(genericInfo);
    */
    return 0;
}

Boolean CCommManager::ReconfigureAvailable() {
    return false;
}

void CCommManager::Reconfigure() {}

long CCommManager::GetMaxRoundTrip(short distribution, short *slowPlayerId) {
    return 0; //	Local net.
}
float CCommManager::GetMaxMeanSendCount(short distribution) {
    return 0; //	Local net.
}
float CCommManager::GetMaxMeanReceiveCount(short distribution) {
    return 0; //	Local net.
}
