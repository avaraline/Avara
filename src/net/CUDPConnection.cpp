/*
    Copyright ©1996-1998, Juri Munkki
    All rights reserved.

    File: CUDPConnection.c
    Created: Monday, January 29, 1996, 13:36
    Modified: Saturday, January 3, 1998, 02:14
*/

#include "CUDPConnection.h"

#include "CUDPComm.h"
#include "CommDefs.h"
#include "System.h"

#define kInitialRetransmitTime 480 //	2	seconds
#define kInitialRoundTripTime 240 //	<1 second
#define kMaxAllowedRetransmitTime 960 //	4 seconds
#define kAckRetransmitBase 10
#define kULPTimeOut (7324 * 4) //	30*4 seconds
#define kMaxTransmitQueueLength 128 //	128 packets going out...
#define kMaxReceiveQueueLength 32 //	32 packets...arbitrary guess

#define RTTSMOOTHFACTOR_UP 10
#define RTTSMOOTHFACTOR_DOWN 20

#define MAX_RESENDS_WITHOUT_RECEIVE 2

#if PACKET_DEBUG
void CUDPConnection::DebugPacket(char eType, UDPPacketInfo *p) {
    SDL_Log("CUDPConnection::DebugPacket(%c) cn=%d rsn=%d sn=%d cmd=%d p1=%d p2=%d p3=%d flags=0x%02x sndr=%d dist=0x%02x\n",
        eType,
        myId,
        receiveSerial,
        p->serialNumber,
        p->packet.command,
        p->packet.p1,
        p->packet.p2,
        p->packet.p3,
        p->packet.flags,
        p->packet.sender,
        p->packet.distribution);
}
#endif

void CUDPConnection::IUDPConnection(CUDPComm *theOwner) {
    OSErr err = noErr;
    short i;

    killed = false;
    itsOwner = theOwner;
    myId = -1; //	Unknown

    ipAddr = 0;
    port = 0;

    for (i = 0; i < kQueueCount; i++) {
        queues[i].qFlags = 0;
        queues[i].qHead = 0;
        queues[i].qTail = 0;
    }

    serialNumber = 0;
    receiveSerial = 0;
    maxValid = -kSerialNumberStepSize;

    retransmitTime = kInitialRetransmitTime;
    urgentRetransmitTime = kInitialRoundTripTime;
    meanRoundTripTime = kInitialRoundTripTime;
    stableRoundTripTime = kInitialRoundTripTime;
    varRoundTripTime = meanRoundTripTime*meanRoundTripTime;
    haveToSendAck = false;
    nextAckTime = 0;

    nextWriteTime = 0;
    validTime = 0;

    cramData = 0;
    seed = 0;

    next = NULL;

    ackBase = 0;
    offsetBufferBusy = NULL;
    ackBitmap = 0;

    busyQLen = 0;
    quota = 0;

    routingMask = 0;
    
    totalSent = 0;
    totalResent = 0;
    numResendsWithoutReceive = 0;
}

void CUDPConnection::FlushQueues() {
    QElemPtr elem;
    QHdr *head;
    short i;

    ackBase = 0;
    offsetBufferBusy = NULL;

    ackBitmap = 0;

    for (i = 0; i < kQueueCount; i++) {
        head = &queues[i];
        do {
            elem = head->qHead;
            if (elem) {
                short theErr;

                theErr = Dequeue(elem, head);
                if (theErr == noErr) {
                    itsOwner->ReleasePacket((PacketInfo *)elem);
                }
            }
        } while (elem);
    }

    busyQLen = 0;
}

void CUDPConnection::SendQueuePacket(UDPPacketInfo *thePacket, short theDistribution) {
    //	Put the packet into the busy outgoing queue. It will be moved
    //	into the transmit queue very soon.
    thePacket = (UDPPacketInfo *)itsOwner->DuplicatePacket((PacketInfo *)thePacket);

    if (thePacket) {
        thePacket->packet.distribution = theDistribution;

        Enqueue((QElemPtr)thePacket, &queues[kBusyQ]);
        busyQLen++;
#if PACKET_DEBUG > 1
        if (thePacket->packet.command == kpKeyAndMouse) {
            thePacket->serialNumber = -1;  // assigned later
            DebugPacket('>', thePacket);
        }
#endif
    } else {
        SDL_Log("DuplicatePacket failed\n");
        ErrorKill();
    }
}

