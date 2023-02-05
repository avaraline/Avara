/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CBall.c
    Created: Monday, February 26, 1996, 15:41
    Modified: Saturday, September 7, 1996, 17:46
*/

#include "CBall.h"

#include "CSmartPart.h"
//#include "Palettes.h"

#define MINSPEED FIX3(20)

#define kBSPBall 250
#define kBallFloatForce FIX3(50)
#define kBallGravity FIX3(40)
#define kFriction FIX3(970)
#define kStandardBallMass FIX(30);
#define kBallBaseMass FIX(50);

void CBall::IAbstractActor() {
    CRealShooters::IAbstractActor();

    mass = kStandardBallMass;
    baseMass = kStandardBallMass;

    carryScoreAccumulator = 0;

    shieldsCharge = 0;
    maxShields = -1;
    shootShields = FIX(10000);
    dropDamage = FIX(1);

    holderScoreId = -1;
    holderTeam = 0;
    changeHolderPower = FIX3(300);
    releaseHoldAccumulator = 0;
    holdShieldLimit = shootShields;

    group = -1;

    shotPower = 0;
    burstLength = 3;
    burstSpeed = 5;
    burstCharge = 20;

    customGravity = kBallGravity;

    shapeRes = kBSPBall;
    acceleration = kFriction;

    ownerChangeCount = 0;
    ownerChangeTime = 0;

    carryScore = 0;
    goalScore = 0;

    ejectPitch = FIX(20);
    ejectPower = FIX(1);

    buzzSound = 430;
    buzzVolume = FIX3(500);

    snapSound = 431;
    snapVolume = FIX(1);

    ejectSound = 432;
    ejectVolume = FIX(1);

    reprogramSound = 0;
    reprogramVolume = FIX(1);
}

void CBall::BeginScript() {
    CRealShooters::BeginScript();

    ProgramLongVar(iShape, shapeRes);
    ProgramLongVar(iGoalMsg, 0);
    ProgramLongVar(iOutVar, 0);
    ProgramLongVar(iCarryScore, carryScore);
    ProgramLongVar(iGoalScore, goalScore);

    ProgramMessage(iStartMsg, iStartMsg);
    ProgramLongVar(iStopMsg, 0);
    ProgramLongVar(iStatus, false);

    ProgramFixedVar(iShieldChargeRate, shieldsCharge);
    ProgramFixedVar(iMaxShield, maxShields);
    ProgramFixedVar(iShootShield, shootShields);
    ProgramFixedVar(iGrabShield, holdShieldLimit);

    ProgramFixedVar(iDropEnergy, dropDamage);

    ProgramFixedVar(iCustomGravity, customGravity);
    ProgramFixedVar(iAccelerate, acceleration);

    ProgramLongVar(iGroup, group);

    ProgramFixedVar(iChangeHolderPower, changeHolderPower);
    ProgramLongVar(iChangeOwnerTime, ownerChangeTime);

    ProgramFixedVar(iEjectPitch, ejectPitch);
    ProgramFixedVar(iEjectPower, ejectPower);

    ProgramLongVar(iEjectSound, ejectSound);
    ProgramFixedVar(iEjectVolume, ejectVolume);

    ProgramLongVar(iSound, buzzSound);
    ProgramFixedVar(iVolume, buzzVolume);

    ProgramLongVar(iChangeOwnerSound, reprogramSound);
    ProgramFixedVar(iChangeOwnerVolume, reprogramVolume);

    ProgramLongVar(iSnapSound, snapSound);
    ProgramFixedVar(iSnapVolume, snapVolume);
}

