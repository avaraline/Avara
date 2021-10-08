/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: AvaraDefines.h
    Created: Monday, March 6, 1995, 03:30
    Modified: Thursday, August 22, 1996, 03:18
*/

#pragma once

#define kMaxAvaraPlayers 8
#define kMaxTeamColors 8

#define STANDARDMISSILERANGE FIX(100)
#define LONGYON FIX(180)

#define kSliverSizes 3
enum { kSmallSliver, kMediumSliver, kLargeSliver };

#define kDefaultTraction FIX3(400)
#define kDefaultFriction FIX3(100)

#define CLASSICFRAMETIME 64
#define GAMETICKINTERVAL 16 

// From CInfoPanel
enum { rightAlign = -1, leftAlign, centerAlign };

enum { kipDrawMessage = 1, kipDrawName = 2, kipDrawColorBox = 4, kipDrawErase = 8, kipDrawValidate = 16 };