void CUDPConnection::RoutePacket(UDPPacketInfo *thePacket) {
    short extendedRouting = routingMask | (1 << myId);

    #if PACKET_DEBUG
        DebugPacket('=', thePacket);
    #endif
    SendQueuePacket(thePacket, thePacket->packet.distribution & extendedRouting);
    thePacket->packet.distribution &= ~extendedRouting;
}

void CUDPConnection::ProcessBusyQueue(long curTime) {
    UDPPacketInfo *thePacket = NULL;

    while ((thePacket = (UDPPacketInfo *)queues[kBusyQ].qHead)) {
        if (noErr == Dequeue((QElemPtr)thePacket, &queues[kBusyQ])) {
            thePacket->birthDate = curTime;
            thePacket->nextSendTime = curTime;
            thePacket->serialNumber = serialNumber;
            serialNumber += kSerialNumberStepSize;
            Enqueue((QElemPtr)thePacket, &queues[kTransmitQ]);
        } else { //	Debugger();
            break;
        }
    }

    busyQLen = 0;
}

UDPPacketInfo *CUDPConnection::FindBestPacket(long curTime, long cramTime, long urgencyAdjust) {
    UDPPacketInfo *thePacket = NULL;
    UDPPacketInfo *nextPacket;
    UDPPacketInfo *bestPacket;
    OSErr theErr;
    long bestSendTime, theSendTime;
    long oldestBirth;
    short transmitQueueLength = 0;

    bestPacket = (UDPPacketInfo *)queues[kTransmitQ].qHead;
    oldestBirth = curTime;
    
    while (bestPacket != NULL) {
        // make sure the we are actually beyond packet's nextSendTime, avoids extra resends
        if (curTime >= bestPacket->nextSendTime &&
            (bestPacket->birthDate == bestPacket->nextSendTime || numResendsWithoutReceive < MAX_RESENDS_WITHOUT_RECEIVE)) {
            break;
        }
        bestPacket = (UDPPacketInfo *)bestPacket->packet.qLink;
     }

    if (bestPacket) {
        while (bestPacket && (bestPacket->serialNumber - maxValid > kSerialNumberStepSize * kMaxReceiveQueueLength)) {
            transmitQueueLength++;
            bestPacket = (UDPPacketInfo *)bestPacket->packet.qLink;
        }

        if (bestPacket && bestPacket->birthDate != bestPacket->nextSendTime) {
            oldestBirth = bestPacket->birthDate;
        }
    }

    if (bestPacket) {
        bestSendTime = bestPacket->nextSendTime;

        if (bestPacket->packet.flags & kpUrgentFlag)
            bestSendTime -= urgencyAdjust;

        transmitQueueLength++;

        thePacket = (UDPPacketInfo *)bestPacket->packet.qLink;
        while (thePacket) {
            transmitQueueLength++;

            nextPacket = (UDPPacketInfo *)thePacket->packet.qLink;

            if (thePacket->serialNumber - maxValid <= kSerialNumberStepSize * kMaxReceiveQueueLength) {
                theSendTime = thePacket->nextSendTime;

                if (thePacket->birthDate - oldestBirth < 0 && thePacket->birthDate != theSendTime) {
                    oldestBirth = thePacket->birthDate;
                }

                if (thePacket->packet.flags & kpUrgentFlag)
                    theSendTime -= urgencyAdjust;

                #if PACKET_DEBUG > 1
                    DebugPacket('C', thePacket);  // 'C' for Candidate
                    SDL_Log(" CUDPConnection::FindBestPacket bestSendTime = %ld\n", bestSendTime);
                    SDL_Log(" CUDPConnection::FindBestPacket theSendTime = %ld\n", theSendTime);
                #endif
                // if this candidate packet has a smaller sendTime AND (it's never been sent OR we haven't reached the resend limit)
                if (bestSendTime > theSendTime &&
                    (thePacket->birthDate == thePacket->nextSendTime || numResendsWithoutReceive < MAX_RESENDS_WITHOUT_RECEIVE)) {
                    // this is the NEW bestPacket
                    bestSendTime = theSendTime;
                    bestPacket = thePacket;
                }
            }

            thePacket = nextPacket;
        }

        thePacket = NULL; //	It already is NULL from the loop.

        if (bestPacket->serialNumber - maxValid > 0) {
            if (bestSendTime - cramTime - curTime <= 0) {
                theErr = Dequeue((QElemPtr)bestPacket, &queues[kTransmitQ]);

                if (theErr == noErr) { //	An error could have occured if the packet had been
                    //	taken while we were looking at the queue. I
                    //	don't think it will ever happen, but it pays
                    //	to be careful.

                    thePacket = bestPacket;
                } else {
                    SDL_Log("Dequeue failed in FindBestPacket\n");
                }
            }
        } else {
            ValidatePacket(bestPacket, curTime);
        }
    }

    if (curTime - oldestBirth > kULPTimeOut && curTime - validTime > kULPTimeOut) {
        SDL_Log("Birthdate too old\n");
        ErrorKill();
    }

    if (transmitQueueLength > kMaxTransmitQueueLength) {
        SDL_Log("Transmit Queue Overflow\n");
        ErrorKill();
    }

    return thePacket;
}