CAbstractActor *CBall::EndScript() {
    if (CRealShooters::EndScript()) {
        customGravity = ReadFixedVar(iCustomGravity);
        acceleration = ReadFixedVar(iAccelerate);

        ejectPitch = ReadFixedVar(iEjectPitch);
        ejectPower = ReadFixedVar(iEjectPower);

        carryScore = ReadFixedVar(iCarryScore);
        goalScore = ReadLongVar(iGoalScore);

        shieldsCharge = ReadFixedVar(iShieldChargeRate);
        maxShields = ReadFixedVar(iMaxShield);
        shootShields = ReadFixedVar(iShootShield);
        holdShieldLimit = ReadFixedVar(iGrabShield);

        dropDamage = ReadFixedVar(iDropEnergy);

        goalMsg = ReadLongVar(iGoalMsg);
        captureMsg = ReadLongVar(iOutVar);
        RegisterReceiver(&startMsg, ReadLongVar(iStartMsg));
        RegisterReceiver(&stopMsg, ReadLongVar(iStopMsg));
        status = ReadLongVar(iStatus);

        changeHolderPower = ReadFixedVar(iChangeHolderPower);
        ownerChangeTime = ReadLongVar(iChangeOwnerTime);

        group = ReadLongVar(iGroup);

        buzzSound = ReadLongVar(iSound);
        buzzVolume = ReadFixedVar(iVolume);
        gHub->PreLoadSample(buzzSound);

        ejectSound = ReadLongVar(iEjectSound);
        ejectVolume = ReadFixedVar(iEjectVolume);
        gHub->PreLoadSample(ejectSound);

        reprogramSound = ReadLongVar(iChangeOwnerSound);
        reprogramVolume = ReadFixedVar(iChangeOwnerVolume);
        gHub->PreLoadSample(reprogramSound);

        snapSound = ReadLongVar(iSnapSound);
        snapVolume = ReadFixedVar(iSnapVolume);
        gHub->PreLoadSample(snapSound);

        pitchZ = FMul(ejectPower, FDegCos(ejectPitch));
        pitchY = FMul(ejectPower, FDegSin(ejectPitch));

        buzzLink = NULL;
        itsSoundLink = gHub->GetSoundLink();
        PlaceSoundLink(itsSoundLink, location);

        hostPart = NULL;
        oldHost = 0;
        hostIdent = 0;
        looseFrame = 0;

        origLongColor = GetPixelColor();
        origWatchTeams = watchTeams;
        origTeam = teamColor;

        partCount = 1;
        LoadPartWithColors(0, ReadLongVar(iShape));

        PlaceParts();
        VECTORCOPY(origLocation, location);

        maskBits |= kSolidBit + kTargetBit;
        isActive |= kIsActive;

        actionCommand = 0;

        ChangeOwnership(-1, origTeam);

        return this;

    } else {
        return NULL;
    }
}
void CBall::Dispose() {
    BuzzControl(false);
    ReleaseAttachment();

    CRealShooters::Dispose();
}

void CBall::ChangeOwnership(short ownerId, short ownerTeamColor) {
    teamColor = ownerTeamColor;
    teamMask = 1 << teamColor;
    ownerScoringId = ownerId;

    uint32_t longTeamColor = GetTeamColorOr(origLongColor);

    for (CSmartPart **thePart = partList; *thePart; thePart++) {
        (*thePart)->ReplaceColor(*ColorManager::getMarkerColor(0), longTeamColor);
    }
}

void CBall::PlaceParts() {
    CSmartPart *theBall;

    theBall = partList[0];
    theBall->Reset();
    if (hostPart) {
        TranslatePart(theBall, localSnap[0], localSnap[1], localSnap[2]);
        theBall->ApplyMatrix(&hostPart->itsTransform);
    } else {
        TranslatePart(theBall, location[0], location[1], location[2]);
    }

    theBall->MoveDone();

    LinkSphere(theBall->itsTransform[3], theBall->bigRadius);

    CRealShooters::PlaceParts();
}

void CBall::ReleaseAttachment() {
    CAbstractActor *host;

    releaseHoldAccumulator = 0;
    ownerChangeCount = 0;

    if (hostPart) {
        host = itsGame->FindIdent(hostIdent);
        if (host) {
            host->GetSpeedEstimate(speed);
            host->Detach(&clamp);
            hostIdent = 0;
            hostPart = NULL;
            location[0] = partList[0]->itsTransform[3][0];
            location[1] = partList[0]->itsTransform[3][1];
            location[2] = partList[0]->itsTransform[3][2];
        }
    }
}

void CBall::BuzzControl(Boolean doBuzz) {
    if (itsSoundLink)
        UpdateSoundLink(itsSoundLink, location, speed, itsGame->soundTime);

    if (buzzSound) {
        if (buzzLink) {
            if (!doBuzz) {
                buzzLink->meta = metaFade;
                gHub->ReleaseLink(buzzLink);
                buzzLink = NULL;
            }
        } else {
            if (doBuzz) {
                CBasicSound *theSound;

                buzzLink = gHub->GetSoundLink();
                theSound = gHub->GetSoundSampler(hubRate, buzzSound);
                theSound->SetSoundLink(itsSoundLink);
                theSound->SetControlLink(buzzLink);
                theSound->SetVolume(buzzVolume);
                theSound->Start();
            }
        }
    }
}

