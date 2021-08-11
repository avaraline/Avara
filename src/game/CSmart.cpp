/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CSmart.c
    Created: Wednesday, February 14, 1996, 01:39
    Modified: Tuesday, September 17, 1996, 07:36
*/
// #define ENABLE_FPS_DEBUG  // uncomment if you want to see FPS_DEBUG output for this file

#include "CSmart.h"

#include "CDepot.h"
#include "CSmartPart.h"
//#include "CInfoPanel.h"
#include "CAvaraApp.h"

#define kSmartAccel itsDepot->missileAcceleration
#define kSmartFriction FIX3(50)

void CSmart::IWeapon(CDepot *theDepot) {
    CWeapon::IWeapon(theDepot);

    partCount = 1;
    LoadPart(0, 802);
    partList[0]->usesPrivateHither = true;
    partList[0]->hither = FIX3(100);

    mass = FIX(1);
}

void CSmart::ResetWeapon() {
    CWeapon::ResetWeapon();

    goodYaw = goodPitch = 0;
    sightDistance = 0;
    thrust = 0;
    inSight = 0;
}

void CSmart::PlaceParts() {
    partList[0]->Reset();

    if (flyCount == 0 && hostPart) {
        TranslatePart(partList[0], 0, FIX3(450), FIX3(600));
        partList[0]->ApplyMatrix(&hostPart->itsTransform);
    } else {
        InitialRotatePartZ(partList[0], roll);
        partList[0]->RotateOneX(pitch);
        partList[0]->RotateOneY(yaw);
        TranslatePart(partList[0], location[0], location[1], location[2]);
    }

    partList[0]->MoveDone();
    LinkSphere(partList[0]->itsTransform[3], partList[0]->bigRadius);

    CWeapon::PlaceParts();
}

long CSmart::Arm(CSmartPart *aPart) {
    blastPower = itsDepot->missilePower;
    shields = FIX3(100);

    partList[0]->ReplaceColor(kOtherMarkerColor, 0x400000);
    targetIdent = 0;
    targetPart = NULL;

    angleStep = itsDepot->missileTurnRate;

    return CWeapon::Arm(aPart);
}

void CSmart::DoTargeting() {
    RayHitRecord rayHit;

    VECTORCOPY(rayHit.origin, partList[0]->itsTransform[3]);
    VECTORCOPY(rayHit.direction, partList[0]->itsTransform[2]);
    NormalizeVector(3, rayHit.direction);
    rayHit.distance = FIX(120);
    rayHit.closestHit = NULL;

    RayTestWithGround(&rayHit, kSolidBit);

    sightDistance = rayHit.distance;
    inSight = -1;

    if (rayHit.closestHit) {
        CAbstractActor *targetActor;

        targetActor = rayHit.closestHit->theOwner;
        if (targetActor->maskBits & kTargetBit) {
            targetIdent = targetActor->ident;
            targetPart = rayHit.closestHit;
            inSight = targetActor->GetPlayerPosition();
        }
    } else {
        if (sightDistance > FIX(60)) {
            sightDistance = FIX(60);
        }
    }
}

