/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CDoorActor.c
    Created: Sunday, December 4, 1994, 10:01
    Modified: Saturday, September 14, 1996, 00:21
*/
// #define ENABLE_FPS_DEBUG  // uncomment if you want to see FPS_DEBUG output for this file

#include "AssetManager.h"
#include "CDoorActor.h"

#include "CSmartPart.h"

void CDoorActor::ProgramDoorVariables() {
    ProgramFixedVar(iDeltaX, 0);
    ProgramFixedVar(iDeltaY, FIX3(2600));
    ProgramFixedVar(iDeltaZ, 0);

    ProgramFixedVar(iPitch, 0);
    ProgramFixedVar(iYaw, 0);
    ProgramFixedVar(iRoll, 0);
}

void CDoorActor::ReadDoorVariables() {
    deltas[0] = ReadFixedVar(iDeltaX);
    deltas[1] = ReadFixedVar(iDeltaY);
    deltas[2] = ReadFixedVar(iDeltaZ);

    twists[0] = ReadFixedVar(iPitch);
    twists[1] = ReadFixedVar(iYaw);
    twists[2] = ReadFixedVar(iRoll);
}

void CDoorActor::BeginScript() {
    maskBits |= kSolidBit + kDoorIgnoreBit;
    canGlow = false;
    collisionGuardTime = 5;

    CGlowActors::BeginScript();

    ProgramLongVar(iOpenMsg, 0);
    ProgramLongVar(iCloseMsg, 0);

    ProgramLongVar(iDidOpen, 0);
    ProgramLongVar(iDidClose, 0);

    ProgramLongVar(iOpenDelay, 0);
    ProgramLongVar(iCloseDelay, 0);
    ProgramLongVar(iGuardDelay, collisionGuardTime);
    ProgramFixedVar(iStatus, kDoorClosed);
    ProgramLongVar(iShape, kDefaultDoorBSP);

    ProgramFixedVar(iOpenSpeed, kDoorSpeed * 100);
    ProgramFixedVar(iCloseSpeed, kDoorSpeed * 100);

    ProgramLongVar(iOpenSound, 400);
    ProgramLongVar(iCloseSound, 400);
    ProgramLongVar(iStopSound, 401);
    ProgramFixedVar(iVolume, FIX(15));

    ProgramLongVar(iShotPower, 0);

    ProgramDoorVariables();
}

CSmartPart *CDoorActor::CollisionTest() {
    CAbstractActor *theActor;
    CSmartPart *thePart;
    //CSmartPart **thePartList;

    theActor = itsGame->actorList;

    for (thePart = proximityList.p; thePart; thePart = (CSmartPart *)thePart->nextTemp) {
        theActor = thePart->theOwner;
        if (!(theActor->maskBits & kDoorIgnoreBit) && partList[0]->CollisionTest(thePart)) {
            return thePart;
        }
    }

    return NULL;
}

void CDoorActor::TouchDamage() {
    CAbstractActor *anActor;//, *next;
    CSmartPart *thePart;

    if (hitPower) {
        BlastHitRecord theBlast;

        searchCount = ++itsGame->searchCount;

        theBlast.blastPower = hitPower;
        VECTORCOPY(theBlast.blastPoint, partList[0]->itsTransform[3]);
        theBlast.team = teamColor;
        theBlast.playerId = -1;

        for (thePart = proximityList.p; thePart; thePart = (CSmartPart *)thePart->nextTemp) {
            anActor = (CAbstractActor *)thePart->theOwner;
            if (anActor && anActor->searchCount != searchCount && !(anActor->maskBits & kDoorIgnoreBit) &&
                partList[0]->CollisionTest(thePart)) {
                itsGame->scoreReason = ksiObjectCollision;
                anActor->BlastHit(&theBlast);
                anActor->searchCount = searchCount;
            }
        }

        SecondaryDamage(teamColor, -1, ksiObjectCollision);
    }
}

void CDoorActor::PlaceParts() {
    Fixed r;
    Fixed t[3];
    Fixed stat = doorStatus;
    CSmartPart *theDoor;

    theDoor = partList[0];
    theDoor->Reset();

    if (stat) {
        r = FMul(twists[0], stat);
        if (r)
            theDoor->RotateX(r);
        r = FMul(twists[1], stat);
        if (r)
            theDoor->RotateY(r);
        r = FMul(twists[2], stat);
        if (r)
            theDoor->RotateZ(r);

        t[0] = FMul(deltas[0], stat);
        t[1] = FMul(deltas[1], stat);
        t[2] = FMul(deltas[2], stat);
        if (t[0] || t[1] || t[2])
            TranslatePart(theDoor, t[0], t[1], t[2]);
    }

    theDoor->RotateOneY(heading);
    TranslatePart(theDoor, location[0], location[1], location[2]);
    theDoor->MoveDone();

    CGlowActors::PlaceParts();
}