void CBall::MagnetAction() {
    CAbstractActor *theActor;
    FrameNumber thisFrame = itsGame->frameNumber;
    Boolean didAttract = false;

    BuildActorProximityList(location, partList[0]->bigRadius, kBallSnapBit);

    for (theActor = proximityList.a; theActor; theActor = theActor->proximityList.a) {
        short snapCode;
        Vector snapTo;
        CSmartPart *newHost;

        if ((theActor->ident != oldHost || looseFrame < thisFrame) &&
            !(theActor->maskBits & kPlayerBit && holdShieldLimit < shields && theActor->teamColor != teamColor)) {
            snapCode = theActor->GetBallSnapPoint(group, location, snapTo, localSnap, &newHost);

            if (snapCode == kSnapHere) {
                Matrix savedMatrix;
                CAbstractActor *hostActor;

                MATRIXCOPY(savedMatrix, partList[0]->itsTransform);
                hostPart = newHost;
                hostActor = newHost->theOwner;
                PlaceParts();

                hostActor->searchCount = itsGame->searchCount + 1;
                BuildPartProximityList(snapTo, partList[0]->bigRadius, kSolidBit);
                if (DoCollisionTest(&proximityList.p)) {
                    hostPart = NULL;
                    snapCode = kSnapAttract;
                    MATRIXCOPY(partList[0]->itsTransform, savedMatrix);
                    partList[0]->MoveDone();

                    BuildActorProximityList(location, partList[0]->bigRadius, kBallSnapBit);
                } else {
                    playerAttach = (hostActor->maskBits & kPlayerBit) != 0;
                    clamp.me = this;
                    hostIdent = hostActor->Attach(&clamp);
                    hostActor->ReceiveSignal(kBallAttached, this);
                    holderScoreId = hostActor->GetActorScoringId();
                    holderTeam = hostActor->teamColor;
                    releaseHoldAccumulator = 0;
                    carryScoreAccumulator = 0;

                    DoSound(snapSound, snapTo, snapVolume, FIX(1));
                    break;
                }
            }

            if (snapCode == kSnapAttract) {
                Vector delta;
                Fixed len;

                delta[0] = snapTo[0] - location[0];
                delta[1] = snapTo[1] - location[1];
                delta[2] = snapTo[2] - location[2];
                len = NormalizeVector(3, delta);
                speed[0] += (delta[0] - speed[0]) >> 2;
                speed[1] += (delta[1] - speed[1]) >> 2;
                speed[2] += (delta[2] - speed[2]) >> 2;

                didAttract = true;
            }
        }
    }

    BuzzControl(didAttract);
}
void CBall::FrameAction() {
    Vector oldLocation;
    CSmartPart *hitPart;

    CRealShooters::FrameAction();

    if (isActive & kHasMessage) {
        if (stopMsg.triggerCount > startMsg.triggerCount) {
            status = false;
        } else if (stopMsg.triggerCount < startMsg.triggerCount) {
            status = true;
        }

        stopMsg.triggerCount = 0;
        startMsg.triggerCount = 0;

        isActive &= ~kHasMessage;
        if (status)
            isActive |= kIsActive;
        else
            isActive &= ~kIsActive;
    }

    if (hostPart) {
        CAbstractActor *theOwner;

        carryScoreAccumulator += carryScore;
        if (carryScoreAccumulator > FIX(1)) {
            theOwner = itsGame->FindIdent(hostIdent);
            if (theOwner && theOwner->maskBits & kPlayerBit) {
                itsGame->scoreReason = ksiHoldBall;
                itsGame->Score(
                    theOwner->teamColor, theOwner->GetActorScoringId(), carryScoreAccumulator >> 16, 0, 0, -1);
            }
            carryScoreAccumulator &= 0xFFFF;
        }

        ownerChangeCount++;
        if (ownerChangeCount > ownerChangeTime) {
            theOwner = itsGame->FindIdent(hostIdent);
            if (theOwner && (theOwner->maskBits & kPlayerBit)) {
                itsGame->FlagMessage(captureMsg);
                ChangeOwnership(theOwner->GetActorScoringId(), theOwner->teamColor);
                watchTeams = ~(1 << teamColor);

                DoSound(reprogramSound, partList[0]->sphereGlobCenter, reprogramVolume, FIX(1));
            }

            ownerChangeCount = -32767;
        }
    }

    shields += shieldsCharge;
    if (shields > maxShields) {
        shields = maxShields;
    }

    releaseHoldAccumulator -= releaseHoldAccumulator >> 7;

    if (status && hostPart == NULL) {
        MagnetAction();

        if (location[1] > partList[0]->minBounds.y) {
            speed[1] -= FMul(customGravity, itsGame->gravityRatio);
        }

        speed[0] = FMul(speed[0], acceleration);
        speed[1] = FMul(speed[1], acceleration);
        speed[2] = FMul(speed[2], acceleration);

        if (speed[1] < 0 && speed[1] + location[1] < -partList[0]->minBounds.y) {
            speed[1] = -partList[0]->minBounds.y - location[1];
        }

        if (speed[0] > MINSPEED || speed[0] < -MINSPEED || speed[1] > MINSPEED || speed[1] < -MINSPEED ||
            speed[2] > MINSPEED || speed[2] < -MINSPEED) {
            VECTORCOPY(oldLocation, location);
            location[0] += speed[0];
            location[1] += speed[1];
            location[2] += speed[2];
            OffsetParts(speed);
            BuildPartProximityList(
                location, partList[0]->bigRadius + FDistanceOverEstimate(speed[0], speed[1], speed[2]), kSolidBit);

            hitPart = DoCollisionTest(&proximityList.p);
            if (hitPart) {
                Vector negSpeed;

                negSpeed[0] = -speed[0];
                negSpeed[1] = -speed[1];
                negSpeed[2] = -speed[2];
                VECTORCOPY(location, oldLocation);
                OffsetParts(negSpeed);
                FindBestMovement(hitPart);
            }

            LinkSphere(location, partList[0]->bigRadius);
        }

        if (shields >= shootShields && shotPower && itsGame->frameNumber >= nextShotFrame) {
            Boolean didShoot;

            didShoot = Shoot();
            if (didShoot) {
                if (burstCount == burstLength) {
                    burstStartFrame = itsGame->FramesFromNow(burstCharge);
                }

                if (--burstCount == 0) {
                    burstCount = burstLength;
                    nextShotFrame = burstStartFrame;
                } else {
                    nextShotFrame = itsGame->FramesFromNow(burstSpeed);
                }
            }
        }
    }

    if (actionCommand) {
        switch (actionCommand) {
            case kDoSelfDestruct:
                WasDestroyed();
                SecondaryDamage(teamColor, -1);
                return; //	*** return after dispose! ***
            case kDoRelease:
                looseFrame = itsGame->FramesFromNow(100);
                oldHost = hostIdent;
            case kDoReset: {
                CSmartPart *savedHost;

                ChangeOwnership(-1, origTeam);
                watchTeams = origWatchTeams;

                savedHost = hostPart;
                hostPart = NULL;
                VECTORCOPY(location, origLocation);
                PlaceParts();

                BuildPartProximityList(location, partList[0]->bigRadius, kSolidBit);
                hostPart = savedHost;
                if (DoCollisionTest(&proximityList.p) == NULL) {
                    speed[0] = 0;
                    speed[1] = 0;
                    speed[2] = 0;
                    hostPart = savedHost;
                    ReleaseAttachment();
                    actionCommand = 0;
                }
            } break;
        }
    }
}