UDPPacketInfo *CUDPConnection::GetOutPacket(long curTime, long cramTime, long urgencyAdjust) {
    UDPPacketInfo *thePacket = NULL;
    UDPPacketInfo *nextPacket;
    UDPPacketInfo *bestPacket;
    OSErr theErr;
    long bestSendTime, theSendTime;

    if (port) {
        ProcessBusyQueue(curTime);
        thePacket = FindBestPacket(curTime, cramTime, urgencyAdjust);
    }

    if (thePacket == NULL) { //	We don't have packets to send, but we might want to send a
        //	special ACK packet instead when the other end is sending
        //	data that we already got and it looks like we are not missing
        //	any packets. (The receive queue would have them.)

        if (haveToSendAck &&
            nextAckTime - curTime <= 0) { //	This is a pointer value of -1. No "useful" data is actually
            //	sent, but the acknowledge part of the UDP packet is sent.

            thePacket = kPleaseSendAcknowledge;
        }
    }

    if (thePacket) {
        haveToSendAck = false;
        nextAckTime = curTime + kAckRetransmitBase + retransmitTime;
        nextWriteTime = curTime + retransmitTime;

        if (thePacket == kPleaseSendAcknowledge) {
            #if PACKET_DEBUG
                SDL_Log("CUDPConnection::DebugPacket(S) <ACK> cn=%d rsn=%d\n", myId, receiveSerial - kSerialNumberStepSize);
            #endif
        } else {
            totalSent++;
            if (thePacket->birthDate != thePacket->nextSendTime) {
                totalResent++;
                numResendsWithoutReceive++;
                #if PACKET_DEBUG
                    SDL_Log("CUDPConnection::GetOutPacket   RESENDING cn=%d sn=%d, age=%ld, percentResends = %.1f, numResendsWithoutReceive = %ld\n",
                            myId, thePacket->serialNumber, curTime - thePacket->birthDate, 100.0*totalResent/totalSent, numResendsWithoutReceive);
                #endif
            }
            #if PACKET_DEBUG
                DebugPacket('S', thePacket);
            #endif
        }
    }

    return thePacket;
}

bool UseCommandForStats(long command) {
    // only use faster commands for stats
    switch(command) {
       case kpPing:
       case kpRosterMessage:
       case kpKeyAndMouse:
           return true;
    }
    return false;
}