CAbstractActor *CDoorActor::EndScript() {
    short resId;

    CGlowActors::EndScript();

    resId = ReadLongVar(iShape);

    auto bsp = AssetManager::GetBsp(resId);
    if (bsp) {
        //CBSPWorld *theWorld;

        RegisterReceiver(&openActivator, ReadLongVar(iOpenMsg));
        RegisterReceiver(&closeActivator, ReadLongVar(iCloseMsg));

        didOpen = ReadLongVar(iDidOpen);
        didClose = ReadLongVar(iDidClose);

        doorSoundVolume = ReadFixedVar(iVolume);
        openSoundId = ReadLongVar(iOpenSound);
        closeSoundId = ReadLongVar(iCloseSound);
        stopSoundId = ReadLongVar(iStopSound);

        // Preload sounds.
        auto _ = AssetManager::GetOgg(openSoundId);
        _ = AssetManager::GetOgg(closeSoundId);
        _ = AssetManager::GetOgg(stopSoundId);

        openCounter = 0;
        closeCounter = 0;

        doorStatus = ReadFixedVar(iStatus);

        //		location[0] = (location[0] + FIX3(125)) & ~(FIX3(250)-1);
        //		location[2] = (location[2] + FIX3(125)) & ~(FIX3(250)-1);

        hitPower = ReadFixedVar(iShotPower);
        
        classicOpenDelay = FrameNumber(ReadLongVar(iOpenDelay));
        classicCloseDelay = FrameNumber(ReadLongVar(iCloseDelay));
        classicGuardDelay = FrameNumber(ReadLongVar(iGuardDelay));
        classicOpenSpeed = ReadFixedVar(iOpenSpeed);
        classicCloseSpeed = ReadFixedVar(iCloseSpeed);

        ReadDoorVariables();

        LoadPartWithColors(0, resId);

        PlaceParts();
        LinkPartSpheres();

        lastMovement[0] = 0;
        lastMovement[1] = 0;
        lastMovement[2] = 0;

        partCount = 1;
        action = kDoorStopped;
        isActive = kIsActive;
    }

    return this;
}

void CDoorActor::AdaptableSettings() {
    openDelay = FpsFramesPerClassic(classicOpenDelay);
    closeDelay = FpsFramesPerClassic(classicCloseDelay);
    collisionGuardTime = FpsFramesPerClassic(classicGuardDelay);

    FPS_DEBUG("openDelay = " << openDelay << ", closeDelay = " << closeDelay << ", collisionGuardTime = " << collisionGuardTime << "\n");

    openSpeed =  classicOpenSpeed / 100;
    if (openSpeed <= 0 || openSpeed > kDoorOpen)
        openSpeed = kDoorSpeed;

    closeSpeed = classicCloseSpeed / 100;
    if (closeSpeed <= 0 || closeSpeed > kDoorOpen)
        closeSpeed = kDoorSpeed;

    openSpeed = FpsCoefficient2(openSpeed);
    closeSpeed = FpsCoefficient2(closeSpeed);
    FPS_DEBUG("openSpeed = " << openSpeed << ", closeSpeed = " << closeSpeed << "\n");
}

CDoorActor::~CDoorActor() {
    itsGame->RemoveReceiver(&openActivator);
    itsGame->RemoveReceiver(&closeActivator);
}

void CDoorActor::DoorSound() {
    short theId = 0;
    long soundLoop = -1;
    CBasicSound *theSound;

    if (itsSoundLink)
        gHub->ReleaseLinkAndKillSounds(itsSoundLink);

    switch (action) {
        case kDoorOpening:
            theId = openSoundId;
            break;
        case kDoorClosing:
            theId = closeSoundId;
            break;
        case kDoorStopped:
            theId = stopSoundId;
            soundLoop = 0;
            break;
    }

    if (theId) {
        itsSoundLink = gHub->GetSoundLink();
        PlaceSoundLink(itsSoundLink, partList[0]->itsTransform[3]);
        theSound = gHub->GetSoundSampler(hubRate, theId);
        theSound->SetVolume(doorSoundVolume);
        theSound->SetLoopCount(soundLoop);
        theSound->SetSoundLink(itsSoundLink);
        theSound->Start();
    } else {
        itsSoundLink = NULL;
    }
}

