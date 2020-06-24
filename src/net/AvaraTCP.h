/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: AvaraTCP.h
    Created: Sunday, January 28, 1996, 17:22
    Modified: Tuesday, September 17, 1996, 05:14
*/

#pragma once

#include "Types.h"
#include <SDL_net.h>

#define UDPSTREAMBUFFERSIZE 16384
#define UDPSENDBUFFERSIZE 1024

OSErr PascalStringToAddress(StringPtr name, ip_addr *addr);
OSErr AddressToPascalString(ip_addr addr, StringPtr name);

OSErr OpenAvaraTCP();

typedef void ReadCompleteProc(UDPpacket *packet, void *userData);
typedef void WriteCompleteProc(int result, void *userData);

UDPsocket CreateSocket(uint16_t port);
void DestroySocket(UDPsocket sock);
void CheckSockets();

void UDPRead(UDPsocket sock, ReadCompleteProc callback, void *userData);
void UDPWrite(UDPsocket sock, UDPpacket *packet, WriteCompleteProc callback, void *userData);
