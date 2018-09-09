/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CPill.c
    Created: Tuesday, August 27, 1996, 04:34
    Modified: Wednesday, September 4, 1996, 00:12
*/

#include "CPill.h"

#define kBSPPill 252
#define kStandardPillMass FIX(40)
#define kPillGravity FIX3(120)
#define kFriction FIX3(800)

void CPill::IAbstractActor() {
    CBall::IAbstractActor();

    mass = kStandardPillMass;

    shieldsCharge = FIX3(5);
    maxShields = FIX(17);
    shootShields = FIX(15);
    shields = maxShields;
    dropDamage = FIX(1);

    holdShieldLimit = shootShields;

    group = 0;

    shotPower = FIX3(300);
    burstLength = 4;
    burstSpeed = 6;
    burstCharge = 32;

    customGravity = kPillGravity;

    shapeRes = kBSPPill;
    acceleration = kFriction;

    ownerChangeCount = 0;
    ownerChangeTime = 100;

    ejectPitch = FIX(5);
    ejectPower = FIX3(300);

    watchMask = kPlayerBit + kScoutBit + kRobotBit;

    reprogramSound = 433;
}
