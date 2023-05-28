/*
    Copyright ©1996-1997, Juri Munkki
    All rights reserved.

    File: CFreeSolid.c
    Created: Friday, August 9, 1996, 17:02
    Modified: Monday, March 10, 1997, 16:26
*/

// #define ENABLE_FPS_DEBUG  // uncomment if you want to see FPS_DEBUG output for this file

#include "CFreeSolid.h"

#include "CBSPWorld.h"
#include "CSmartPart.h"
#include "CWallActor.h"

extern CWallActor *lastWallActor;

#define MINSPEED FIX3(20)
#define kSolidGravity FIX3(120)
#define kFriction FIX3(950)

/*
**	Initialize default parameter values for this object.
*/
void CFreeSolid::BeginScript() {
    mass = FIX(50);
    baseMass = FIX(50);

    CRealMovers::BeginScript();

    ProgramLongVar(iShape, 0);

    ProgramMessage(iStartMsg, iStartMsg); //	Message to enable object (otherwise motionless)
    ProgramLongVar(iStopMsg, 0); //	Message to disable object (to stop it)
    ProgramLongVar(iStatus, false); //	Default is motionless (but iStartMsg will start it)

    ProgramLongVar(iShotPower, 0); //	Collisions cause damage relative to shotPower
    ProgramFixedVar(iCustomGravity, kSolidGravity); //	Private gravity/frame
    ProgramFixedVar(iAccelerate, kFriction); //	Private slowdown/frame while moving.
}

/*
**	Read back values that may have been changed by scripting.
*/
CAbstractActor *CFreeSolid::EndScript() {
    if (CRealMovers::EndScript()) {
        short shapeId;

        RegisterReceiver(&startMsg, ReadLongVar(iStartMsg)); //	Interested in start messages
        RegisterReceiver(&stopMsg, ReadLongVar(iStopMsg)); //	Interested in stop messages
        status = ReadLongVar(iStatus); //	What's our status? go/stop?

        hitPower = ReadFixedVar(iShotPower); //	Collision damage to other party
    
        classicAcceleration = ReadFixedVar(iAccelerate);
        classicGravity = ReadFixedVar(iCustomGravity);

        shapeId = ReadLongVar(iShape); //	Read our shape resource ID
        if (shapeId) {
            partCount = 1;
            LoadPartWithColors(0, shapeId); //	Create our shape
        } else if (lastWallActor) //	Use the last wall brick as our shape
        {
            CSmartPart *thePart;

            thePart = lastWallActor->partList[0];
            thePart->theOwner = this;

            partCount = 1;
            partList[0] = thePart;

            //TranslatePartY(thePart, ReadLongVar(iHeight));
            VECTORCOPY(location, thePart->itsTransform[3]);
            itsGame->itsWorld->RemovePart(thePart);

            heading = 0;
            lastWallActor->partList[0] = NULL;
            lastWallActor->partCount = 0;
            lastWallActor->Dispose(); //	Destroy wall actor (now without shape).
            lastWallActor = NULL;
        } else {
            Dispose();
            return NULL;
        }

        PlaceParts(); //	Locate shape
        LinkSphere(location, partList[0]->bigRadius); //	Collision detection maintenance

        maskBits |= kSolidBit; //	This object is solid.

        if (status)
            isActive |= kIsActive; //	Are we active?

        return this;

    } else {
        return NULL;
    }
}

void CFreeSolid::AdaptableSettings() {
    Fixed fpsSpeedOffset;
    FpsCoefficients(classicAcceleration, classicGravity, &acceleration, &customGravity, &fpsSpeedOffset);

    if (itsGame->frameNumber == 0) {
        // high-FPS initially falls a little slower so compensate by adding an offset
        speed[1] -= fpsSpeedOffset;
        location[1] += FpsCoefficient2(speed[1]);
    }
}

/*
**	Locate part in 3D.
*/
void CFreeSolid::PlaceParts() {
    partList[0]->Reset(); //	Reset transform matrix
    InitialRotatePartY(partList[0], heading); //	Rotate to heading
    TranslatePart(partList[0], location[0], location[1], location[2]); //	Translate to location
    partList[0]->MoveDone(); //	Signal that move was done.

    CRealMovers::PlaceParts(); //	Place attachments etc. by calling CRealMovers method.
}

