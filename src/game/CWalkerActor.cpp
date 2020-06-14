/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CWalkerActor.c
    Created: Monday, February 6, 1995, 11:59
    Modified: Monday, September 16, 1996, 19:28
*/

#include "CWalkerActor.h"

#include "AvaraDefines.h"
#include "CBSPWorld.h"
#include "CDepot.h"
#include "CPlayerMissile.h"
#include "CSmartPart.h"
#include "CViewParameters.h"
#include "KeyFuncs.h"

#define SCOUTPLATFORM FIX3(1500)
#define MAXHEADHEIGHT FIX3(1750)
#define MINHEADHEIGHT FIX3(900)
#define BESTSPEEDHEIGHT FIX3(1700)
#define HEADSCALEFACTOR FIX3(2) /*	Multiply mouse.v by this to convert to height delta. */

#define LEGSPACE FIX3(-600)
#define LEGHIGHLEN FIX3(905)
#define LEGLOWLEN FIX3(1150) /*FIX3(1050)*/
#define MAXLEGLEN FIX3(2000)

#define HULLMASS FIX(150)
#define BASEMASS FIX(165)

#define noMARKERCUBE

void CWalkerActor::LoadParts() {
    short i;

    viewPortPart = partList[0];

#ifdef MARKERCUBE
    markerCube = new CBSPPart;
    markerCube->IBSPPart(213);
    itsGame->itsWorld->AddPart(markerCube);
#endif

    partCount = 5;
    LoadPart(0, kHeadBSP);
    LoadPart(1, kLegHighBSP);
    LoadPart(2, kLegHighBSP);
    LoadPart(3, kLegLowBSP);
    LoadPart(4, kLegLowBSP);

    for (i = 0; i < 5; i++) {
        partList[i]->usesPrivateHither = true;
        partList[i]->hither = FIX3(100);
    }

    viewPortPart = partList[0];
    proximityRadius = viewPortPart->enclosureRadius << 2;
}

void CWalkerActor::StartSystems() {
    CAbstractPlayer::StartSystems();

    mass = HULLMASS;
    baseMass = BASEMASS;

    elevation = BESTSPEEDHEIGHT;
    scoutBaseHeight = elevation + SCOUTPLATFORM;
    stance = BESTSPEEDHEIGHT;
    crouch = 0;
    tractionFlag = true;
    oldTractionFlag = true;
    jumpFlag = false;

    baseFriction = motorFriction;
    minPitch = FIX(-30);
    maxPitch = FIX(30);

    legConstant = FMul(LEGHIGHLEN, LEGHIGHLEN) - FMul(LEGLOWLEN, LEGLOWLEN);

    legPhase = 0;
    absAvgSpeed = 0;
    legs[0].x = 0;
    legs[0].y = 0;
    legs[0].touchIdent = 0;
    legs[1].x = 0;
    legs[1].y = 0;
    legs[1].touchIdent = 0;
    targetHeight = 0;

    jumpBasePower = FIX3(700);

    viewPortHeight = FIX3(350);
}

void CWalkerActor::Dispose() {
#ifdef MARKERCUBE
    itsGame->itsWorld->RemovePart(markerCube);
    markerCube->Dispose();
#endif
    CAbstractPlayer::Dispose();
}

void CWalkerActor::PlaceParts() {
    short i;
    CSmartPart *aPart;
    Fixed deltaY;

    aPart = partList[0];
    aPart->Reset();
    InitialRotatePartZ(aPart, FMul(viewYaw, -30)); //	Divide by -6 and convert to "one" angles
    aPart->RotateX(viewPitch);
    aPart->RotateY(viewYaw);
    TranslatePartY(aPart, viewPortHeight);

    aPart = partList[1];
    aPart->Reset();
    InitialRotatePartX(aPart, legs[0].highAngle);
    TranslatePartX(aPart, -LEGSPACE);

    aPart = partList[2];
    aPart->Reset();
    InitialRotatePartX(aPart, legs[1].highAngle);
    PreFlipY(aPart);

    TranslatePartX(aPart, LEGSPACE);

    aPart = partList[3];
    aPart->Reset();
    InitialRotatePartX(aPart, legs[0].lowAngle);
    TranslatePartY(aPart, -LEGHIGHLEN);
    aPart->RotateOneX(legs[0].highAngle);
    TranslatePartX(aPart, -LEGSPACE);

    aPart = partList[4];
    aPart->Reset();
    InitialRotatePartX(aPart, legs[1].lowAngle);
    PreFlipY(aPart);

    TranslatePartY(aPart, -LEGHIGHLEN);
    aPart->RotateOneX(legs[1].highAngle);
    TranslatePartX(aPart, LEGSPACE);

    deltaY = location[1] + headHeight;

    for (i = 0; i < partCount; i++) {
        aPart = partList[i];

        aPart->RotateOneY(heading);
        TranslatePart(aPart, location[0], deltaY, location[2]);
        aPart->MoveDone();
    }

    CAbstractPlayer::PlaceParts();
}

