/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: AvaraTCP.c
    Created: Sunday, January 28, 1996, 17:24
    Modified: Monday, September 2, 1996, 13:42
*/

#define MAIN_AVARA_TCP

#ifdef _WIN32 
typedef int socklen_t;
/* POSIX ssize_t is not a thing on Windows */
typedef signed long long int ssize_t;

#include <winsock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
#define close closesocket
#else
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/time.h>
#endif

#include "AvaraTCP.h"

#include "Memory.h"

#include <SDL.h>

typedef struct {
    ReadCompleteProc *callback;
    void *userData;
} UDPReadData;

static Boolean gAvaraTCPOpen = false;
static int gReadSocket = -1;
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
    gAvaraTCPOpen = true;
    return noErr;
}

int CreateSocket(uint16_t port) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock != -1) {
        //int reuseAddrValue = 1;
        //setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseAddrValue, sizeof(int));

        struct sockaddr_in sock_addr;
        memset(&sock_addr, 0, sizeof(sock_addr));
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_addr.s_addr = INADDR_ANY;
        sock_addr.sin_port = htons(port);

        if (bind(sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1) {
            SDL_Log("Failed to bind socket to port %d", port);
            DestroySocket(sock);
        }
    } else {
        SDL_Log("Failed to create socket on port %d", port);
    }
    return sock;
}

void DestroySocket(int sock) {
    if(sock != -1) {
        gReadSocket = -1;
        close(sock);
    }
}

UDPpacket * CreatePacket(int bufferSize) {
    UDPpacket *packet = (UDPpacket *)malloc(sizeof(UDPpacket));
    packet->data = (uint8_t *)malloc(bufferSize);
    packet->len = 0;
    packet->data[0] = 0;
    return packet;
}

void FreePacket(UDPpacket *packet) {
    if (packet) {
        free(packet->data);
        free(packet);
    }
}

int ResolveHost(IPaddress *address, const char *host, uint16_t port) {
    address->port = htons(port);
    if (host == NULL) {
        address->host = INADDR_ANY;
    }
    else {
        address->host = inet_addr(host);
        if (address->host == INADDR_NONE) {
            struct hostent *hp = gethostbyname(host);
            if (hp) {
                memcpy(&address->host, hp->h_addr, hp->h_length);
            } else {
                return -1;
            }
        }
    }
    return noErr;
}

void CheckSockets() {
    if (!gAvaraTCPOpen) {
        SDL_Log("CheckSockets called before OpenAvaraTCP\n");
        return;
    }

    if (gReadSocket == -1) return;

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(gReadSocket, &readSet);

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    if (select(gReadSocket + 1, &readSet, NULL, NULL, &timeout) > 0) {
        if (FD_ISSET(gReadSocket, &readSet)) {
            struct sockaddr_in addr;
            socklen_t len = sizeof(addr);
            UDPpacket *packet = CreatePacket(UDPSTREAMBUFFERSIZE);
            int status = recvfrom(
                gReadSocket,
                (char *)packet->data,
                UDPSTREAMBUFFERSIZE,
                0,
                (struct sockaddr *)&addr,
                &len
            );
            if (status > 0) {
                packet->len = status;
                packet->address.host = addr.sin_addr.s_addr;
                packet->address.port = addr.sin_port;
                gReadCallback.callback(packet, gReadCallback.userData);
            }
            FreePacket(packet);
        }
    }
}

void UDPRead(int sock, ReadCompleteProc callback, void *userData) {
    if (sock != -1) {
        gReadCallback.callback = callback;
        gReadCallback.userData = userData;
        // setting gReadSocket signals to CheckSockets that we're ready to start accepting data
        gReadSocket = sock;
    }
}

void UDPWrite(int sock, UDPpacket *packet, WriteCompleteProc callback, void *userData) {
    if (sock != -1) {
        struct sockaddr_in sock_addr;
        memset(&sock_addr, 0, sizeof(sock_addr));
        sock_addr.sin_addr.s_addr = packet->address.host;
        sock_addr.sin_port = packet->address.port;
        sock_addr.sin_family = AF_INET;
        ssize_t sent = sendto(
            sock,
            (char*)packet->data,
            packet->len,
            0,
            (struct sockaddr *)&sock_addr,
            sizeof(sock_addr)
        );
        callback(sent > 0 ? noErr : -1, userData);
    }
}

// A utility function to byte-swap the host & port and create a human-readable format
#include <sstream>
std::string FormatHostPort(uint32_t host, uint16_t port) {
    std::ostringstream os;
    ip_addr ipaddr = SDL_SwapBE32(host);
    os << (ipaddr >> 24) << "."
       << ((ipaddr >> 16) & 0xff) << "."
       << ((ipaddr >> 8) & 0xff) << "."
       << (ipaddr & 0xff) <<  ":"
       << SDL_SwapBE16(port);
    return os.str();
}
