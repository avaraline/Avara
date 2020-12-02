/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: KeyFuncs.h
    Created: Tuesday, November 22, 1994, 05:14
    Modified: Friday, February 2, 1996, 10:41
*/

#pragma once

#include "Types.h"

typedef struct {
    uint32_t down;
    uint32_t up;
    uint32_t held;
    /*
    long	down[2];
    long	up[2];
    long	held[2];
    */
    Point mouseDelta;
    uint8_t buttonStatus;
    char msgChar;
} FunctionTable;

typedef struct {
    FunctionTable ft;
    uint32_t validFrame;
} FrameFunction;

//	Flags for "p1" of packet.
#define kbuIsDown 1
#define kbuWentDown 2
#define kbuWentUp 4
#define kNoDownData 0x80
#define kNoUpData 0x40
#define kNoHeldData 0x20
#define kNoMouseV 0x10
#define kNoMouseH 0x08
#define kDataFlagMask (char)(kNoDownData | kNoUpData | kNoHeldData | kNoMouseH | kNoMouseV)

#define FUNMAPID 8000
#define FUNMAPTYPE 'FMAP'

#define TESTFUNC(n, map) ((map & (1 << n)) != 0)

enum {
    kfuForward,
    kfuReverse,
    kfuLeft,
    kfuRight,
    /*
        kfuLeftForward,
        kfuRightForward,
        kfuLeftReverse,
        kfuRightReverse,
    */
    kfuVerticalMotion,

    kfuFireWeapon,
    kfuLoadGrenade,
    kfuLoadMissile,
    kfuBoostEnergy,
    kfuTypeText,

    kfuZoomIn,
    kfuZoomOut,

    kfuScoutView,
    kfuScoutControl,
    
    kfuSpectateNext,
    kfuSpectatePrevious,
    
    kfuScoreboard,

    kfuPauseGame,
    kfuAbortGame,

    kfuDebug1,
    kfuDebug2,

    kfuJump,
    kfuLookLeft,
    kfuLookRight,
    kfuAimForward,

    kKeyFuncCount
};

/*
extern	Point			MTemp;
extern	Point			RawMouse;
extern	Point			Mouse;
extern	short			CrsrNewCouple;
extern	Byte			CrsrNew;
extern	Byte			CrsrCouple;
extern	Byte			CrsrScale;
*/
