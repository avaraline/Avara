/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CGrenade.c
    Created: Tuesday, February 13, 1996, 00:14
    Modified: Monday, August 26, 1996, 03:07
*/

#include "CGrenade.h"

#include "CDepot.h"
#include "CSmartPart.h"
//#include "CInfoPanel.h"
#include "CAvaraApp.h"

// #define DEBUGFPS 1  // will override def in CAbstractActor.h but gives compiler warnings

#define kGravity FIX3(120)
#define kGrenadeFriction FIX3(10)

void CGrenade::IWeapon(CDepot *theDepot) {
    CWeapon::IWeapon(theDepot);

    blastSound = 231;

    partCount = 1;
    LoadPart(0, 820);

    mass = FIX(1);
}

void CGrenade::PlaceParts() {
    if (flyCount == 0 && hostPart) {
        partList[0]->Reset();
        TranslatePart(partList[0], 0, FIX3(-200), FIX3(950));
        partList[0]->ApplyMatrix(&hostPart->itsTransform);
    } else {
        partList[0]->Reset();
        InitialRotatePartZ(partList[0], roll);
        partList[0]->RotateOneX(pitch);
        partList[0]->RotateOneY(yaw);
        TranslatePart(partList[0], location[0], location[1], location[2]);
    }

    partList[0]->MoveDone();
    LinkSphere(partList[0]->itsTransform[3], partList[0]->bigRadius);

    CWeapon::PlaceParts();
}

long CGrenade::Arm(CSmartPart *aPart) {
    // using classic numbers in the targeting calculation to keep it fast
    classicGravity = FMul(kGravity, itsGame->gravityRatio);
    classicFriction = FIX1 - kGrenadeFriction;

    FpsCoefficients(classicFriction, classicGravity, &friction, &gravity);

    blastPower = itsDepot->grenadePower;
    shields = FIX3(100);

    return CWeapon::Arm(aPart);
}

void CGrenade::Locate() {
    Matrix fullTransform;

    PlaceParts();

    if (hostPart) {
        CAbstractActor *theHost;

        theHost = hostPart->theOwner;
        theHost->GetSpeedEstimate(speed);
    } else {
        teamColor = 0;
        teamMask = 1;
        speed[0] = 0;
        speed[1] = 0;
        speed[2] = 0;
    }

    VECTORCOPY(location, partList[0]->itsTransform[3]);
    MATRIXCOPY(fullTransform, partList[0]->itsTransform);

    fullTransform[3][0] = 0;
    fullTransform[3][1] = 0;
    fullTransform[3][2] = 0;

    speed[0] += (fullTransform[1][0] + 2 * fullTransform[2][0]);
    speed[1] += (fullTransform[1][1] + 2 * fullTransform[2][1]);
    speed[2] += (fullTransform[1][2] + 2 * fullTransform[2][2]);

    // correct for high-FPS bias of grenades flying a little high by doing 3/4 of a
    // classic frame's worth of gravity adjustments up front
    speed[1] -= int(0.75 / itsGame->fpsScale) * gravity * 0.5;

    FPS_DEBUG("GRENADE initial location = " << FormatVector(location, 3) << "\n")
    FPS_DEBUG("GRENADE initial speed = " << FormatVector(speed, 3) << "\n")

    yaw = -FOneArcTan2(fullTransform[2][2], fullTransform[0][2]);
    MRotateY(FOneSin(-yaw), FOneCos(yaw), &fullTransform);
    pitch = -FOneArcTan2(fullTransform[1][1], fullTransform[2][1]);
    MRotateX(FOneSin(-pitch), FOneCos(pitch), &fullTransform);
    roll = -FOneArcTan2(fullTransform[0][0], fullTransform[1][0]);
}

void CGrenade::Fire() {
    CBasicSound *theSound;

    Locate();

    itsSoundLink = gHub->GetSoundLink();
    UpdateSoundLink(itsSoundLink, location, speed, itsGame->soundTime);

    theSound = gHub->GetSoundSampler(hubRate, 201);
    theSound->SetRate(FIX3(500));
    theSound->SetVolume(FIX(3));
    theSound->SetSoundLink(itsSoundLink);
    theSound->SetLoopCount(-1);
    theSound->Start();

    flyCount = 1;
}

