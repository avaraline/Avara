/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CCommManager.c
    Created: Tuesday, February 21, 1995, 00:03
    Modified: Saturday, September 14, 1996, 05:24
*/

#include "CCommManager.h"

#include "Memory.h"
#include "System.h"
#include "Debug.h"

#include <SDL2/SDL.h>

/*
**	Initialize the packet buffer queues and allocate space
**	for buffers.
*/
void CCommManager::ICommManager(short packetSpace) {
    // base class creates PacketInfo buffers/queues
    InitializePacketQueues(packetSpace, sizeof(PacketInfo));
}

void CCommManager::InitializePacketQueues(int numPackets, std::size_t pSize) {
    packetSize = pSize;
    myId = 0; //	Default to server.

    freeQ.qHead = 0;
    freeQ.qTail = 0;

    inQ.qHead = 0;
    inQ.qTail = 0;

    firstReceivers[0] = NULL;
    firstReceivers[1] = NULL;

    AllocatePacketBuffers(numPackets);
}

void CCommManager::AllocatePacketBuffers(int numPackets) {
    // allocate enough space for numPackets packets
    packetBuffers.push_back(std::vector<std::byte>(numPackets*packetSize));

    // chop up the buffer into packetSize'd pieces and add them to the freeQ
    std::byte* pp = packetBuffers.back().data();
    while (numPackets--) {
        Enqueue((QElemPtr)pp, &freeQ);
        pp += packetSize;
    }
}

/*
**	Release allocated packet buffer storage and then dispose of self.
*/
void CCommManager::Dispose() {
    // SDL_Log("  - called Dispose with &inQ = %lx, &freeQ = %lx\n", &inQ, &freeQ);
    DisposeQueue(&freeQ);
    DisposeQueue(&inQ);
}

/*
**	Move parameters to a new packet buffer and use WriteAndSignPacket method to
**	send the packet.
*/
OSErr CCommManager::SendPacket(short distribution,
                               int8_t command,
                               int8_t p1,
                               int16_t p2,
                               int32_t p3,
                               int16_t dataLen,
                               Ptr dataPtr,
                               int16_t flags) {
    Ptr ps, pd;

    PacketInfo *thePacket;

    if (distribution) {
        thePacket = GetPacket();
        if (thePacket) {
            thePacket->flags = flags;
            thePacket->distribution = distribution;
            thePacket->command = command;
            thePacket->p1 = p1;
            thePacket->p2 = p2;
            thePacket->p3 = p3;

            if (dataLen > PACKETDATABUFFERSIZE || dataLen < 0) {
                SDL_Log("SENDPACKET ERROR, cmd=%d, dataLen = %d", command, dataLen);
                // will be caught by nanogui event handler
                throw std::length_error(std::string("Invalid packet buffer data size = ") + std::to_string(dataLen));
            }
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
                                     int8_t command,
                                     int8_t p1,
                                     int16_t p2,
                                     int32_t p3,
                                     int16_t dataLen,
                                     Ptr dataPtr) {
    return SendPacket(distribution, command, p1, p2, p3, dataLen, dataPtr, kpUrgentFlag);
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
 * Return an unused packet from the freeQ
 */
PacketInfo* CCommManager::GetPacket() {
    PacketInfo* thePacket = nullptr;

    while (thePacket == nullptr) {
        thePacket = (PacketInfo*)freeQ.qHead;
        if (thePacket) {
            Dequeue((QElemPtr)thePacket, &freeQ);
            break;
        } else {
            DBG_Log("q", "CCommManager::GetPacket myId=%d, allocating %d more packets, inQ size = %zu\n", myId, FRESHALLOCSIZE, QueueSize(&inQ));
            // no packets left? dynamically increase the freeQ then try again
            AllocatePacketBuffers(FRESHALLOCSIZE);
        }
    }

    return thePacket;
}

/*
**	Duplicate the contents of a packet buffer into
**	another one.
*/
PacketInfo* CCommManager::DuplicatePacket(PacketInfo* original) {
    PacketInfo* duplicate = GetPacket();
    if (duplicate) {
        *duplicate = *original;
    }
    return duplicate;
}

/*
**	After a packet buffer is no longer needed, it is
**	returned to the free queue.
*/
void CCommManager::ReleasePacket(PacketInfo *thePacket) {
    if (thePacket) {
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