long CBall::ReceiveSignal(long theSignal, void *miscData) {
    switch (theSignal) {
        case kDidGoalSignal:
            itsGame->FlagMessage(goalMsg);
            break;

        case kBallReleaseSignal: {
            CSmartPart *theBall = partList[0];

            looseFrame = itsGame->FramesFromNow(32);
            oldHost = hostIdent;
            ReleaseAttachment();
            speed[0] += FMul(pitchZ, theBall->itsTransform[2][0]) + FMul(pitchY, theBall->itsTransform[1][0]);
            speed[1] += FMul(pitchZ, theBall->itsTransform[2][1]) + FMul(pitchY, theBall->itsTransform[1][1]);
            speed[2] += FMul(pitchZ, theBall->itsTransform[2][2]) + FMul(pitchY, theBall->itsTransform[1][2]);

            DoSound(ejectSound, partList[0]->sphereGlobCenter, ejectVolume, FIX(1));
            return true;
        }

        case kDoSelfDestruct:
        case kDoReset:
        case kDoRelease:
            actionCommand = theSignal;
            break;
        case kHostDamage:
            ReleaseDamage(*((Fixed *)miscData));
            break;

        default:
            return CRealShooters::ReceiveSignal(theSignal, miscData);
    }

    return 0;
}

void CBall::ReleaseDamage(Fixed hitEnergy) {
    if (hostPart && playerAttach) {
        releaseHoldAccumulator += hitEnergy;
        if (releaseHoldAccumulator > dropDamage) {
            looseFrame = itsGame->FramesFromNow(10);
            oldHost = hostIdent;
            ReleaseAttachment();
        }
    }
}

void CBall::WasHit(RayHitRecord *theHit, Fixed hitEnergy) {
    ReleaseDamage(hitEnergy);

    if (hostPart == NULL && hitEnergy >= changeHolderPower) {
        holderScoreId = theHit->playerId;
        holderTeam = theHit->team;
    }

    CRealShooters::WasHit(theHit, hitEnergy);
}

short CBall::GetShooterId() {
    return ownerScoringId;
}