void CWalkerActor::AvoidBumping() {
    enum { undoMotion, undoElevation, undoHeadTurn, undoLegs, undoTurn, undoCount };

    short undoStep;
    CSmartPart *objHit, *objTest;

    PlaceParts();

    objHit = DoCollisionTest(&proximityList.p);

    if (objHit) {
        Fixed oldY;

        oldY = speed[1];
        didBump = true;
        undoStep = 0;

        do {
            switch (undoStep) {
                case undoMotion:
                    if (speed[0] || speed[1] || speed[2]) {
                        location[0] -= speed[0];
                        location[2] -= speed[2];
                        location[1] -= speed[1];
                        goto redoLegs;
                        break;
                    }
                    undoStep++; //	No speed and we still collide?

                case undoElevation:
                    if (crouchUndo != crouch || stanceUndo != stance) {
                        stance = stanceUndo;
                        crouch = crouchUndo;
                        goto redoLegs;
                    }
                    undoStep++; //	Fall through

                case undoHeadTurn:
                    if (oldYaw != viewYaw || viewPitch != oldPitch) {
                        viewYaw = oldYaw;
                        viewPitch = oldPitch;
                        goto redoLegs;
                    }
                    undoStep++; //	Fall through

#define DIFFERENTLEGS(a, b) ((a.highAngle != b.highAngle) || (a.lowAngle != b.lowAngle))

                case undoLegs:
#ifdef TESTIT
                    if (DIFFERENTLEGS(legs[0], legUndo[0]) || DIFFERENTLEGS(legs[1], legUndo[1]) ||
                        legPhase != phaseUndo)
#endif
                    {
                        legs[0] = legUndo[0];
                        legs[1] = legUndo[1];
                        legPhase = phaseUndo;
                        break;
                    }
                    undoStep++;

                case undoTurn:
                    if (heading != headChange) {
                        Vector offset;
                        Vector push;

                        heading -= headChange;

                        PlaceParts();

                        offset[0] = FOneCos(heading) >> 4;
                        offset[1] = 0;
                        offset[2] = FOneSin(heading) >> 4;
                        OffsetParts(offset);

                        push[0] = speed[0] - (offset[0] << 1);
                        push[1] = speed[1];
                        push[2] = speed[2] - (offset[2] << 1);

                        if (DoCollisionTest(&proximityList.p)) {
                            Push(push);
                        }

                        offset[0] = -2 * offset[0];
                        offset[2] = -2 * offset[2];
                        OffsetParts(offset);

                        if (DoCollisionTest(&proximityList.p)) {
                            push[0] = speed[0] - offset[0];
                            push[1] = speed[1];
                            push[2] = speed[2] - offset[2];
                            Push(push);
                        }
                        break;
                    } else
                        goto nothingToUndo;

                redoLegs:
                    UndoLegs();
                    MoveLegs();
                    break;
            }

            PlaceParts();
            objTest = DoCollisionTest(&proximityList.p);

        } while (objTest && ++undoStep < undoCount);

    nothingToUndo:
        FindBestMovement(objHit);

        oldY = oldY > speed[1];
    } else {
        didBump = false;
    }

    DoLegTouches();

    if (speedLimit < 0) {
        speedLimit -= elevation >> 1;
        if (speed[1] < speedLimit) {
            speed[1] = speedLimit;
        }
    }
}