void CGrenade::FrameAction() {
    CWeapon::FrameAction();

    if (flyCount) {
        RayHitRecord rayHit;
        Fixed realSpeed;

        speed[0] = FMul(speed[0], friction);
        speed[1] = FMul(speed[1], friction) - gravity;
        speed[2] = FMul(speed[2], friction);

        // FPS_DEBUG("\nframeNumber = " << itsGame->frameNumber << "\n")
        FPS_DEBUG("GRENADE speed = " << FormatVector(speed, 3) << "\n")

        VECTORCOPY(rayHit.direction, speed);
        VECTORCOPY(rayHit.origin, location);
        rayHit.closestHit = NULL;
        realSpeed = NormalizeVector(3, rayHit.direction);
        rayHit.distance = realSpeed;

        RayTestWithGround(&rayHit, kSolidBit);

        // if the location adjustment will put us below ground
        if (FpsCoefficient2(realSpeed) > rayHit.distance) {
            realSpeed = FpsCoefficient2(rayHit.distance);
            speed[0] = FMul(rayHit.direction[0], realSpeed);
            speed[1] = FMul(rayHit.direction[1], realSpeed);
            speed[2] = FMul(rayHit.direction[2], realSpeed);
            doExplode = true;
            FPS_DEBUG("GRENADE explode speed = " << FormatVector(speed, 3) << "\n")
        }

        location[0] += FpsCoefficient2(speed[0]);
        location[1] += FpsCoefficient2(speed[1]);
        location[2] += FpsCoefficient2(speed[2]);
        FPS_DEBUG("GRENADE location = " << FormatVector(location, 3) << "\n")

        UpdateSoundLink(itsSoundLink, location, speed, itsGame->soundTime);

        PlaceParts();

        if (hostIdent) {
            if (flyCount * itsGame->fpsScale > 5) {
                ReleaseAttachment();
            } else {
                CAbstractActor *oldHost;

                oldHost = itsGame->FindIdent(hostIdent);
                if (oldHost) {
                    oldHost->searchCount = itsGame->searchCount + 1;
                }
            }
        }

        if (flyCount * itsGame->fpsScale > 100) {
            doExplode = true;
        }

        BuildPartProximityList(location, partList[0]->bigRadius, kSolidBit);

        if (location[1] <= 0 || DoCollisionTest(&proximityList.p)) {
            location[0] -= speed[0] * itsGame->fpsScale;
            location[1] -= speed[1] * itsGame->fpsScale;
            location[2] -= speed[2] * itsGame->fpsScale;
            doExplode = true;
            FPS_DEBUG("GRENADE collision location = " << FormatVector(location, 3) << "\n")
        }

        flyCount++;
    }

    if (doExplode) {
        itsGame->scoreReason = ksiGrenadeHit;
        Explode();
    }
}

void CGrenade::ShowTarget() {
    RayHitRecord rayHit;
    Vector tLoc;
    Vector tSpeed;
    Fixed realDistance;
    Matrix *m;
    CBSPPart *sight, *sight2;
    short i;
    Fixed tYaw, tPitch;
    CAbstractActor *theHost;

    if (hostPart) {
        theHost = hostPart->theOwner;
        theHost->GetSpeedEstimate(tSpeed);

        m = &partList[0]->itsTransform;
        tLoc[0] = (*m)[3][0];
        tLoc[1] = (*m)[3][1];
        tLoc[2] = (*m)[3][2];

        tSpeed[0] += (*m)[1][0] + 2 * (*m)[2][0];
        tSpeed[1] += (*m)[1][1] + 2 * (*m)[2][1];
        tSpeed[2] += (*m)[1][2] + 2 * (*m)[2][2];

        rayHit.closestHit = NULL;

        do {
            rayHit.origin[0] = tLoc[0];
            rayHit.origin[1] = tLoc[1];
            rayHit.origin[2] = tLoc[2];

            // advance hypothetical location 5*64msec worth of classic frames
            for (i = 0; i < 5; i++) {
                tSpeed[0] = FMul(tSpeed[0], classicFriction);
                tSpeed[1] = FMul(tSpeed[1], classicFriction) - classicGravity;
                tSpeed[2] = FMul(tSpeed[2], classicFriction);

                tLoc[0] += tSpeed[0];
                tLoc[1] += tSpeed[1];
                tLoc[2] += tSpeed[2];
            }

            // draw directional vector over last 5 frames then test if it hits the ground
            rayHit.direction[0] = tLoc[0] - rayHit.origin[0];
            rayHit.direction[1] = tLoc[1] - rayHit.origin[1];
            rayHit.direction[2] = tLoc[2] - rayHit.origin[2];

            realDistance = NormalizeVector(3, rayHit.direction);
            rayHit.distance = realDistance;

            RayTestWithGround(&rayHit, kSolidBit);

        } while (rayHit.distance >= realDistance);

        if (rayHit.closestHit) {
            itsGame->itsApp->BrightBox(itsGame->frameNumber, rayHit.closestHit->theOwner->GetPlayerPosition());
        }

        realDistance = rayHit.distance;
        FindSpaceAngle(rayHit.direction, &tYaw, &tPitch);

        sight = itsDepot->grenadeSight;
        sight->isTransparent = false;
        sight->Reset();
        InitialRotatePartX(sight, tPitch);
        sight->RotateOneY(tYaw);

        rayHit.origin[0] += FMul(realDistance, rayHit.direction[0]);
        rayHit.origin[1] += FMul(realDistance, rayHit.direction[1]);
        rayHit.origin[2] += FMul(realDistance, rayHit.direction[2]);

        TranslatePart(sight, rayHit.origin[0], rayHit.origin[1], rayHit.origin[2]);
        sight->MoveDone();

        sight2 = itsDepot->grenadeTop;
        sight2->CopyTransform(&sight->itsTransform);
        sight2->isTransparent = false;
        sight2->MoveDone();
    }
}

void CGrenade::PreLoadSounds() {
    CWeapon::PreLoadSounds();

    gHub->PreLoadSample(201);
}
