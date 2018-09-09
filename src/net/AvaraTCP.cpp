/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: AvaraTCP.c
    Created: Sunday, January 28, 1996, 17:24
    Modified: Monday, September 2, 1996, 13:42
*/

#define MAIN_AVARA_TCP

#include "AvaraTCP.h"

#include "Memory.h"

#include <map>

typedef struct {
    ReadCompleteProc *callback;
    void *userData;
} UDPReadData;

SDLNet_SocketSet sockSet = NULL;
// std::map<UDPsocket, UDPReadData*> pendingReads;
Boolean gAvaraTCPOpen = false;

UDPsocket gReadSocket;
UDPReadData gReadCallback;

OSErr PascalStringToAddress(StringPtr name, ip_addr *addr) {
    return noErr;
}

OSErr AddressToPascalString(ip_addr addr, StringPtr name) {
    return noErr;
}

OSErr OpenAvaraTCP() {
    if (gAvaraTCPOpen) {
        return noErr;
    }
    SDL_Log("OpenAvaraTCP\n");
    sockSet = SDLNet_AllocSocketSet(16); // kMaxAvaraPlayers? Or maybe just 1-2?
    gAvaraTCPOpen = true;
    return noErr;
}

UDPsocket CreateSocket(uint16_t port) {
    UDPsocket sock = SDLNet_UDP_Open(port);
    if (!sock) {
        SDL_Log("Failed to create socket on port %d: %s\n", port, SDLNet_GetError());
    }
    return sock;
}

void CheckSockets() {
    if (sockSet == NULL) {
        SDL_Log("CheckSockets called before OpenAvaraTCP\n");
        return;
    }
    if (int ready = SDLNet_CheckSockets(sockSet, 0) > 0) {
        if (SDLNet_SocketReady(gReadSocket)) {
            UDPpacket *packet = SDLNet_AllocPacket(UDPSTREAMBUFFERSIZE);
            while (SDLNet_UDP_Recv(gReadSocket, packet) > 0) {
                gReadCallback.callback(packet, gReadCallback.userData);
            }
            SDLNet_FreePacket(packet);
            SDLNet_UDP_DelSocket(sockSet, gReadSocket);
        }
        /*
        for(auto it = pendingReads.cbegin(); it != pendingReads.cend(); ) {
            UDPsocket sock = it->first;
            UDPReadData *completion = it->second;
            if(SDLNet_SocketReady(sock)) {
                // Remove this BEFORE the callback, since it may ask for another read...
                it = pendingReads.erase(it);
                //SDL_Log("Socket %x ready for action!\n", sock);
                UDPpacket *packet = SDLNet_AllocPacket(UDPSTREAMBUFFERSIZE);
                if(SDLNet_UDP_Recv(sock, packet)) {
                    //SDL_Log("Socket %x received packet (len=%d)\n", sock, packet->len);
                    completion->callback(packet, completion->userData);
                    DisposePtr((Ptr)completion);
                }
                SDLNet_FreePacket(packet);
                SDLNet_UDP_DelSocket(sockSet, sock);
            }
            else {
                it = std::next(it);
            }
        }
        */
    }
}

void UDPRead(UDPsocket sock, ReadCompleteProc callback, void *userData) {
    gReadSocket = sock;
    gReadCallback.callback = callback;
    gReadCallback.userData = userData;
    // SDL_Log("UDPRead socket=%x\n", sock);
    // TODO: make sure MacTCP didn't allow queueing multiple UDPRead calls
    /*
    UDPReadData *completion = (UDPReadData *)NewPtr(sizeof(UDPReadData));
    completion->callback = callback;
    completion->userData = userData;
    pendingReads.insert(std::pair<UDPsocket,UDPReadData*>(sock, completion));
    */
    SDLNet_UDP_AddSocket(sockSet, gReadSocket);
}

void UDPWrite(UDPsocket sock, UDPpacket *packet, WriteCompleteProc callback, void *userData) {
    // SDL_Log("UDPWrite socket=%x dest=%lu:%d\n", sock, packet->address.host, packet->address.port);
    // TODO: I think UDP writes are non-blocking, but check
    int sent = SDLNet_UDP_Send(sock, -1, packet);
    if (sent <= 0) {
        SDL_Log("Send failed: %s\n", SDLNet_GetError());
    }
    callback(sent > 0 ? noErr : -1, userData);
}