void CWalkerActor::DoStandingTouches() {
    CAbstractActor *touchActor;
    Fixed fricAcc, tracAcc;
    short tractCount = 0;

    fricAcc = 0;
    tracAcc = 0;

    if (legs[0].touchIdent) {
        touchActor = itsGame->FindIdent(legs[0].touchIdent);
        if (touchActor) {
            touchActor->StandingOn(this, legs[0].where, true);
            touchActor->GetFrictionTraction(&fricAcc, &tracAcc);
            tractCount = 1;
        }
    } else if (legs[0].where[1] < FIX3(50)) {
        tractCount = 1;
        tracAcc = itsGame->groundTraction;
        fricAcc = itsGame->groundFriction;
    }

    if (legs[1].touchIdent) {
        touchActor = itsGame->FindIdent(legs[1].touchIdent);
        if (touchActor) {
            Fixed fricAdd, tracAdd;

            touchActor->StandingOn(this, legs[1].where, legs[0].touchIdent != legs[1].touchIdent);
            touchActor->GetFrictionTraction(&fricAdd, &tracAdd);
            fricAcc += fricAdd;
            tracAcc += tracAdd;
            tractCount++;
        }
    } else if (legs[1].where[1] < FIX3(50)) {
        tractCount++;
        tracAcc += itsGame->groundTraction;
        fricAcc += itsGame->groundFriction;
    }

    if (tractCount) {
        if (tractCount == 2) {
            tracAcc >>= 1;
            fricAcc >>= 1;
        }
    } else {
        tracAcc = 0;

        if (didBump) {
            fricAcc = FIX3(50);
        } else {
            fricAcc = FIX3(5);
        }
    }

    supportTraction = tracAcc;
    supportFriction = fricAcc;
}

void CWalkerActor::DoLegTouches() {
    CAbstractActor *touchActor[2];
    short i;
    Fixed power[2];
    short soundId[2];

    if (legs[0].touchIdent) {
        touchActor[0] = itsGame->FindIdent(legs[0].touchIdent);
        if (touchActor[0])
            touchActor[0]->StandOn(this,
                legs[0].where,
                true,
                (legUndo[0].touchIdent != legs[0].touchIdent) && (legUndo[1].touchIdent != legs[0].touchIdent));
    } else {
        touchActor[0] = NULL;
    }

    if (legs[1].touchIdent) {
        touchActor[1] = itsGame->FindIdent(legs[1].touchIdent);
        if (touchActor[1])
            touchActor[1]->StandOn(this,
                legs[1].where,
                legs[0].touchIdent != legs[1].touchIdent,
                (legUndo[0].touchIdent != legs[1].touchIdent) && (legUndo[1].touchIdent != legs[1].touchIdent));
    } else {
        touchActor[1] = NULL;
    }

    if (itsGame->soundSwitches & kFootStepToggle) {
        for (i = 0; i < 2; i++) {
            Fixed lsp;

            power[i] = 0;
            soundId[i] = 0;

            lsp = legSpeeds[i];
            if (lsp < 0)
                lsp = -lsp;

            if (touchActor[i] && legUndo[i].touchIdent == 0) {
                power[i] = legUndo[i].where[1] - legs[i].where[1];
                if (power[i] < 0)
                    power[i] = -power[i];
                power[i] += lsp;
                soundId[i] = touchActor[i]->stepSound;
            } else if (legs[i].where[1] == 0 && legUndo[i].where[1] > 0) {
                power[i] = legUndo[i].where[1] + lsp;
                soundId[i] = itsGame->groundStepSound;
            }

            power[i] <<= 1;
        }

        if (soundId[0] == soundId[1]) {
            Vector spot;

            power[0] += power[1];
            for (i = 0; i < 3; i++) {
                spot[i] = (legs[0].where[i] + legs[1].where[i]) >> 1;
            }

            if (power[0] > FIX3(10) && soundId[0]) {
                DoSound(soundId[0], spot, power[0], FIX(1));
            }
        } else {
            for (i = 0; i < 2; i++) {
                if (power[i] > FIX3(10) && soundId[i]) {
                    DoSound(soundId[i], legs[i].where, power[i], FIX(1));
                }
            }
        }
    }
}

