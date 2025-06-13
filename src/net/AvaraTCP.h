/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: AvaraTCP.h
    Created: Sunday, January 28, 1996, 17:22
    Modified: Tuesday, September 17, 1996, 05:14
*/

#pragma once
#include <functional>   // std::function

#include "Types.h"

#define UDPSTREAMBUFFERSIZE 16384
#define UDPSENDBUFFERSIZE 1024


#pragma pack(1)
typedef struct {
    uint32_t host;
    uint16_t port;
} IPaddress;
#pragma pack()

typedef struct {
    uint8_t *data;
    uint32_t len; // could/should probably be uint16_t?
    IPaddress address;
} UDPpacket;

enum PunchType {
    kPunchPing = 1,     // ping the punch server to keep connection open
    kPunchRequest = 2,  // request the punch server send a Punch to another client
    kPunch = 3,         // the message sent by punch server in response to kPunchRequest
    kHolePunch = 4      // simple packet sent directly between clients with the sender's connection ID
};

OSErr PascalStringToAddress(StringPtr name, ip_addr *addr);
OSErr AddressToPascalString(ip_addr addr, StringPtr name);

OSErr OpenAvaraTCP();

typedef void ReadCompleteProc(UDPpacket *packet, void *userData);
typedef void WriteCompleteProc(int result, void *userData);

UDPpacket * CreatePacket(int bufferSize);
void FreePacket(UDPpacket *packet);

int ResolveHost(IPaddress *address, const char *host, uint16_t port);

void PunchSetup(const char *host, uint16_t port);
void RegisterPunchServer(IPaddress &localAddr);
void RequestPunch(IPaddress &addr);
void PunchHole(const IPaddress &addr, const int8_t connectionId);

int CreateSocket(uint16_t &port /* value changed when port==0 */);
void DestroySocket(int sock);
void CheckSockets();

void UDPRead(int sock, ReadCompleteProc callback, void *userData);
void UDPWrite(int sock, UDPpacket *packet, WriteCompleteProc callback, void *userData);

std::string FormatHostPort(uint32_t host, uint16_t port);
std::string FormatAddress(const IPaddress &addr);

// call this handler when any PunchPacket received
typedef std::function<void(PunchType cmd, const IPaddress &addr, int8_t connId)> PunchHandler;
void SetPunchMessageHandler(PunchHandler handler);