void CUDPConnection::ValidatePacket(UDPPacketInfo *thePacket, long when) {
    #if PACKET_DEBUG
        DebugPacket('V', thePacket);
    #endif
    if (noErr == Dequeue((QElemPtr)thePacket, &queues[kTransmitQ])) {
        long roundTrip;

        roundTrip = when - thePacket->birthDate;

        if (maxValid == 4) {
            if (roundTrip < meanRoundTripTime) {
                meanRoundTripTime = roundTrip;
                varRoundTripTime = roundTrip * roundTrip;  // err on the high side initially
            }
        } else if (UseCommandForStats(thePacket->packet.command)) {
            // compute an exponential moving average & variance of the roundTrip time
            // see: https://fanf2.user.srcf.net/hermes/doc/antiforgery/stats.pdf
            float difference = roundTrip - meanRoundTripTime;
            // quicker to move up on latency spikes, slower to move down
            float alpha =  1.0 / ((difference > 0) ? RTTSMOOTHFACTOR_UP : RTTSMOOTHFACTOR_DOWN);
            float increment = alpha * difference;
            meanRoundTripTime = meanRoundTripTime + increment;
            varRoundTripTime = (1 - alpha) * (varRoundTripTime + difference * increment);
            float stdevRoundTripTime = sqrt(varRoundTripTime);

            // for display purposes, use a more stable slow-moving alpha (TBR)
            stableRoundTripTime = meanRoundTripTime + difference / RTTSMOOTHFACTOR_DOWN;

            // use +3.5 sigma(probability 99.9%) for retransmitTime, +3 sigma (99.7%) for urgentRetransmitTime
            // (thought: consider dynamically adjusting the multiplier based on % of resends?)
            retransmitTime = meanRoundTripTime + (long)(3.5*stdevRoundTripTime);
            urgentRetransmitTime = meanRoundTripTime + (long)(3.0*stdevRoundTripTime);
            
            // don't let the retransmit times fall below threshold based on frame rate or go above kMaxAllowedRetransmitTime
            retransmitTime = std::max(retransmitTime, itsOwner->urgentResendTime);
            retransmitTime = std::min(retransmitTime, (long)kMaxAllowedRetransmitTime);
            urgentRetransmitTime = std::max(urgentRetransmitTime, itsOwner->urgentResendTime);
            urgentRetransmitTime = std::min(urgentRetransmitTime, (long)kMaxAllowedRetransmitTime);

            #if PACKET_DEBUG
                SDL_Log("                               roundTrip=%ld mean=%.1f std = %.1f retransmitTime=%ld urgentRetransmit=%ld\n",
                        roundTrip, meanRoundTripTime, stdevRoundTripTime, retransmitTime, urgentRetransmitTime);
            #endif
        }

#if PACKET_DEBUG > 1
        if (thePacket->packet.command == kpKeyAndMouse) {
            DebugPacket('/', thePacket);
        }
        DebugPacket('X', thePacket);
#endif
        itsOwner->ReleasePacket((PacketInfo *)thePacket);
    }
}

void CUDPConnection::RunValidate() {
    UDPPacketInfo *thePacket, *nextPacket;

    thePacket = (UDPPacketInfo *)queues[kTransmitQ].qHead;
    #if PACKET_DEBUG > 1
        SDL_Log("   CUDPConnection::RunValidate maxValid=%d\n", maxValid);
    #endif
    while (thePacket) {
        nextPacket = (UDPPacketInfo *)thePacket->packet.qLink;

        if (thePacket->serialNumber <= maxValid) {
            ValidatePacket(thePacket, validTime);
        }

        thePacket = nextPacket;
    }
}

char *CUDPConnection::ValidateReceivedPackets(char *validateInfo, long curTime) {
    short transmittedSerial;
    short dummyStackVar;
    UDPPacketInfo *thePacket, *nextPacket;

    // received something, reset the counter
    numResendsWithoutReceive = 0;

    transmittedSerial = *(short *)validateInfo;
    validateInfo += sizeof(short);

    #if PACKET_DEBUG
        SDL_Log("ValidateReceivedPackets transmittedSerial=%d, maxValid = %d\n", transmittedSerial, maxValid);
    #endif

    if (transmittedSerial & 1) {
        int32_t ackMap;

        transmittedSerial &= ~1;

        ackMap = *(int32_t *)validateInfo;
        #if PACKET_DEBUG
            SDL_Log("           ------------ AckMap = 0x%04x ----------\n", ackMap);
        #endif

        for (thePacket = (UDPPacketInfo *)queues[kTransmitQ].qHead; thePacket; thePacket = nextPacket) {
            nextPacket = (UDPPacketInfo *)thePacket->packet.qLink;

            if (ackMap & (1 << (((thePacket->serialNumber - transmittedSerial) >> 1) - 1))) {
                ValidatePacket(thePacket, curTime);
            }
        }

        validateInfo += sizeof(ackBitmap);
    }

    validTime = curTime;

    if (maxValid < transmittedSerial) {
        maxValid = transmittedSerial;
        RunValidate();
    }

    return validateInfo;
}

void CUDPConnection::Dispose() {
    FlushQueues();

    CDirectObject::Dispose();
}

static short receiveSerialStorage[512];