void CWalkerActor::ExtendLeg(LegInfo *theLeg) {
    Fixed extendLength;
    Fixed extendDelta[2];
    Fixed highPart, lowPart;
    Fixed normalPart;
    int acc[2];

    extendDelta[0] = -theLeg->x;
    extendDelta[1] = headHeight - theLeg->y;
    theLeg->highAngle = FOneArcTan2(extendDelta[1], extendDelta[0]);
    extendLength = VectorLength(2, extendDelta);
    if (extendLength > MAXLEGLEN) {
        extendLength = MAXLEGLEN;
    }

    highPart = (FDiv(legConstant, extendLength) + extendLength) >> 1;
    lowPart = extendLength - highPart;

    acc[0] = acc[1] = 0;
    FSquareAccumulate(LEGHIGHLEN, acc);
    FSquareSubtract(highPart, acc);
    normalPart = FSqroot(acc);

    theLeg->lowAngle = FOneArcTan2(highPart, normalPart);
    theLeg->highAngle = theLeg->highAngle + theLeg->lowAngle;
    theLeg->lowAngle += FOneArcTan2(lowPart, normalPart);
    theLeg->lowAngle = -theLeg->lowAngle;
}

#define fConstant FIX3(650 * 1.4)
void CWalkerActor::MoveLegs() {
    Fixed temp;
    Fixed phaseChange;
    short i;
    LegInfo *theLeg;
    RayHitRecord legSensor;
    Fixed tempSin, tempCos;

    speedLimit = speed[1];

    tractionFlag = false;
    tempSin = FOneSin(heading);
    tempCos = FOneCos(heading);

    targetHeight = 0;

    elevation = stance - crouch;
    headHeight = elevation + FMul(viewPitch, FIX3(10));
    scoutBaseHeight = elevation + SCOUTPLATFORM;

    temp = FMul(-LEGSPACE * 18, headChange);
    legSpeeds[0] = distance - temp;
    legSpeeds[1] = distance + temp;

    absAvgSpeed = (legSpeeds[0] > 0) ? legSpeeds[0] : -legSpeeds[0];
    absAvgSpeed += (legSpeeds[1] > 0) ? legSpeeds[1] : -legSpeeds[1];

    phaseChange = FMulDiv(FSqrt(absAvgSpeed), fConstant, elevation);
    legPhase += phaseChange / 10;

    for (i = 0; i < 2; i++) {
        Fixed moveRadius;
        Fixed targetX, targetY;
        Fixed tempX, tempZ;
        Fixed theSpeed = legSpeeds[i];

        theLeg = &legs[i];
        theLeg->x -= theSpeed / 4;

        if (phaseChange)
            moveRadius = FDivNZ(theSpeed, phaseChange);
        else
            moveRadius = theSpeed;

        targetX = -FMul(FOneCos(legPhase), moveRadius);
        if (moveRadius < 0)
            moveRadius = -moveRadius;
        targetY = location[1] - (elevation >> 4) + FMul(FOneSin(legPhase), moveRadius);

        tempX = i ? -LEGSPACE : LEGSPACE;
        tempZ = (targetX + theLeg->x) >> 1;

        legSensor.origin[0] = location[0] + FMul(tempZ, tempSin) - FMul(tempX, tempCos);
        legSensor.origin[1] = location[1] + FIX3(200) + (elevation >> 1);
        legSensor.origin[2] = location[2] + FMul(tempZ, tempCos) + FMul(tempX, tempSin);
        legSensor.direction[0] = 0;
        legSensor.direction[1] = FIX(-1);
        legSensor.direction[2] = 0;

        tempZ = elevation + FIX3(200);
        if (speedLimit < 0) {
            tempZ -= speedLimit;
        }

        if (tempZ > legSensor.origin[1]) {
            tempZ = legSensor.origin[1];
        }
        legSensor.distance = tempZ;

        legSensor.closestHit = NULL;
        RayTest(&legSensor, kSolidBit);

        if (legSensor.distance < -speedLimit) {
            speedLimit = -legSensor.distance;
        }

        tempZ = legSensor.origin[1] - legSensor.distance;

        if (tempZ > targetHeight)
            targetHeight = tempZ;

        if (tempZ + FIX3(100) >= location[1]) {
            tractionFlag = true;
        }

        theLeg->y += location[1]; //	Convert theLeg->y into global coordinates
        theLeg->y = (targetY + theLeg->y) >> 1;

        if (theLeg->y > tempZ) //	Leg is in the air?
        {
            theLeg->x = (targetX + theLeg->x) >> 1;
            theLeg->touchIdent = 0;
        } else //	Standing on something
        {
            if (legSensor.closestHit) {
                theLeg->touchIdent = ((CAbstractActor *)legSensor.closestHit->theOwner)->ident;
            } else {
                theLeg->touchIdent = 0;
            }
            theLeg->x = (targetX - theSpeed + theLeg->x) >> 1;
            theLeg->y = tempZ;
        }

        theLeg->where[0] = legSensor.origin[0];
        theLeg->where[1] = theLeg->y;
        theLeg->where[2] = legSensor.origin[2];

        theLeg->y -= location[1]; //	Make theLeg->y relative to location
        legPhase += FIX3(500); //	Next leg (always 180 degree phase difference

#ifdef MARKERCUBE
        if (i == 0) {
            markerCube->Reset();

            if (false) // newWayFlag
            {
                TranslatePart(markerCube, -LEGSPACE, theLeg->y, theLeg->x);
                markerCube->RotateOneY(heading);
                TranslatePart(markerCube, location[0], location[1], location[2]);
            } else {
                TranslatePart(markerCube, legSensor.origin[0], tempZ, legSensor.origin[2]);
            }
            markerCube->MoveDone();
        }
#endif
    }

    ExtendLeg(&legs[0]);
    ExtendLeg(&legs[1]);
}

