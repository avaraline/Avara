/*
    Copyright ©1996-1998, Juri Munkki
    All rights reserved.

    File: CUDPConnection.c
    Created: Monday, January 29, 1996, 13:36
    Modified: Saturday, January 3, 1998, 02:14
*/

#include "CUDPConnection.h"

#include "CUDPComm.h"
#include "System.h"
#include "CAvaraGame.h"  // gCurrentGame->fpsScale
#include "CApplication.h"  // gApplication
#include "Debug.h"
#include "System.h"

#include <SDL2/SDL.h>

#define kInitialRetransmitTime    (2000 / MSEC_PER_GET_CLOCK)  // 2 seconds
#define kInitialRoundTripTime     (500 / MSEC_PER_GET_CLOCK)   // 0.5 second
#define kInitialRoundTripVar      long(CLASSICFRAMECLOCK*CLASSICFRAMECLOCK)
#define kMaxAllowedRetransmitTime (4000 / MSEC_PER_GET_CLOCK)  // 4 seconds
#define kAckRetransmitBase (41 / MSEC_PER_GET_CLOCK)
#define kULPTimeOut (120000 / MSEC_PER_GET_CLOCK) //	120 seconds
#define kMaxTransmitQueueLength 128*8 //	128 packets going out... (times 8 for high FPS)
#define kMaxReceiveQueueLength kMaxTransmitQueueLength   // receive as much as is sent

#define RTTSMOOTHFACTOR_UP 100
#define RTTSMOOTHFACTOR_DOWN 100
#define COUNTSMOOTHFACTOR 1000

#define MAX_RESENDS_WITHOUT_RECEIVE 3

#if PACKET_DEBUG || LATENCY_DEBUG
void CUDPConnection::DebugPacket(char eType, UDPPacketInfo *p) {
    if (p) {
        SDL_Log("CUDPConnection::DebugPacket(%c) cn=%d rsn=%d sn=%d-%d cmd=%d p1=%d p2=%d p3=%d flags=0x%02x sndr=%d dist=0x%02x\n",
                eType,
                myId,
                (uint16_t)receiveSerial,
                (uint16_t)p->serialNumber,
                p->sendCount,
                p->packet.command,
                p->packet.p1,
                p->packet.p2,
                p->packet.p3,
                p->packet.flags,
                p->packet.sender,
                p->packet.distribution);
    } else {
        SDL_Log("CUDPConnection::DebugPacket(%c) ----------NULL PACKET----------\n", eType);
    }
}
#endif

