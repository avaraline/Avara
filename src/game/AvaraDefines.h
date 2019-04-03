/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: AvaraDefines.h
    Created: Monday, March 6, 1995, 03:30
    Modified: Thursday, August 22, 1996, 03:18
*/

#pragma once

#define kMaxAvaraPlayers 6
#define kMaxTeamColors 6

#define kNeutralTeamColor 0x00ffffff
#define kGreenTeamColor 0x00006600
#define kYellowTeamColor 0x00cccc00
#define kRedTeamColor 0x00cc0000
#define kPinkTeamColor 0x00cc0099
#define kPurpleTeamColor 0x009900cc
#define kBlueTeamColor 0x000099cc
#define kSpecialBlackColor 0x00303030
#define kSpecialWhiteColor 0x00e0e0e0

#define STANDARDMISSILERANGE FIX(100)
#define LONGYON FIX(180)
#define SHORTYON FIX(75)

#define kSliverSizes 3
enum { kSmallSliver, kMediumSliver, kLargeSliver };

#define kDefaultTraction FIX3(400)
#define kDefaultFriction FIX3(100)

#define CLASSICFRAMETIME 64

// From CInfoPanel
enum { rightAlign = -1, leftAlign, centerAlign };

enum { kipDrawMessage = 1, kipDrawName = 2, kipDrawColorBox = 4, kipDrawErase = 8, kipDrawValidate = 16 };
