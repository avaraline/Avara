/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: AvaraTCP.h
    Created: Sunday, January 28, 1996, 17:22
    Modified: Tuesday, September 17, 1996, 05:14
*/

#pragma once

#include "Types.h"

#define UDPSTREAMBUFFERSIZE 16384
#define UDPSENDBUFFERSIZE 1024

typedef struct {
    u_int32_t host;
    u_int16_t port;
} IPaddress;

typedef struct {
    u_int8_t *data;
    unsigned int len;
    IPaddress address;
} UDPpacket;

OSErr PascalStringToAddress(StringPtr name, ip_addr *addr);
OSErr AddressToPascalString(ip_addr addr, StringPtr name);

OSErr OpenAvaraTCP();

typedef void ReadCompleteProc(UDPpacket *packet, void *userData);
typedef void WriteCompleteProc(int result, void *userData);

UDPpacket * CreatePacket(int bufferSize);
void FreePacket(UDPpacket *packet);

int ResolveHost(IPaddress *address, const char *host, u_int16_t port);

int CreateSocket(uint16_t port);
void DestroySocket(int sock);
void CheckSockets();

void UDPRead(int sock, ReadCompleteProc callback, void *userData);
void UDPWrite(int sock, UDPpacket *packet, WriteCompleteProc callback, void *userData);
