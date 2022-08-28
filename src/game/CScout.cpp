/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CScout.c
    Created: Wednesday, March 15, 1995, 07:48
    Modified: Monday, September 16, 1996, 19:31
*/
// #define ENABLE_FPS_DEBUG  // uncomment if you want to see FPS_DEBUG output for this file

#include "CScout.h"

#include "CAbstractPlayer.h"
#include "CSmartPart.h"
#include "CViewParameters.h"
//#include "Mat2D.h"

#define kScoutBSP 220
#define TURNSPEED FIX(0.3944)
#define SCOUTSHIELD FIX(10)

#define SCOUTSPEED FIX3(100)
#define SCOUTHEIGHT FIX(5)
#define SCOUTUPDOWN FIX3(100)
#define SCOUTCLOSERADIUS FIX(5)
#define SCOUTVERYCLOSE FIX(3)

#define FOLLOWRADIUS 10

void CScout::IScout(CAbstractPlayer *thePlayer, short theTeam, uint32_t longTeamColor) {
    IAbstractActor();

    mass = FIX(20);
    baseMass = FIX(40);
    teamColor = theTeam;
    teamMask = 1 << teamColor;
    itsPlayer = thePlayer;
    itsGame = thePlayer->itsGame;

    isActive = kIsActive;
    maskBits |= kTargetBit + kScoutBit + kSolidBit + kCollisionDamageBit;
    action = kScoutInactive;
    shields = SCOUTSHIELD;
    nextRotateFlag = true;

    partCount = 1;
    LoadPart(0, kScoutBSP);
    partList[0]->ReplaceColor(kMarkerColor, longTeamColor);

    hitSoundId = 220;
    gHub->PreLoadSample(hitSoundId);

    glow = 0;
}

void CScout::PlaceParts() {
    partList[0]->Reset();
    InitialRotatePartY(partList[0], heading);
    TranslatePart(partList[0], location[0], location[1], location[2]);
    partList[0]->MoveDone();

    CRealMovers::PlaceParts();
}

#define kScoutFloatFriction 0.96
#define kScoutFriction FIX(kScoutFloatFriction)
#define kScoutCourseCorrector 5
#define kScoutAcceleration FIX3(50)

Fixed CScout::MoveToTarget() {
    Vector delta;
    Vector course;
    short alpha;
    Fixed dist;

    FPS_DEBUG("CScout::MoveToTarget frameNumber = " << itsGame->frameNumber <<
              ", speed = " << FDistanceEstimate(speed[0], speed[1], speed[2]) << "\n");

    delta[0] = targetPosition[0] - location[0];
    delta[1] = targetPosition[1] - location[1];
    delta[2] = targetPosition[2] - location[2];

    dist = FDistanceEstimate(delta[0], delta[1], delta[2]);

    if (dist < FpsCoefficient2(FIX3(100))) {
        course[0] = -speed[0] >> 1;
        course[1] = -speed[1] >> 1;
        course[2] = -speed[2] >> 1;
    } else {
        Fixed speedEstimate;
        Fixed framesEstimate;
        Fixed revisedDistance;

        delta[0] -= speed[0] * kScoutCourseCorrector;
        delta[1] -= speed[1] * kScoutCourseCorrector;
        delta[2] -= speed[2] * kScoutCourseCorrector;

        if (delta[0] || delta[1] || delta[2])
            NormalizeVector(3, delta);

        course[0] = FMul(delta[0], kScoutAcceleration);
        course[1] = FMul(delta[1], kScoutAcceleration);
        course[2] = FMul(delta[2], kScoutAcceleration);
    }

    Fixed fpsFriction, fpsCoeff2;
    // both terms are multiplied by kScoutFriction
    FpsCoefficients(kScoutFriction, kScoutFriction, &fpsFriction, &fpsCoeff2);
    speed[0] = FMul(speed[0], fpsFriction) + FMul(course[0], fpsCoeff2);
    speed[1] = FMul(speed[1], fpsFriction) + FMul(course[1], fpsCoeff2);
    speed[2] = FMul(speed[2], fpsFriction) + FMul(course[2], fpsCoeff2);

    location[0] += FpsCoefficient2(speed[0]);
    location[1] += FpsCoefficient2(speed[1]);
    location[2] += FpsCoefficient2(speed[2]);

    FPS_DEBUG("CScout::MoveToTarget location = " << FormatVector(location, 3) <<
                                    ", speed = " << FormatVector(speed, 3) << "\n");

    return dist;
}

