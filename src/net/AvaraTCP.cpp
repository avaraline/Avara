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
#else
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#endif

#include "AvaraTCP.h"

#include "Memory.h"

#include <SDL2/SDL.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/time.h>
#endif

#define PUNCHTIME 5000

typedef struct {
    ReadCompleteProc *callback;
    void *userData;
} UDPReadData;

#pragma pack(1)
typedef struct {
    uint8_t command;
    IPaddress address;
} PunchPacket;
#pragma pack()

enum { kPunchPing = 1, kPunchRequest = 2, kPunch = 3 };

static Boolean gAvaraTCPOpen = false;
static int gAvaraSocket = -1;
UDPReadData gReadCallback;
PunchAddressHandler gPunchAddressHandler;

static IPaddress punchServer = {0, 0};
static IPaddress punchLocal = {0, 0};
static uint32_t lastPunchPing = 0;

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

int CreateSocket(uint16_t &port) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock != -1) {
        //int reuseAddrValue = 1;
        //setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseAddrValue, sizeof(int));

        struct sockaddr_in sock_addr;
        socklen_t len = sizeof(sock_addr);
        memset(&sock_addr, 0, len);
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_addr.s_addr = INADDR_ANY;
        sock_addr.sin_port = htons(port);

        if (bind(sock, (struct sockaddr *)&sock_addr, len) == -1) {
            SDL_Log("Failed to bind socket to port %d", port);
            DestroySocket(sock);
            return -1;
        }

        if (port == 0) {
            // get/return the actual port number
            getsockname(sock, (struct sockaddr *)&sock_addr, &len);
            port = ntohs(sock_addr.sin_port);
        }

        // This is taking advantage of the fact that Avara only ever creates one socket at a time.
        gAvaraSocket = sock;
    } else {
        SDL_Log("Failed to create socket on port %d", port);
    }
    return sock;
}

void DestroySocket(int sock) {
    if(sock != -1) {
        if (sock == gAvaraSocket) {
            gAvaraSocket = -1;
            punchLocal.host = 0;
            punchLocal.port = 0;
        }
#ifdef WIN32
        closesocket(sock);
#else
        close(sock);
#endif
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

void PunchSetup(const char *host, uint16_t port) {
    ResolveHost(&punchServer, host, port);
}

void PunchSend(uint8_t cmd, IPaddress &addr) {
    PunchPacket pp = {cmd, addr};
    //SDL_Log("Sending PunchPacket %d = %s", cmd, FormatAddress(pp.address).c_str());
    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_addr.s_addr = punchServer.host;
    sock_addr.sin_port = punchServer.port;
    sock_addr.sin_family = AF_INET;
    sendto(
        gAvaraSocket,
        (char *)&pp,
        sizeof(PunchPacket),
        0,
        (struct sockaddr *)&sock_addr,
        sizeof(sock_addr)
    );
}

void PingPunchServer() {
    if (gAvaraSocket == -1 || punchLocal.port == 0 || punchServer.port == 0) return;

    PunchSend(kPunchPing, punchLocal);
}

void RegisterPunchServer(IPaddress &localAddr) {
    if (gAvaraSocket == -1 || punchServer.port == 0) return;

    SDL_Log("Registering with punch server: %s", FormatAddress(localAddr).c_str());
    memcpy(&punchLocal, &localAddr, sizeof(IPaddress));
    PingPunchServer();
}

void RequestPunch(IPaddress &addr) {
    if (gAvaraSocket == -1 || punchServer.port == 0) return;

    SDL_Log("Requesting that %s punch a hole", FormatAddress(addr).c_str());
    PunchSend(kPunchRequest, addr);
}

void Punch(IPaddress &addr) {
    if (gAvaraSocket == -1) return;

    SDL_Log("Punching back to %s", FormatAddress(addr).c_str());

    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_addr.s_addr = addr.host;
    sock_addr.sin_port = addr.port;
    sock_addr.sin_family = AF_INET;
    sendto(
        gAvaraSocket,
        "",
        0,
        0,
        (struct sockaddr *)&sock_addr,
        sizeof(sock_addr)
   );
}

void HandlePunchPacket(UDPpacket *packet) {
    if (packet->len != sizeof(PunchPacket)) {
        SDL_Log("Invalid PunchPacket size (%d)", packet->len);
        return;
    }
    PunchPacket *pp = (PunchPacket *)packet->data;
    SDL_Log("Got packet from punch server - (%d) %s", pp->command, FormatAddress(pp->address).c_str());

    if (gPunchAddressHandler) {
        gPunchAddressHandler(pp->address);
    }

    if (pp->command == kPunch) {
        Punch(pp->address);
    }
}

void CheckSockets() {
    if (!gAvaraTCPOpen) {
        SDL_Log("CheckSockets called before OpenAvaraTCP\n");
        return;
    }

    if (gAvaraSocket == -1) return;

    if (punchServer.port != 0) {
        if (lastPunchPing == 0 || (SDL_GetTicks() - lastPunchPing >= PUNCHTIME)) {
            PingPunchServer();
            lastPunchPing = SDL_GetTicks();
        }
    }

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(gAvaraSocket, &readSet);

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    if (select(gAvaraSocket + 1, &readSet, NULL, NULL, &timeout) > 0) {
        if (FD_ISSET(gAvaraSocket, &readSet)) {
            struct sockaddr_in addr;
            socklen_t len = sizeof(addr);
            UDPpacket *packet = CreatePacket(UDPSTREAMBUFFERSIZE);
            auto bytesRead = recvfrom(
                gAvaraSocket,
                (char *)packet->data,
                UDPSTREAMBUFFERSIZE,
                0,
                (struct sockaddr *)&addr,
                &len
            );
            packet->len = static_cast<int>(bytesRead);
            packet->address.host = addr.sin_addr.s_addr;
            packet->address.port = addr.sin_port;
            if (bytesRead > 0) {
                if (packet->address.host == punchServer.host && packet->address.port == punchServer.port && punchServer.port != 0) {
                    HandlePunchPacket(packet);
                }
                else {
                    gReadCallback.callback(packet, gReadCallback.userData);
                }
            }
            else {
                SDL_Log("Read 0 bytes from %s - PUNCH", FormatAddress(packet->address).c_str());
            }
            FreePacket(packet);
        }
    }
}

void UDPRead(int sock, ReadCompleteProc callback, void *userData) {
    if (sock != -1) {
        gReadCallback.callback = callback;
        gReadCallback.userData = userData;
    }
}

void UDPWrite(int sock, UDPpacket *packet, WriteCompleteProc callback, void *userData) {
    if (sock != -1) {
        //SDL_Log("writing %d bytes to %s", packet->len, FormatAddress(packet->address).c_str());
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

std::string FormatAddress(IPaddress &addr) {
    return FormatHostPort(addr.host, addr.port);
}

void SetPunchAddressHandler(PunchAddressHandler handler) {
    gPunchAddressHandler = handler;
}
