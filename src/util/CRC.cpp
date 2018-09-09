/*
    Copyright ©1995, Juri Munkki
    All rights reserved.

    File: CRC.c
    Created: Monday, June 5, 1995, 08:27
    Modified: Tuesday, June 6, 1995, 09:27
*/

#include "CRC.h"

/*
    Calculate a 16-bit CCITT CRC checksum
*/
unsigned short CRC16(unsigned char *pkt, long len) {
    unsigned short c;
    unsigned short crc = 0;

    static unsigned short crcTableA[16] = {0L,
        010201L,
        020402L,
        030603L,
        041004L,
        051205L,
        061406L,
        071607L,
        0102010L,
        0112211L,
        0122412L,
        0132613L,
        0143014L,
        0153215L,
        0163416L,
        0173617L};

    static unsigned short crcTableB[16] = {0L,
        010611L,
        021422L,
        031233L,
        043044L,
        053655L,
        062466L,
        072277L,
        0106110L,
        0116701L,
        0127532L,
        0137323L,
        0145154L,
        0155745L,
        0164576L,
        0174367L};

    while (len--) {
        c = crc ^ *pkt++;
        crc = (crc >> 8) ^ (crcTableA[(c & 0xF0) >> 4] ^ crcTableB[c & 0x0F]);
    }

    return crc;
}

unsigned short CRCHandle(Handle theHandle) {
    if (theHandle) {
        return CRC16((unsigned char *)*theHandle, GetHandleSize(theHandle));
    } else {
        return 0;
    }
}
