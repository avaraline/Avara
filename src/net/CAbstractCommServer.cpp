/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CAbstractCommServer.c
    Created: Monday, October 30, 1995, 18:50
    Modified: Friday, November 10, 1995, 18:37
*/

#include "CAbstractCommServer.h"

/*
**	Initialize object for max number of clients and
**	maximum number of packet buffers. Make sure that
**	both are sufficient for your application!
*/
void CAbstractCommServer::ICommServer(short clientCount, short bufferCount) {
    isServing = false;

    ICommManager(bufferCount);
}

/*
**	Start the server.
*/
void CAbstractCommServer::StartServing() {
    //	Subclass responsibility
}

/*
**	Close down service.
*/
void CAbstractCommServer::StopServing() {
    //	Subclass responsibility
}
