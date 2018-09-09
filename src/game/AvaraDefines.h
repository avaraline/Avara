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

enum {
    kStatusTextAndBorderColor = 1,
    kShadowGrayColor,
    kAvaraBackColor,
    kStatusGreenColor,
    kStatusRedColor
};

#define kGreenTeamColor 0x00006600
#define kYellowTeamColor 0x00cccc00
#define kRedTeamColor 0x00cc0000
#define kPinkTeamColor 0x00cc0099
#define kPurpleTeamColor 0x009900cc
#define kBlueTeamColor 0x000099cc
#define kSpecialBlackColor 0x00303030
#define kSpecialWhiteColor 0x00e0e0e0

#define kStatusAreaHeight 0 /*29*/
#define kIndicatorAreaWidth 80
#define kIndicatorLabelWidth 11
#define kFirstStatusLine 12
#define kStatusLineHeight 11
#define kSecondStatusLine (kFirstStatusLine + kStatusLineHeight)
#define kIndicatorBarHeight 9

#define kEnergyTop 4
#define kEnergyLine kFirstStatusLine

#define kShieldsTop (kEnergyTop + kIndicatorBarHeight + 2)
#define kShieldsLine kSecondStatusLine
#define kBarLabelOffset 9

#define kScoreTop 4
#define kScoreLine kFirstStatusLine

#define kTimeTop (kScoreTop + kIndicatorBarHeight + 2)
#define kTimeLine kSecondStatusLine

#define kMessageAreaMargin 3

#define STANDARDMISSILERANGE FIX(100)
#define LONGYON FIX(180)
#define SHORTYON FIX(75)

#define kSliverSizes 3
enum { kSmallSliver, kMediumSliver, kLargeSliver };

#define kAvaraBroadcastDDPSocket 0x67 /*	Just a number off the top of my head. (I was born in 1967) */
#define kAvaraDDPProtocolOffset 128

#define kDefaultTraction FIX3(400)
#define kDefaultFriction FIX3(100)

// From CInfoPanel
enum { kliMissiles, kliBoosters, kliGrenades, kliLives };
enum { rightAlign = -1, leftAlign, centerAlign };

enum {
    kipScore,
    kipOtherScore1,
    kipOtherScore2,
    kipOtherScore3,
    kipOtherScore4,
    kipOtherScore5,
    kipOtherScore6,
    kipLives,
    kipTimer,
    kipConsole1,
    kipConsole2,
    kipConsole3,
    kipConsole4,
    kipMessage1,
    kipMessage2,
    kipMessage3,
    kipMessage4,
    kipMessage5,
    kipMessage6,
    kipPlayerName1,
    kipPlayerName2,
    kipPlayerName3,
    kipPlayerName4,
    kipPlayerName5,
    kipPlayerName6,

    kipKeyCount
};

enum { kipDrawMessage = 1, kipDrawName = 2, kipDrawColorBox = 4, kipDrawErase = 8, kipDrawValidate = 16 };