void CSmart::ShowTarget() {
    CBSPPart *targ;
    Fixed dist;
    Fixed *v, *u;

    dist = sightDistance - FIX3(250);

    itsGame->itsApp->BrightBox(itsGame->frameNumber, inSight);

    if (dist > 0) {
        v = partList[0]->itsTransform[2];

        targ = itsDepot->smartSight;
        targ->CopyTransform(&partList[0]->itsTransform);
        TranslatePart(targ, FMul(v[0], dist), FMul(v[1], dist), FMul(v[2], dist));
        targ->MoveDone();
        targ->isTransparent = false;
    }

    if (targetIdent) {
        CAbstractActor *targetActor;

        targetActor = itsGame->FindIdent(targetIdent);

        if (targetActor) {
            RayHitRecord rayHit;

            v = targetPart->itsTransform[3];
            u = partList[0]->itsTransform[3];
            VECTORCOPY(rayHit.origin, u);

            rayHit.direction[0] = v[0] - u[0];
            rayHit.direction[1] = v[1] - u[1];
            rayHit.direction[2] = v[2] - u[2];

            rayHit.distance = FIX(5) + NormalizeVector(3, rayHit.direction);
            rayHit.closestHit = NULL;

            if (rayHit.distance < FIX(160)) {
                RayTestWithGround(&rayHit, kSolidBit);

                if (rayHit.closestHit && rayHit.closestHit->theOwner == targetActor) {
                    dist = rayHit.distance - FIX3(250);
                    v = rayHit.direction;

                    targ = itsDepot->smartHairs;
                    targ->CopyTransform(&partList[0]->itsTransform);
                    TranslatePart(targ, FMul(v[0], dist), FMul(v[1], dist), FMul(v[2], dist));
                    targ->MoveDone();
                    targ->isTransparent = false;
                }
            }
        }
    }
}
void CSmart::Locate() {
    Matrix fullTransform;

    if (hostPart) {
        CAbstractActor *theHost;

        theHost = hostPart->theOwner;
        theHost->GetSpeedEstimate(speed);

        location[0] = partList[0]->itsTransform[3][0];
        location[1] = partList[0]->itsTransform[3][1];
        location[2] = partList[0]->itsTransform[3][2];
    } else {
        teamColor = 0;
        teamMask = 1;
        speed[0] = 0;
        speed[1] = 0;
        speed[2] = 0;
    }
    PlaceParts();
    VECTORCOPY(location, partList[0]->itsTransform[3]);

    MATRIXCOPY(fullTransform, partList[0]->itsTransform);

    fullTransform[3][0] = 0;
    fullTransform[3][1] = 0;
    fullTransform[3][2] = 0;

    yaw = -FOneArcTan2(fullTransform[2][2], fullTransform[0][2]);
    MRotateY(FOneSin(-yaw), FOneCos(yaw), &fullTransform);
    pitch = -FOneArcTan2(fullTransform[1][1], fullTransform[2][1]);
    MRotateX(FOneSin(-pitch), FOneCos(pitch), &fullTransform);
    roll = -FOneArcTan2(fullTransform[0][0], fullTransform[1][0]);

    goodYaw = yaw;
    goodPitch = pitch;

    partList[0]->ReplaceColor(kOtherMarkerColor, kOtherMarkerColor);
}
void CSmart::Fire() {
    CBasicSound *theSound;

    Locate();

    itsSoundLink = gHub->GetSoundLink();
    UpdateSoundLink(itsSoundLink, location, speed, itsGame->soundTime);

    theSound = gHub->GetSoundSampler(hubRate, 201);
    theSound->SetRate(FIX(1));
    theSound->SetVolume(FIX(3));
    theSound->SetSoundLink(itsSoundLink);
    theSound->SetLoopCount(-1);
    theSound->Start();

    fireFrame = itsGame->frameNumber+1;
    flyCount = 1;
}

void CSmart::TurnTowardsTarget() {
    Fixed *v, *u;
    Vector toTarget;
    short delta;

    thrust = kSmartAccel;

    if (targetIdent) {
        CAbstractActor *targetActor;

        targetActor = itsGame->FindIdent(targetIdent);

        if (targetActor) {
            RayHitRecord rayHit;

            v = targetPart->itsTransform[3];
            u = partList[0]->itsTransform[3];
            VECTORCOPY(rayHit.origin, u);

            toTarget[0] = v[0] - u[0];
            toTarget[1] = v[1] - u[1];
            toTarget[2] = v[2] - u[2];
            rayHit.direction[0] = toTarget[0];
            rayHit.direction[1] = toTarget[1];
            rayHit.direction[2] = toTarget[2];

            rayHit.distance = FIX(5) + NormalizeVector(3, rayHit.direction);
            rayHit.closestHit = NULL;

            if (rayHit.distance < FIX(160)) {
                RayTestWithGround(&rayHit, kSolidBit);

                if (rayHit.closestHit && rayHit.closestHit->theOwner == targetActor) {
                    toTarget[0] -= speed[0] << 1;
                    toTarget[1] -= speed[1] << 1;
                    toTarget[2] -= speed[2] << 1;

                    FindSpaceAngle(toTarget, &goodYaw, &goodPitch);

                    if (rayHit.distance < FIX(2)) {
                        doExplode = true;
                        FPS_DEBUG("Missile TTT EXPLODE location = " << FormatVector(location, 3) << "\n");
                    }
#define MOD_THRUST
#ifdef MOD_THRUST
                    if (rayHit.distance < FIX(8)) {
                        thrust = FMul((rayHit.distance >> 3), kSmartAccel);
                    }
#endif
                }
            }
        }
    }

    delta = goodYaw - yaw;

#ifdef ZERO_THRUST
    if (delta > 0x6000 || delta < -0x6000)
        thrust >>= 3;
#endif

    if (delta < -angleStep)
        yaw -= angleStep;
    else if (delta > angleStep)
        yaw += angleStep;
    else
        yaw = goodYaw;
    yaw = (short)yaw;

    delta = goodPitch - pitch;
    if (delta < -angleStep)
        pitch -= angleStep;
    else if (delta > angleStep)
        pitch += angleStep;
    else
        pitch = goodPitch;
}