void CWalkerActor::TractionControl() {
    Fixed extraHeight;
    Fixed bounceTarget;
    Fixed adjustedGravity;

    DoStandingTouches();

    motorFriction = elevation - BESTSPEEDHEIGHT;
    if (motorFriction < 0)
        motorFriction = -motorFriction;
    motorFriction = baseFriction - (motorFriction >> 2);

    adjustedGravity = FMul(FIX3(120), itsGame->gravityRatio);
    speed[1] -= adjustedGravity;

    bounceTarget = FMul(absAvgSpeed, (0x4000 - (legPhase & 0x7FFF)) >> 2);
    if (bounceTarget > 0)
        bounceTarget = -bounceTarget;
    bounceTarget += targetHeight;

    extraHeight = bounceTarget + adjustedGravity * 2; // FIX3(120)*2;

    if (!jumpFlag && location[1] < extraHeight) {
        speed[1] = ((bounceTarget - location[1]) >> 1) + (speed[1] >> 1);
    }

    if (speed[1] < 0)
        jumpFlag = false;

    CAbstractPlayer::TractionControl();
}

void CWalkerActor::MotionControl() {
    CAbstractPlayer::MotionControl();

    MoveLegs();
}

void CWalkerActor::KeyboardControl(FunctionTable *ft) {
    Fixed savedAcceleration;

    PrepareForLegUndo();

    if (tractionFlag && !oldTractionFlag)
        jumpFlag = false;

    oldTractionFlag = tractionFlag;

    CAbstractPlayer::KeyboardControl(ft);

    if (!isInLimbo) {
        if (TESTFUNC(kfuVerticalMotion, ft->held)) {
            viewPitch = oldPitch;

            stance -= HEADSCALEFACTOR * ft->mouseDelta.v;

            if (stance > MAXHEADHEIGHT)
                stance = MAXHEADHEIGHT;
            else if (stance < MINHEADHEIGHT)
                stance = MINHEADHEIGHT;
        }

        if (TESTFUNC(kfuJump, ft->down)) {
            crouch += (stance - crouch - MINHEADHEIGHT) >> 3;
        } else if (TESTFUNC(kfuJump, ft->held)) {
            crouch += (stance - crouch - MINHEADHEIGHT) >> 2;
        } else {
            crouch >>= 1;
        }

        if (TESTFUNC(kfuJump, ft->up) && tractionFlag) {
            speed[1] >>= 1;
            speed[1] += FMulDivNZ((crouch >> 1) + jumpBasePower, baseMass, GetTotalMass());
            jumpFlag = true;
        }
    }
}

void CWalkerActor::PlayerWasMoved() {
    PrepareForLegUndo();

    targetHeight = location[1];
    MoveLegs();

    CAbstractPlayer::PlayerWasMoved();

    UndoLegs();
}