/*
**	For each frame, perform the following actions.
*/
void CFreeSolid::FrameAction() {
    Vector oldLocation;
    CSmartPart *hitPart;

    CRealMovers::FrameAction();

    if (isActive & kHasMessage) //	Do we have a message waiting?
    {
        if (stopMsg.triggerCount > startMsg.triggerCount) //	More stops than starts?
        {
            status = false;
        } else if (stopMsg.triggerCount < startMsg.triggerCount) //	More starts than stops?
        {
            status = true;
        }

        stopMsg.triggerCount = 0; //	Clear message counts for next frame.
        startMsg.triggerCount = 0;

        isActive &= ~kHasMessage;
        if (status)
            isActive |= kIsActive;
        else
            isActive &= ~kIsActive;
    }

    if (status) //	If we are active, move.
    {
        FPS_DEBUG("\nframeNum = " << itsGame->frameNumber);
        FPS_DEBUG(", acceleration = " << acceleration << ", customGravity = " << customGravity << "\n");
        FPS_DEBUG("CFreeSolid initial location = " << FormatVector(location, 3) << ", speed = " << FormatVector(speed, 3) << "\n");

        //	Handle gravity
        if (location[1] > partList[0]->minBounds.y) {
            FPS_DEBUG("customGravity = " << customGravity << ", gravityRatio = " << itsGame->gravityRatio << "\n");
            speed[1] -= FMul(customGravity, itsGame->gravityRatio);
        }

        //	Handle friction
        speed[0] = FMul(speed[0], acceleration);
        speed[1] = FMul(speed[1], acceleration);
        speed[2] = FMul(speed[2], acceleration);

        FPS_DEBUG("CFreeSolid speed = " << FormatVector(speed, 3) << "\n");

        //	Check for ground level (not done by regular collision checks
        Fixed fallSpeed = ClassicCoefficient2(-partList[0]->minBounds.y - location[1]);
        if ( // speed[1] < 0 &&
            speed[1] < fallSpeed) { //	Bounce up on ground hit.
            speed[1] = fallSpeed;
            FPS_DEBUG("  using fallSpeed = " << fallSpeed << "\n");
        }

        //	If we are moving fast enough, translate in 3D.
        if (speed[0] > MINSPEED || speed[0] < -MINSPEED || speed[1] > MINSPEED || speed[1] < -MINSPEED ||
            speed[2] > MINSPEED || speed[2] < -MINSPEED) {
            VECTORCOPY(oldLocation, location);

            Vector locOffset;
            locOffset[0] = FpsCoefficient2(speed[0]);
            locOffset[1] = FpsCoefficient2(speed[1]);
            locOffset[2] = FpsCoefficient2(speed[2]);
            location[0] += locOffset[0];
            location[1] += locOffset[1];
            location[2] += locOffset[2];
            OffsetParts(locOffset);
            FPS_DEBUG("  location before collision = " << FormatVector(location, 3) << "\n");

            BuildPartProximityList(
                location, partList[0]->bigRadius + FDistanceOverEstimate(locOffset[0], locOffset[1], locOffset[2]), kSolidBit);

            hitPart = DoCollisionTest(&proximityList.p);
            if (hitPart) { //	If we hit something, cause damage to it.
                FPS_DEBUG("---- collision with object of type " << typeid(*hitPart->theOwner).name() << " -----\n");

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
                        if (anActor && anActor->searchCount != searchCount &&
                            (anActor->maskBits & kCollisionDamageBit) && partList[0]->CollisionTest(thePart)) {
                            itsGame->scoreReason = ksiObjectCollision;
                            anActor->BlastHit(&theBlast);
                            anActor->searchCount = searchCount;
                        }
                    }

                    SecondaryDamage(teamColor, -1);

                    // if part blew up, we're done here
                    if (!partList[0]) {
                        return;
                    }
                    BuildPartProximityList(location,
                                           partList[0]->bigRadius + FDistanceOverEstimate(locOffset[0], locOffset[1], locOffset[2]),
                                           kSolidBit);
                }

                //	Move back to where we were.

                locOffset[0] = -FpsCoefficient2(speed[0]);
                locOffset[1] = -FpsCoefficient2(speed[1]);
                locOffset[2] = -FpsCoefficient2(speed[2]);
                VECTORCOPY(location, oldLocation);
                OffsetParts(locOffset);

                //	Move again, but slide along obstacles.
                FindBestMovement(hitPart);
            }

            //	Collision detection maintenance:
            LinkSphere(location, partList[0]->bigRadius);
        }
        FPS_DEBUG("CFreeSolid final location = " << FormatVector(location, 3) << ", speed = " << FormatVector(speed, 3) << "\n");
    }
}