void CSmart::FrameAction() {
    CWeapon::FrameAction();

    if (flyCount) {
        Vector accel;
        Fixed pitchCos;
        Fixed speedDotAccel;
        Fixed realSpeed;
        Fixed friction;
        RayHitRecord rayHit;

        FPS_DEBUG("Missile frameNumber = " << itsGame->frameNumber << "\n");

if (IsClassicInterval()) { // indented like this because hope to remove it in the future
        TurnTowardsTarget();

        pitchCos = FOneCos(pitch);
        accel[0] = FMul(pitchCos, FOneSin(yaw));
        accel[2] = FMul(pitchCos, FOneCos(yaw));
        accel[1] = FOneSin(-pitch);

        speedDotAccel = FMul(accel[0], speed[0]) + FMul(accel[1], speed[1]) + FMul(accel[2], speed[2]);

        VECTORCOPY(rayHit.direction, speed);
        VECTORCOPY(rayHit.origin, location);
        rayHit.closestHit = NULL;

        rayHit.distance = NormalizeVector(3, rayHit.direction);

        realSpeed = rayHit.distance;

        RayTestWithGround(&rayHit, kSolidBit);

        if (rayHit.closestHit) {
            if (hostIdent) {
                CAbstractActor *oldHost;

                oldHost = itsGame->FindIdent(hostIdent);
                if (oldHost == rayHit.closestHit->theOwner) {
                    rayHit.closestHit = NULL;
                }
            }

            if (rayHit.closestHit) {
                realSpeed = rayHit.distance;
                speed[0] = FMul(rayHit.direction[0], realSpeed);
                speed[1] = FMul(rayHit.direction[1], realSpeed);
                speed[2] = FMul(rayHit.direction[2], realSpeed);
                FPS_DEBUG("Missile FA1 EXPLODE location = " << FormatVector(location, 3) << "\n");
                doExplode = true;
            }
        }

        if (realSpeed > 100) {
            speedDotAccel = FDivNZ(speedDotAccel, realSpeed);
            if (speedDotAccel < 0)
                speedDotAccel = -speedDotAccel;
        } else {
            speedDotAccel = FIX(1);
        }

        friction = kSmartFriction + ((FIX(1) - speedDotAccel) >> 3);

        speed[0] += FMul(thrust, accel[0]) - FMul(speed[0], friction);
        speed[1] += FMul(thrust, accel[1]) - FMul(speed[1], friction);
        speed[2] += FMul(thrust, accel[2]) - FMul(speed[2], friction);
        FPS_DEBUG("Missile speed = " << FormatVector(speed, 3) << "\n");
} // IsClassicInterval()

        location[0] += FpsCoefficient2(speed[0]);
        location[1] += FpsCoefficient2(speed[1]);
        location[2] += FpsCoefficient2(speed[2]);
        FPS_DEBUG("Missile location = " << FormatVector(location, 3) << "\n");

        UpdateSoundLink(itsSoundLink, location, speed, itsGame->soundTime);

        PlaceParts();

        if (hostIdent) {
            if (FpsCoefficient2(flyCount) > 5) {
                ReleaseAttachment();
            } else {
                CAbstractActor *oldHost;

                oldHost = itsGame->FindIdent(hostIdent);
                if (oldHost) {
                    oldHost->searchCount = itsGame->searchCount + 1;
                }
            }
        }

        if (FpsCoefficient2(flyCount) > 100) {
            doExplode = true;
        }

if (IsClassicInterval()) {
        BuildPartProximityList(location, partList[0]->bigRadius, kSolidBit);

        if (location[1] <= 0 || DoCollisionTest(&proximityList.p)) {
            location[0] -= FpsCoefficient2(speed[0]);
            location[1] -= FpsCoefficient2(speed[1]);
            location[2] -= FpsCoefficient2(speed[2]);
            doExplode = true;
            FPS_DEBUG("Missile FA2 EXPLODE location = " << FormatVector(location, 3) << "\n");
        }
} // IsClassicInterval()

        flyCount++;
    }

    if (doExplode) {
        itsGame->scoreReason = ksiMissileHit;
        Explode();
    }
}

void CSmart::PreLoadSounds() {
    CWeapon::PreLoadSounds();

    gHub->PreLoadSample(201);
}

bool CSmart::IsClassicInterval() {
    return (itsGame->frameNumber-fireFrame) % int(1/itsGame->fpsScale) == 0;
}
