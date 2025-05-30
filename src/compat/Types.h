// Most of this is from https://opensource.apple.com/source/CarbonHeaders/CarbonHeaders-18.1/MacTypes.h
// See also https://opensource.apple.com/source/CarbonHeaders/CarbonHeaders-8A428/OSUtils.h

#pragma once

#define INTEL_ARCH

// included here to pull in NULL
#include <cstdlib>
#include <cstddef>
#include <stdint.h>
#include <string>

typedef bool Boolean;

typedef size_t Size;
typedef char *Ptr;
typedef Ptr *Handle;

typedef int32_t Fixed;
typedef int32_t Fract;
typedef uint32_t UnsignedFixed;

typedef uint8_t *StringPtr;
typedef uint8_t Str255[256];
static inline std::string ToString(Str255 s255) { return std::string((char *)s255 + 1, s255[0]); }

typedef uint8_t Byte;
typedef int8_t SignedByte;

typedef int32_t FrameNumber;
typedef int32_t FrameTime;   // 64,32,16(default),8

typedef uint32_t ip_addr;
typedef uint16_t port_num;

typedef int32_t ClockTick;   // integer counter returned by UDPComm::GetClock()

struct Rect {
    Fixed top;
    Fixed left;
    Fixed bottom;
    Fixed right;
};
typedef struct Rect Rect;

typedef int16_t OSErr;
typedef uint32_t OSType;

// This is certainly not the right way to do this.
#define noErr 0
#define qErr -1
#define memFullErr -108
#define eofErr -39
#define mFulErr -41
#define tmfoErr -42
#define fnfErr -43
#define writErr -20
#define afpPwdExpiredErr -5042

struct Point {
    int16_t v;
    int16_t h;
};
typedef struct Point Point;

struct QElem {
    struct QElem *qLink;
};
typedef struct QElem QElem;
typedef QElem *QElemPtr;
struct QHdr {
    QElemPtr qHead;
    QElemPtr qTail;
    size_t qSize;
};
typedef struct QHdr QHdr;
typedef QHdr *QHdrPtr;

// From QuickDraw
enum { kQDGrafVerbFrame = 0, kQDGrafVerbPaint = 1, kQDGrafVerbErase = 2, kQDGrafVerbInvert = 3, kQDGrafVerbFill = 4 };

typedef uint8_t GrafVerb;

#ifdef _WIN32
#include <winsock2.h>
#include <Windows.h>
#else
#include <arpa/inet.h>
#endif