void CUDPConnection::ReceivedPacket(UDPPacketInfo *thePacket) {
    UDPPacketInfo *pack;
    Boolean changeInReceiveQueue = false;

#if PACKET_DEBUG
    DebugPacket('R', thePacket);
#endif

    //	thePacket->packet.sender = myId;

    haveToSendAck = true;

    if (thePacket->serialNumber < receiveSerial) { //	We already got this one, so just release it.
        // if the sender re-sent a packet we already have, that indicates they didn't get the ACK before
        // they sent the message
        
        itsOwner->ReleasePacket((PacketInfo *)thePacket);
    } else {
        if (thePacket->serialNumber ==
            receiveSerial) { //	Packet was next in line to arrive, so it may release others from the received queue
            UDPPacketInfo *nextPack;

#if PACKET_DEBUG > 1
            DebugPacket('%', thePacket);
#endif
            receiveSerial = thePacket->serialNumber + kSerialNumberStepSize;
            itsOwner->ReceivedGoodPacket(&thePacket->packet);

            for (pack = (UDPPacketInfo *)queues[kReceiveQ].qHead; pack; pack = nextPack) {
                nextPack = (UDPPacketInfo *)pack->packet.qLink;

                if (pack->serialNumber <= receiveSerial) {
                    OSErr theErr;

                    theErr = Dequeue((QElemPtr)pack, &queues[kReceiveQ]);
                    if (theErr == noErr) {
                        if (pack->serialNumber == receiveSerial) {
#if PACKET_DEBUG
                            DebugPacket('+', pack);
#endif
                            receiveSerial = pack->serialNumber + kSerialNumberStepSize;
                            itsOwner->ReceivedGoodPacket(&pack->packet);
                        } else {
#if PACKET_DEBUG
                            DebugPacket('-', pack);
#endif
                            itsOwner->ReleasePacket(&pack->packet);
                        }

                        changeInReceiveQueue = true;
                        nextPack = (UDPPacketInfo *)queues[kReceiveQ].qHead;
                    }
#if PACKET_DEBUG
                    else {
                        SDL_Log("Dequeue failed for received packet.\n");
                    }
#endif
                }
            }
        } else { //	We're obviously missing a packet somewhere...queue this one for later.
            short receiveQueueLength = 0;

            for (pack = (UDPPacketInfo *)queues[kReceiveQ].qHead;
                 pack != NULL && pack->serialNumber != thePacket->serialNumber;
                 pack = (UDPPacketInfo *)pack->packet.qLink) {
                receiveQueueLength++;
            }

            if (pack) { //	The queue already contains this packet.
#if PACKET_DEBUG
                DebugPacket('D', pack);
#endif
                itsOwner->ReleasePacket((PacketInfo *)thePacket);
            } else {
#if PACKET_DEBUG
                DebugPacket('Q', thePacket);
#endif
                changeInReceiveQueue = true;
                Enqueue((QElemPtr)thePacket, &queues[kReceiveQ]);
            }
        }
    }

    if (changeInReceiveQueue && offsetBufferBusy == NULL) {
        short dummyShort;

        offsetBufferBusy = &dummyShort;
        if (offsetBufferBusy == &dummyShort) {
            ackBase = receiveSerial - kSerialNumberStepSize;

            if (queues[kReceiveQ].qHead) {
                ackBitmap = 0;
                for (pack = (UDPPacketInfo *)queues[kReceiveQ].qHead; pack != NULL;
                     pack = (UDPPacketInfo *)pack->packet.qLink) {
                    ackBitmap |= 1 << (((pack->serialNumber - ackBase) >> 1) - 1);
                }
                ackBase++;
            }

            offsetBufferBusy = NULL;
        }
    }

    {
        short *receiveSerials;
        receiveSerials = receiveSerialStorage;
        for (pack = (UDPPacketInfo *)queues[kReceiveQ].qHead; pack != NULL;
             pack = (UDPPacketInfo *)pack->packet.qLink) {
            *receiveSerials++ = pack->serialNumber;
        }

        *receiveSerials++ = -12345;
    }
}

char *CUDPConnection::WriteAcks(char *dest) {
    short *mainAck;

    mainAck = (short *)dest;
    dest += sizeof(short);
    // (receiveSerial - kSerialNumberStepSize) is the last "valid" serial number received
    // this lets recipient know that they don't need to re-send anything with this serial number or less
    *mainAck = receiveSerial - kSerialNumberStepSize;

    if (offsetBufferBusy == NULL && ackBase & 1) {
        short dummyShort;
        char offset;

        offsetBufferBusy = &dummyShort;
        if (offsetBufferBusy == &dummyShort) {
            char *deltas;

            *mainAck = ackBase;
            *(int32_t *)dest = ackBitmap;
            dest += sizeof(ackBitmap);
            offsetBufferBusy = NULL;
        }
    }

    return dest;
}