void CWalkerActor::PrepareForLegUndo() {
    legUndo[0] = legs[0];
    legUndo[1] = legs[1];
    phaseUndo = legPhase;

    crouchUndo = crouch;
    stanceUndo = stance;
}

void CWalkerActor::UndoLegs() {
    legPhase = phaseUndo;
    legs[0] = legUndo[0];
    legs[1] = legUndo[1];
}

void CWalkerActor::ReceiveConfig(PlayerConfigRecord *config) {
    if (itsGame->frameNumber == 0) {
        short hullRes;
        HullConfigRecord hull;
        Handle hullHandle;

        hullRes = config->hullType;
        if (hullRes < 0 || hullRes > 2)
            hullRes = 1;

        hullRes = ReadLongVar(iFirstHull + hullRes);

        hullHandle = GetResource('HULL', hullRes);
        if (hullHandle == NULL)
            hullHandle = GetResource('HULL', 129);

        // TODO: good candidate for JSON
        hull = **(HullConfigRecord **)hullHandle;
        hull.hullBSP = ntohs(hull.hullBSP);
        hull.maxMissiles = ntohs(hull.maxMissiles);
        hull.maxGrenades = ntohs(hull.maxGrenades);
        hull.maxBoosters = ntohs(hull.maxBoosters);
        hull.mass = ntohl(hull.mass);
        hull.energyRatio = ntohl(hull.energyRatio);
        hull.energyChargeRatio = ntohl(hull.energyChargeRatio);
        hull.shieldsRatio = ntohl(hull.shieldsRatio);
        hull.shieldsChargeRatio = ntohl(hull.shieldsChargeRatio);
        hull.minShotRatio = ntohl(hull.minShotRatio);
        hull.maxShotRatio = ntohl(hull.maxShotRatio);
        hull.shotChargeRatio = ntohl(hull.shotChargeRatio);
        hull.rideHeight = ntohl(hull.rideHeight);
        hull.accelerationRatio = ntohl(hull.accelerationRatio);
        hull.jumpPowerRatio = ntohl(hull.jumpPowerRatio);

        hullRes = hull.hullBSP;
        itsGame->itsWorld->RemovePart(viewPortPart);
        viewPortPart->Dispose();
        LoadPart(0, hullRes);

        viewPortPart = partList[0];
        viewPortPart->usesPrivateHither = true;
        // set magic value to turn on backface culling
        // for this part (so we can see through it with the camera)
        viewPortPart->hither = FIX3(101);
        viewPortPart->ReplaceColor(kMarkerColor, longTeamColor);

        proximityRadius = viewPortPart->enclosureRadius << 2;

        itsGame->itsWorld->AddPart(viewPortPart);

        viewPortHeight = hull.rideHeight;

        PlaceParts();

        mass = hull.mass;

        if (hull.maxBoosters < boosterLimit)
            boosterLimit = hull.maxBoosters;

        if (hull.maxMissiles < missileLimit)
            missileLimit = hull.maxMissiles;

        if (hull.maxGrenades < grenadeLimit)
            grenadeLimit = hull.maxGrenades;

        maxShields = FMul(maxShields, hull.shieldsRatio);
        shieldRegen = FMul(shieldRegen, hull.shieldsChargeRatio);
        shields = maxShields;

        maxEnergy = FMul(maxEnergy, hull.energyRatio);
        energy = maxEnergy;
        generatorPower = FMul(generatorPower, hull.energyChargeRatio);

        chargeGunPerFrame = FMul(chargeGunPerFrame, hull.shotChargeRatio);
        activeGunEnergy = FMul(activeGunEnergy, hull.minShotRatio);
        fullGunEnergy = FMul(fullGunEnergy, hull.maxShotRatio);

        maxAcceleration = FMul(maxAcceleration, hull.accelerationRatio);
        jumpBasePower = FMul(jumpBasePower, hull.jumpPowerRatio);
        if (hullHandle) {
            ReleaseResource(hullHandle);
        }

        gunEnergy[0] = fullGunEnergy;
        gunEnergy[1] = fullGunEnergy;
    }

    CAbstractPlayer::ReceiveConfig(config);
}