void CDoorActor::FrameAction() {
    Fixed oldDoorStatus;
    CGlowActors::FrameAction();

    if (isActive & kHasMessage) {
        if (openActivator.triggerCount) {
            openActivator.triggerCount = 0;
            openCounter = openDelay + 1;
            //			isActive |= isActive;
        }

        if (closeActivator.triggerCount) {
            closeActivator.triggerCount = 0;
            closeCounter = closeDelay + 1;
            //			isActive |= isActive;
        }
    }

    if (closeCounter > 0) {
        closeCounter--;
        if (closeCounter == 0 && doorStatus != kDoorClosed && action != kDoorClosing) {
            action = kDoorClosing;
            DoorSound();
        }
    }
    if (openCounter > 0) {
        openCounter--;
        if (openCounter == 0 && doorStatus != kDoorOpen && action != kDoorOpening) {
            action = kDoorOpening;
            DoorSound();
        }
    }

    if (action != kDoorStopped) {
        FPS_DEBUG("\n frameNumber = " << itsGame->frameNumber << "\n");
        Vector oldOrigin;

        oldOrigin[0] = partList[0]->itsTransform[3][0];
        oldOrigin[1] = partList[0]->itsTransform[3][1];
        oldOrigin[2] = partList[0]->itsTransform[3][2];

        FPS_DEBUG("oldOrigin = " << FormatVector(oldOrigin, 3) << "\n");

        oldDoorStatus = doorStatus;

        if (action == kDoorOpening) {
            doorStatus += openSpeed;
            if (doorStatus > kDoorOpen) {
                doorStatus = kDoorOpen;
            }
        } else {
            doorStatus -= closeSpeed;
            if (doorStatus < kDoorClosed)
                doorStatus = kDoorClosed;
        }

        UnlinkLocation();
        PlaceParts();

        BuildPartProximityList(partList[0]->sphereGlobCenter, partList[0]->enclosureRadius, kSolidBit);
        if (CollisionTest()) {
            TouchDamage();

            doorStatus = oldDoorStatus;
            if (collisionGuardTime > 0) {
                if (action == kDoorClosing) {
                    action = kDoorOpening;
                    closeCounter = collisionGuardTime;
                } else {
                    action = kDoorClosing;
                    openCounter = collisionGuardTime;
                }
            }
            PlaceParts();
        }
        LinkPartSpheres();

        if (doorStatus == kDoorClosed || doorStatus == kDoorOpen) {
            itsGame->FlagMessage(doorStatus == kDoorOpen ? didOpen : didClose);
            action = kDoorStopped;
            DoorSound();
            //	isActive &= ~kIsActive;
        }

        // lastMovement is for external interfaces (sound, slide), so it belongs in classic units
        lastMovement[0] = ClassicCoefficient2(partList[0]->itsTransform[3][0] - oldOrigin[0]);
        lastMovement[1] = ClassicCoefficient2(partList[0]->itsTransform[3][1] - oldOrigin[1]);
        lastMovement[2] = ClassicCoefficient2(partList[0]->itsTransform[3][2] - oldOrigin[2]);

        FPS_DEBUG("lastMovement = " << FormatVector(lastMovement, 3) << "\n");

        if (itsSoundLink) {
            UpdateSoundLink(itsSoundLink, partList[0]->itsTransform[3], lastMovement, itsGame->soundTime);
        }
    }
}

void CDoorActor::StandingOn(CAbstractActor *who, //	Who is touching me?
    Fixed *where, //	Location of touch in global coordinates
    Boolean firstLeg) //	True, when this is the only or first leg to touch this object
{
    CGlowActors::StandingOn(who, where, firstLeg);

    if (firstLeg && action != kDoorStopped) {
        who->Slide(lastMovement);
    }
}

bool CDoorActor::IsGeometryStatic()
{
    if (!CGlowActors::IsGeometryStatic()) {
        return false;
    }
    
    if (doorStatus == kDoorClosed && openActivator.messageId > 0) {
        if (classicOpenSpeed != 0 && (
                deltas[0] != 0 ||
                deltas[1] != 0 ||
                deltas[2] != 0 ||
                twists[0] != 0 ||
                twists[1] != 0 ||
                twists[2] != 0
            )
        ) {
            return false;
        }
    }
    
    if (doorStatus == kDoorOpen && closeActivator.messageId > 0) {
        if (classicCloseSpeed != 0 && (
                deltas[0] != 0 ||
                deltas[1] != 0 ||
                deltas[2] != 0 ||
                twists[0] != 0 ||
                twists[1] != 0 ||
                twists[2] != 0
            )
        ) {
            return false;
        }
    }
    
    return true;
}