void CUDPConnection::IUDPConnection(CUDPComm *theOwner) {
    short i;

    killed = false;
    itsOwner = theOwner;
    myId = -1; //	Unknown

    ipAddr = 0;
    port = 0;

    for (i = 0; i < kQueueCount; i++) {
        InitQueue(&queues[i]);
    }

    serialNumber = INITIAL_SERIAL_NUMBER;
    receiveSerial = INITIAL_SERIAL_NUMBER;
    maxValid = INITIAL_SERIAL_NUMBER-kSerialNumberStepSize;

    retransmitTime = kInitialRetransmitTime;
    urgentRetransmitTime = kInitialRoundTripTime;
    meanRoundTripTime = kInitialRoundTripTime;
    varRoundTripTime = kInitialRoundTripVar;

    // sliding histogram of 16 seconds (1000 samples) to track latency stats
    latencyHistogram = new SlidingHistogram<float>(16*1000/(gCurrentGame->fpsScale*CLASSICFRAMETIME),
                                                   0.0, 4.0, gCurrentGame->fpsScale);

    meanSendCount = 1.0;     // average of # packets sent out including re-sends for each packet
    meanReceiveCount = 0.0;  // average # of times the first sent packet sent not received (reflects packet loss)
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
    recentResendRate = 0;
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
            thePacket->serialNumber = INITIAL_SERIAL_NUMBER-1;  // assigned later
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

void CUDPConnection::ProcessBusyQueue(int32_t curTime) {
    UDPPacketInfo *thePacket = NULL;

    while ((thePacket = (UDPPacketInfo *)queues[kBusyQ].qHead)) {
        if (noErr == Dequeue((QElemPtr)thePacket, &queues[kBusyQ])) {
            thePacket->sendCount = 0;
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

UDPPacketInfo *CUDPConnection::FindBestPacket(int32_t curTime, int32_t cramTime, int32_t urgencyAdjust) {
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
            (bestPacket->sendCount == 0 || numResendsWithoutReceive < MAX_RESENDS_WITHOUT_RECEIVE)) {
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
                    (thePacket->sendCount == 0 || numResendsWithoutReceive < MAX_RESENDS_WITHOUT_RECEIVE)) {
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
        DBG_Log("q", "Transmit Queue Overflow - myId = %d\n", myId);
        for (int i = 0; i < kQueueCount; i++) {
            DBG_Log("q", "   Queue[%d] size = %zu\n", i, queues[i].qSize);
        }
        ErrorKill();
    }

    return thePacket;
}

UDPPacketInfo *CUDPConnection::GetOutPacket(int32_t curTime, int32_t cramTime, int32_t urgencyAdjust) {
    UDPPacketInfo *thePacket = NULL;

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
                SDL_Log("CUDPConnection::DebugPacket(S) <ACK> cn=%d rsn=%hu\n", myId, uint16_t(receiveSerial - kSerialNumberStepSize));
            #endif
        } else {
            totalSent++;
            static double RECENT_RESEND_SMOOTH = (gCurrentGame->fpsScale/80.0);
            recentResendRate *= (1.0-RECENT_RESEND_SMOOTH);
            if (thePacket->birthDate != thePacket->nextSendTime) {
                totalResent++;
                numResendsWithoutReceive++;
                recentResendRate += RECENT_RESEND_SMOOTH;
                #if PACKET_DEBUG | LATENCY_DEBUG
                    SDL_Log("CUDPConnection::GetOutPacket   RESENDING cn=%d sn=%d age=%d resend:count=%ld total=%.1f%% recent=%.1f%%\n",
                            myId, (uint16_t)thePacket->serialNumber, curTime - thePacket->birthDate,
                            numResendsWithoutReceive, 100.0*totalResent/totalSent, 100.0*recentResendRate);
                #endif
            }
            #if PACKET_DEBUG
                DebugPacket('S', thePacket);
            #endif
        }
    }

    return thePacket;
}

float CommandMultiplierForStats(const PacketInfo& thePacketInfo) {
    float multiplier = 0;
    // only use faster commands for stats
    // keep multiplier value below RTTSMOOTHFACTOR_UP.
    switch(thePacketInfo.command) {
        case kpKeyAndMouse:
            // normal game play commands (multiply by fpsScale to make influence consistent across frame rates)
            multiplier = gCurrentGame->fpsScale;
            break;
        case kpPing:
            // ping times are an important influence on RTT (not affected by frame rate)
            if (thePacketInfo.p3 > 0) {
                // the last ping time can be too big if there aren't other messages coming after it
                // because the receiver may have no reason to respond in a timely manner so ignore p3==0
                multiplier = CLASSICFRAMETIME * 0.4;  // smooth similarly to in-game rate
            }
            break;
    }
    return multiplier;
}

void CUDPConnection::ValidatePacket(UDPPacketInfo *thePacket, int32_t when) {
    #if PACKET_DEBUG || LATENCY_DEBUG
        DebugPacket('V', thePacket);
    #endif
    if (noErr == Dequeue((QElemPtr)thePacket, &queues[kTransmitQ])) {
        long roundTrip;

        roundTrip = when - thePacket->birthDate;
        float commandMultiplier = CommandMultiplierForStats(thePacket->packet);

        if (uint16_t(maxValid) <= 10) {
            // use best RTT of early message(s) to prime initial values of RTT mean/var
            if (roundTrip < meanRoundTripTime) {
                meanRoundTripTime = roundTrip;
                varRoundTripTime = std::min(roundTrip*roundTrip, kInitialRoundTripVar);
            }
            #if PACKET_DEBUG || LATENCY_DEBUG
                SDL_Log("INITIALIZING RTT...            cn=%d cmd=%d roundTrip=%ld mean=%.1f std = %.1f, maxValid=%hd\n",
                        myId, thePacket->packet.command, roundTrip, meanRoundTripTime, sqrt(varRoundTripTime), uint16_t(maxValid));
            #endif
        } else if (commandMultiplier > 0) {
            if (thePacket->packet.command == kpPing) {
                // decrease ping roundTrip because pings aren't as fast as urgent game packets and we
                // want to start the game at about the right LT (which is based the average ping times)
                roundTrip *= 0.82;
            }

            // compute an exponential moving average & variance of the roundTrip time
            // see: https://fanf2.user.srcf.net/hermes/doc/antiforgery/stats.pdf
            float difference = roundTrip - meanRoundTripTime;

            // if kpPing packet RTT is above the mean by more than a full classic frame,
            // then it's an outlier...de-weight it heavily, but don't remove it
            // completely in case ALL packets are suddenly worse
            if (thePacket->packet.command == kpPing && difference*MSEC_PER_GET_CLOCK > CLASSICFRAMETIME) {
                double reduction = difference*difference / varRoundTripTime;
                #if PACKET_DEBUG || LATENCY_DEBUG
                    SDL_Log("                               cn=%d cmd=%d roundTrip=%ld mean=%.1f OUTLIER!! (rtt-mean) = %.1lf * stddev\n",
                            myId, thePacket->packet.command, roundTrip, meanRoundTripTime, sqrt(reduction));
                #endif

                // ratio can be less than 1 if variance is high, guard to ensure alpha < 1.0
                reduction = std::max(4.0, reduction);
                // de-weight this packet
                commandMultiplier /= reduction;
            }

            // quicker to move up on latency spikes, slower to move down
            float alpha = commandMultiplier / ((difference > 0) ? RTTSMOOTHFACTOR_UP : RTTSMOOTHFACTOR_DOWN);

            float increment = alpha * difference;
            meanRoundTripTime = meanRoundTripTime + increment;
            varRoundTripTime = (1 - alpha) * (varRoundTripTime + difference * increment);
            float stdevRoundTripTime = sqrt(varRoundTripTime);

            // meanSendCount is an approximate average of how many times a command is sent, including re-sends
            // ideally this would be close to 1.0
            commandMultiplier = gCurrentGame->fpsScale;  // all commands treated same for meanSendCount
            alpha = commandMultiplier / COUNTSMOOTHFACTOR;
            meanSendCount = meanSendCount + alpha * (thePacket->sendCount - meanSendCount);
            #if LATENCY_DEBUG
                SDL_Log("                              sn=%hd count=%hd meanSendCount=%.4f\n", thePacket->serialNumber, thePacket->sendCount, meanSendCount);
            #endif


            // Use +2.5 sigma(probability 98.7%) for non-urgent retransmitTime
            retransmitTime = meanRoundTripTime + (long)(2.5*stdevRoundTripTime);

            // don't let the retransmit times fall below urgentResendTime (LT =~ 2) or go above kMaxAllowedRetransmitTime
            retransmitTime = std::max(retransmitTime, itsOwner->urgentResendTime);
            retransmitTime = std::min(retransmitTime, kMaxAllowedRetransmitTime);

            // If we want the game to stay smooth, resend urgent/game packets with the goal that both the orignal packet and
            // the 1st resend packet could arrive before they need to be acted on.
            // ...at difference between actual LT and mean roundTripTime
            // for this connection (minus a small buffer) to ensure that the 1st re-sent packet has better than a 50% chance
            // of being received on time (within LT*2) on the SLOWEST connection (higher probability for all other connections).
            // This may result in extra re-sends but should help the game flow, especially if a connection is dropping packets.
            // This latency estimate goes across all active connections so that faster connections won't be penalized and
            // have to re-send to each other as often.
            static float RESEND_PACKET_BUFFER = 16.0 / MSEC_PER_GET_CLOCK;  // want resent packet to arrive roughly this many msec before 2*LT
            urgentRetransmitTime =
                2*gCurrentGame->latencyTolerance * CLASSICFRAMECLOCK - meanRoundTripTime - RESEND_PACKET_BUFFER;
            // make sure it doesn't go below urgentResendTime or above retransmitTime
            urgentRetransmitTime = std::max(urgentRetransmitTime, itsOwner->urgentResendTime);
            urgentRetransmitTime = std::min(urgentRetransmitTime, retransmitTime);

            #if PACKET_DEBUG || LATENCY_DEBUG
            SDL_Log("                               cn=%d cmd=%d roundTrip=%ld mean=%.1f std = %.1f retransmitTime=%d urgentRetransmit=%d\n",
                        myId, thePacket->packet.command, roundTrip, meanRoundTripTime, stdevRoundTripTime, retransmitTime, urgentRetransmitTime);
            #endif

            if (Debug::IsEnabled("hist")) {
                latencyHistogram->push(roundTrip / (2.0*CLASSICFRAMETIME));
                if (thePacket->serialNumber % (latencyHistogram->size()*2/3) == 0) {  // 1/3 overlap each output
                    SDL_Log("\n       LT histogram for connection #%d", myId);
                    std::cerr << *latencyHistogram;
                }
            }
        }

#if PACKET_DEBUG > 1
        if (thePacket->packet.command == kpKeyAndMouse) {
            DebugPacket('/', thePacket);
        }
        DebugPacket('X', thePacket);
#endif
        itsOwner->ReleasePacket((PacketInfo *)thePacket);
    } else {
        SDL_Log("ERROR dequeueing packet (sn=%d) from kTransmitQ\n", (uint16_t)thePacket->serialNumber);
    }
}

void CUDPConnection::ValidateReceivedPacket(UDPPacketInfo *thePacket) {
    float commandMultiplier = CommandMultiplierForStats(thePacket->packet);
    if (commandMultiplier > 0) {
        float alpha = commandMultiplier / COUNTSMOOTHFACTOR;

        // meanReceiveCount is a measure of how often we don't receive/process the first sent packet from a client,
        // when 1% of packets are being lost then this should trend towards ~0.01 (1%)
        meanReceiveCount = meanReceiveCount + alpha * (thePacket->sendCount - meanReceiveCount);
    }

    // dispacth & release the packet
    itsOwner->ReceivedGoodPacket(&thePacket->packet);
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

char *CUDPConnection::ValidatePackets(char *validateInfo, int32_t curTime) {
    SerialNumber transmittedSerial;
    UDPPacketInfo *thePacket, *nextPacket;

    // received something, reset the counter
    numResendsWithoutReceive = 0;

    transmittedSerial = *(short *)validateInfo;  // Last received "valid" serial number by this connection
    validateInfo += sizeof(short);               // point to the AckMap field (if there is one)

    #if PACKET_DEBUG
        SDL_Log("ValidatePackets transmittedSerial=%hu, maxValid = %hu\n", uint16_t(transmittedSerial), uint16_t(maxValid));
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

            // ackMap indicates, via bitmask, which other packets after transmittedSerial have been received by the client
            if (ackMap & (1 << (((thePacket->serialNumber - transmittedSerial) >> 1) - 1))) {
                // received by the client so it can be marked as done, measured for RTT stats then released
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
    delete latencyHistogram;
    CDirectObject::Dispose();
}

static short receiveSerialStorage[512];

size_t CUDPConnection::ReceivedPacket(UDPPacketInfo *thePacket) {
    UDPPacketInfo *pack;
    Boolean changeInReceiveQueue = false;

#if PACKET_DEBUG
    DebugPacket('R', thePacket);
#endif

    //	thePacket->packet.sender = myId;

    haveToSendAck = true;

    if (thePacket == nullptr) {
        // nullptr indicates we need to skip forward and process packets in the queue
        DBG_Log("q", "CUDPConnection::ReceivedPacket: SKIPPING sn=%d", (int)receiveSerial);
        receiveSerial += kSerialNumberStepSize;
        // now go look for all the queued-up packets starting from the new receiveSerial
        changeInReceiveQueue = ReceiveQueuedPackets();

    } else if (thePacket->serialNumber < receiveSerial) { //	We already got this one, so just release it.
        // if the sender re-sent a packet we already have, that indicates they didn't get the ACK before
        // they sent the message

        itsOwner->ReleasePacket((PacketInfo *)thePacket);
    } else {
        if (thePacket->serialNumber ==
            receiveSerial) { //	Packet was next in line to arrive, so it may release others from the received queue

#if PACKET_DEBUG > 1
            DebugPacket('%', thePacket);
#endif
            // receiveSerial = serial number of next serial number we're waiting for
            receiveSerial = thePacket->serialNumber + kSerialNumberStepSize;

            // this will either handle the packet OR put it on a queue (inQ) for later handling
            ValidateReceivedPacket(thePacket);

            // see if any subsequent packets have already been queued up
            changeInReceiveQueue = ReceiveQueuedPackets();

        } else { //	We're obviously missing a packet somewhere...queue this one for later.
            // short receiveQueueLength = 0;

            for (pack = (UDPPacketInfo *)queues[kReceiveQ].qHead;
                 pack != NULL && pack->serialNumber != thePacket->serialNumber;
                 pack = (UDPPacketInfo *)pack->packet.qLink) {
                // receiveQueueLength++;
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
                // we have received a serial number greater than the one we're waiting for so queue this one
                changeInReceiveQueue = true;
                Enqueue((QElemPtr)thePacket, &queues[kReceiveQ]);

                size_t qsize = queues[kReceiveQ].qSize;
                if (Debug::IsEnabled("q")) {
                    if (qsize % 5 == 0) {
                        DBG_Log("q", "%d: rsn=%d, queued sn=%d to kReceivedQ.size = %zu\n", myId, (int)receiveSerial, (int)thePacket->serialNumber, qsize);
                    }
                }
                // give up and forget about receiving sn=receiveSerial after awhile,
                // players will request a frame resend and it could be coming in a later packet so skip this one
                int PACKET_CLEANUP = 4.0/gCurrentGame->fpsScale;   // bigger is NOT better, don't go above 5-ish
                if (qsize >= PACKET_CLEANUP) {  // LT of 8 would be 32 frame packets, plus a little for other packets
                    // call self recursively to skip receiveSerial
                    return ReceivedPacket(nullptr);
                }
            }
        }
    }

    if (changeInReceiveQueue && offsetBufferBusy == NULL) {
        short dummyShort;

        offsetBufferBusy = &dummyShort;
        if (offsetBufferBusy == &dummyShort) {
            // ackBase is last sequential serial number received (i.e. haven't received sn=receiveSerial yet)
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

    return queues[kReceiveQ].qSize;
}

bool CUDPConnection::ReceiveQueuedPackets() {
    bool changeInReceiveQueue = false;
    UDPPacketInfo *nextPack;

    for (UDPPacketInfo* pack = (UDPPacketInfo *)queues[kReceiveQ].qHead; pack; pack = nextPack) {
        nextPack = (UDPPacketInfo *)pack->packet.qLink;

        if (pack->serialNumber <= receiveSerial) {
            OSErr theErr;

            theErr = Dequeue((QElemPtr)pack, &queues[kReceiveQ]);
            if (theErr == noErr) {
                if (pack->serialNumber == receiveSerial) {
#if PACKET_DEBUG
                    DebugPacket('+', pack);
#endif
                    if (Debug::IsEnabled("q")) {
                        size_t qsize = queues[kReceiveQ].qSize;
                        if (qsize > 0 && qsize % 5 == 0) {
                            DBG_Log("q", "%d: rsn=%d, dequeued sn=%d to kReceivedQ.size = %zu\n", myId, (int)receiveSerial, (int)pack->serialNumber, qsize);
                        }
                    }

                    receiveSerial = pack->serialNumber + kSerialNumberStepSize;
                    ValidateReceivedPacket(pack);
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

    return changeInReceiveQueue;
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

        offsetBufferBusy = &dummyShort;
        if (offsetBufferBusy == &dummyShort) {
            *mainAck = ackBase;
            *(int32_t *)dest = ackBitmap;
            dest += sizeof(ackBitmap);
            offsetBufferBusy = NULL;
        }
    }

    return dest;
}

void CUDPConnection::MarkOpenConnections(CompleteAddress *table) {
    if (next)  // recurse down the chain of connections
        next->MarkOpenConnections(table);

    if (port && myId != 0) {
        for (int i = 0; i < itsOwner->maxClients; i++) {
            if (table->host == ipAddr && table->port == port) {
                table->host = 0; // this connection is being used
                table->port = 0;
                return;
            }

            table++;
        }

        DBG_Log("login", "%s no longer in connection table, marking as GONE", FormatHostPort(ipAddr, port).c_str());
        port = 0;
        ipAddr = 0;
        myId = -1;
        FlushQueues();
    }
}

void CUDPConnection::RewriteConnections(CompleteAddress *table, const CompleteAddress &myAddressInTOC) {
    static uint32_t LOCALHOST = inet_addr("127.0.0.1");
    // this is called from CUDPComm::connections[0] which should point to the server's connection
    ip_addr serverHost = ipAddr;

    // the connection table represents what the server sees as connections coming in, so
    // we may need to rewrite some of those hosts/ports to allow this client to connect to another client
    // depending on the LAN/WAN situation
    for (int i = 0; i < itsOwner->maxClients; i++) {
        if (table->host == LOCALHOST) {
            // if the server sees the connection coming from localhost, change the host to whatever host we connected to server with (which could ALSO be localhost)
            table->host = serverHost;
        }
        else if (table->host == myAddressInTOC.host && table->port != myAddressInTOC.port) {
            // if I have the same host IP as the client in the connection table, and a diff port, assume we're on the same machine
            // (this helps with the case of connecting a couple of clients/bots out to an external server)
            table->host = LOCALHOST;
        }
        // TODO: what if host is the IP of the router?  e.g. someone connects to a LAN game using the WAN address...
        // the workaround for this is to have everyone in the LAN game use the LAN address but it would be nice to make it automatic
        // else if (IsWanRouter(table->host)) { /* do something, might have to send local IP address to the server??? */ }
        table++;
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


void CUDPConnection::FreshClient(ip_addr remoteHost, port_num remotePort, uint16_t firstReceiveSerial) {
    SDL_Log("CUDPConnection::FreshClient connection = %s\n", FormatHostPort(remoteHost, remotePort).c_str());
    FlushQueues();
    serialNumber = INITIAL_SERIAL_NUMBER;
    receiveSerial = serialNumber + firstReceiveSerial;

    maxValid = INITIAL_SERIAL_NUMBER - kSerialNumberStepSize;

    retransmitTime = kInitialRetransmitTime;
    urgentRetransmitTime = itsOwner->urgentResendTime;
    meanRoundTripTime = kInitialRoundTripTime;
    varRoundTripTime = kInitialRoundTripVar;
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

    ipAddr = ipAddrExt = remoteHost;
    port = remotePort;

    // Normal client-client Avara packets will do the hole-punching for us, so this is unnecessary?
    // Sometimes the hole needs to be punched between clients such as when they are both behind double-NAT.
    IPaddress addr = { remoteHost, remotePort };
    RequestPunch(addr);
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

// latency estimate across all connection in internal time units
long CUDPConnection::LatencyEstimate() {
    float maxRTT = 0;

    for (CUDPConnection *conn = itsOwner->connections; conn; conn = conn->next) {
        // only use active connection
        if (conn->port) {
            maxRTT = std::max(maxRTT, conn->meanRoundTripTime);
        }
    }

    return maxRTT/2;
}
