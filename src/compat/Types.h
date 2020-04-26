// Most of this is from https://opensource.apple.com/source/CarbonHeaders/CarbonHeaders-18.1/MacTypes.h
// See also https://opensource.apple.com/source/CarbonHeaders/CarbonHeaders-8A428/OSUtils.h

#pragma once

#define INTEL_ARCH

// included here to pull in NULL
#include <cstdlib>
#include <stdint.h>

typedef bool Boolean;

typedef long Size;
typedef char *Ptr;
typedef Ptr *Handle;

typedef int32_t Fixed;
typedef int32_t Fract;
typedef uint32_t UnsignedFixed;

typedef unsigned char *StringPtr;
typedef unsigned char Str255[256];

typedef unsigned char UnsignedByte;
typedef signed char SignedByte;

typedef uint32_t ip_addr;

struct Rect {
    short top;
    short left;
    short bottom;
    short right;
};
typedef struct Rect Rect;

typedef short OSErr;
typedef unsigned int OSType;

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
    short v;
    short h;
};
typedef struct Point Point;

struct QElem {
    struct QElem *qLink;
    short qType;
    short qData[1];
};
typedef struct QElem QElem;
typedef QElem *QElemPtr;
struct QHdr {
    volatile short qFlags;
    volatile QElemPtr qHead;
    volatile QElemPtr qTail;
};
typedef struct QHdr QHdr;
typedef QHdr *QHdrPtr;

struct RGBColor {
    unsigned short red;
    unsigned short green;
    unsigned short blue;
};
typedef struct RGBColor RGBColor;
struct ColorSpec {
    short value;
    RGBColor rgb;
};
typedef struct ColorSpec ColorSpec;
typedef ColorSpec CSpecArray[1];
struct ColorTable {
    long ctSeed;
    short ctFlags;
    short ctSize;
    CSpecArray ctTable;
};
typedef struct ColorTable ColorTable;

// From QuickDraw
enum { kQDGrafVerbFrame = 0, kQDGrafVerbPaint = 1, kQDGrafVerbErase = 2, kQDGrafVerbInvert = 3, kQDGrafVerbFill = 4 };

typedef uint8_t GrafVerb;

typedef struct GLData {
    float x, y, z, r, g, b, nx, ny, nz;
} GLData;

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