void CScout::FrameAction() {
    Vector oldLocation;
    Fixed delta[2];
    Fixed distance;
    Fixed oldHeading;
    Fixed baseHeight = itsPlayer->scoutBaseHeight + itsPlayer->location[1];

    CRealMovers::FrameAction();

    oldHeading = heading;
    VECTORCOPY(oldLocation, location);

    if (nextRotateFlag)
        heading += FpsCoefficient2(TURNSPEED);
    FPS_DEBUG("CScout::FrameAction heading = " << heading << "\n");

    switch (action) {
        case kScoutFollow:
            VECTORCOPY(targetPosition, itsPlayer->location);
            targetPosition[1] += SCOUTHEIGHT;
            targetPosition[0] -= FOneSin(itsPlayer->heading) * FOLLOWRADIUS;
            targetPosition[2] -= FOneCos(itsPlayer->heading) * FOLLOWRADIUS;
            MoveToTarget();
            break;
        case kScoutLead:
            VECTORCOPY(targetPosition, itsPlayer->location);
            targetPosition[1] += SCOUTHEIGHT;
            targetPosition[0] += FOneSin(itsPlayer->heading) * (FOLLOWRADIUS + 5);
            targetPosition[2] += FOneCos(itsPlayer->heading) * (FOLLOWRADIUS + 5);
            MoveToTarget();
            break;
        case kScoutLeft:
            VECTORCOPY(targetPosition, itsPlayer->location);
            targetPosition[1] += SCOUTHEIGHT;
            targetPosition[0] += FOneCos(itsPlayer->heading) * FOLLOWRADIUS;
            targetPosition[2] -= FOneSin(itsPlayer->heading) * FOLLOWRADIUS;
            MoveToTarget();
            break;
        case kScoutRight:
            VECTORCOPY(targetPosition, itsPlayer->location);
            targetPosition[1] += SCOUTHEIGHT;
            targetPosition[0] -= FOneCos(itsPlayer->heading) * FOLLOWRADIUS;
            targetPosition[2] += FOneSin(itsPlayer->heading) * FOLLOWRADIUS;
            MoveToTarget();
            break;

        case kScoutDown: {
            Fixed dist;

            VECTORCOPY(targetPosition, itsPlayer->location);

            dist = DistanceEstimate(location[0], location[2], targetPosition[0], targetPosition[2]);
            if (dist < SCOUTCLOSERADIUS) {
                if (dist < SCOUTVERYCLOSE && baseHeight > location[1]) {
                    targetPosition[0] = location[0];
                    targetPosition[1] = location[1];
                    targetPosition[2] = location[2];
                    distance = location[1] - baseHeight + dist;
                    MoveToTarget();
                } else {
                    targetPosition[1] = baseHeight;
                    distance = MoveToTarget();
                }
            } else {
                targetPosition[1] += SCOUTHEIGHT;
                distance = MoveToTarget();
            }

            if (location[1] <= baseHeight + FIX3(200) && distance <= SCOUTSPEED) {
                action = kScoutInactive;
                itsGame->RemoveActor(this);
                itsPlayer->scoutView = false;
                itsPlayer->scoutIdent = 0;
            }
        } break;
        case kScoutUp:
            MoveToTarget();
            break;
    }

    if (action != kScoutInactive) {
        CSmartPart *bump;

        UnlinkLocation();
        PlaceParts();
        BuildPartProximityList(location, 2 * partList[0]->enclosureRadius, kSolidBit);

        if ((bump = DoCollisionTest(&proximityList.p))) {
            nextRotateFlag = false;
            VECTORCOPY(location, oldLocation);
            heading = oldHeading;

            PlaceParts();
            FindBestMovement(bump);
        } else
            nextRotateFlag = true;

        LinkPartSpheres();
    }
}

void CScout::ToggleState(short command) {
    if (action == kScoutInactive) {
        if (command != kScoutDown) {
            itsPlayer->GetSpeedEstimate(speed);
            VECTORCOPY(location, itsPlayer->location);
            VECTORCOPY(targetPosition, itsPlayer->location);
            targetPosition[1] += SCOUTHEIGHT + itsPlayer->scoutBaseHeight;

            location[1] += itsPlayer->scoutBaseHeight;
            heading = 0;

            BuildPartProximityList(location, 2 * partList[0]->enclosureRadius, kSolidBit);
            PlaceParts();

            if (!DoCollisionTest(&proximityList.p)) {
                itsGame->AddActor(this);
                itsPlayer->scoutIdent = ident;
                action = command;
                FrameAction();
            }
        }
    } else {
        action = command;
    }
}

void CScout::ControlViewPoint() {
    CViewParameters *theView;

    theView = itsGame->itsView;
    theView->LookFrom(location[0] + FIX3(100), location[1] + FIX3(100), location[2]);
    theView->LookAtPart(itsPlayer->viewPortPart);
    theView->PointCamera();
}