void CUDPConnection::MarkOpenConnections(CompleteAddress *table) {
    short i;

    if (next)  // recurse down the chain of connections
        next->MarkOpenConnections(table);

    if (port && myId != 0) {
        for (i = 0; i < itsOwner->maxClients; i++) {
            if (table->host == ipAddr && table->port == port) {
                table->host = 0; // this connection is already open
                table->port = 0;
                return;
            }

            table++;
        }

        port = 0;
        ipAddr = 0;
        myId = -1;
        FlushQueues();
    }
}

void CUDPConnection::OpenNewConnections(CompleteAddress *table) {
    short i;
    CompleteAddress *origTable = table;

    if (port == 0) {
        for (i = 1; i <= itsOwner->maxClients; i++) {
            if (table->host && table->port) {
                myId = i;
                FreshClient(table->host, table->port, 0);
                table->host = 0;
                table->port = 0;
                break;
            }

            table++;
        }
    }

    if (next)  // recurse down the chain of connections
        next->OpenNewConnections(origTable);
}

void CUDPConnection::FreshClient(ip_addr remoteHost, port_num remotePort, long firstReceiveSerial) {
    SDL_Log("CUDPConnection::FreshClient(%u, %hu)\n", remoteHost, remotePort);
    FlushQueues();
    serialNumber = 0;
    receiveSerial = firstReceiveSerial;

    maxValid = -kSerialNumberStepSize;

    retransmitTime = kInitialRetransmitTime;
    urgentRetransmitTime = itsOwner->urgentResendTime;
    meanRoundTripTime = kInitialRoundTripTime;
    varRoundTripTime = meanRoundTripTime*meanRoundTripTime;
    // pessimistTime = roundTripTime;
    // optimistTime = roundTripTime;
    // realRoundTrip = roundTripTime;
    // deviation = roundTripTime;

    cramData = 0;

    killed = false;

    quota = 0;
    validTime = itsOwner->GetClock();
    nextWriteTime = validTime + kAckRetransmitBase;
    nextAckTime = validTime + kAckRetransmitBase;
    haveToSendAck = true;

    ipAddr = remoteHost;
    port = remotePort;
}

Boolean CUDPConnection::AreYouDone() {
    if (queues[kReceiveQ].qHead) {
        return false;
    } else {
        if (next)
            return next->AreYouDone();
        else
            return true;
    }
}

void CUDPConnection::CloseSlot(short theId) {
    if (port && myId == theId) {
        routingMask = 0;
        myId = -1;
        ipAddr = 0;
        port = 0;
        FlushQueues();
    } else if (next)
        next->CloseSlot(theId);
}

void CUDPConnection::ErrorKill() {
    if (!killed) {
        killed = true;
        routingMask = 0;

        if (myId == 0) //	Connection to server fails
            itsOwner->SendPacket(1 << itsOwner->myId, kpPacketProtocolLogout, itsOwner->myId, 0, 0, 0, NULL);
        else
            itsOwner->SendPacket(kdServerOnly, kpPacketProtocolLogout, myId, 0, 0, 0, NULL);
    }
}

void CUDPConnection::ReceiveControlPacket(PacketInfo *thePacket) {
    switch (thePacket->p1) {
        case udpCramInfo:
            if (thePacket->sender == myId) {
                cramData = thePacket->p2;
            } else if (next) {
                next->ReceiveControlPacket(thePacket);
            }
            break;
    }
}

void CUDPConnection::GetConnectionStatus(short slot, UDPConnectionStatus *parms) {
    if (slot == myId) {
        parms->hostIP = ipAddr;
        // parms->estimatedRoundTrip = ((realRoundTrip << 9) + 256) / 125;
        parms->averageRoundTrip = ((((long)meanRoundTripTime) << 9) + 256) / 125;
        // parms->pessimistRoundTrip = ((pessimistTime << 9) + 256) / 125;
        // parms->optimistRoundTrip = ((optimistTime << 9) + 256) / 125;
        parms->connectionType = cramData;
    } else {
        if (next)
            next->GetConnectionStatus(slot, parms);
    }
}
