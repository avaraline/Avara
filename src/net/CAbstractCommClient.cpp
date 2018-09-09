/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CAbstractCommClient.c
    Created: Monday, October 30, 1995, 18:55
    Modified: Thursday, February 22, 1996, 09:04
*/

#include "CAbstractCommClient.h"

/*
**	Initialize object. Note that the id is
**	unknown until a connection is opened.
*/
void CAbstractCommClient::ICommClient(short bufferCount) {
    isConnected = false;
    isOpen = false;

    ICommManager(bufferCount);
    myId = -1; //	Unknown until connected
}

/*
**	Connect to a server.
*/
void CAbstractCommClient::Connect() {}

/*
**	Break the connection.
*/
void CAbstractCommClient::Disconnect() {}
