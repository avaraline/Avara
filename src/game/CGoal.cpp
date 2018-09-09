/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CGoal.c
    Created: Sunday, August 25, 1996, 08:20
    Modified: Wednesday, September 4, 1996, 00:06
*/

#include "CGoal.h"

#include "CBall.h"
#include "CSmartPart.h"

void CGoal::BeginScript() {
    maskBits &= ~kSolidBit;
    maskBits |= kDoorIgnoreBit + kBallSnapBit;

    CGlowActors::BeginScript();

    ProgramLongVar(iGoalAction, kDoReset);
    ProgramLongVar(iGoalMsg, 0);
    ProgramLongVar(iShape, 251);
    ProgramLongVar(iRoll, 0);
    ProgramLongVar(iPitch, 0);

    ProgramFixedVar(iDeltaX, 0);
    ProgramFixedVar(iDeltaY, FIX3(1300));
    ProgramFixedVar(iDeltaZ, 0);
    ProgramLongVar(iGroup, -1);
    ProgramLongVar(iGoalScore, 500);

    ProgramFixedVar(iMotionRange, FIX3(1300));
    ProgramFixedVar(iRange, FIX3(100));

    ProgramLongVar(iSound, 434);
    ProgramFixedVar(iVolume, FIX(10));
}

CAbstractActor *CGoal::EndScript() {
    short resId;

    CGlowActors::EndScript();

    deltas[0] = ReadFixedVar(iDeltaX);
    deltas[1] = ReadFixedVar(iDeltaY);
    deltas[2] = ReadFixedVar(iDeltaZ);
    group = ReadLongVar(iGroup);
    goalMsg = ReadLongVar(iGoalMsg);
    motionRange = ReadFixedVar(iMotionRange);
    activeRange = ReadFixedVar(iRange);

    goalSound = ReadLongVar(iSound);
    goalVolume = ReadFixedVar(iVolume);
    gHub->PreLoadSample(goalSound);

    resId = ReadLongVar(iShape);

    if (GetResource('BSPT', resId)) {
        LoadPartWithColors(0, resId);
        partList[0]->Reset();
        partList[0]->RotateZ(ReadFixedVar(iRoll));
        partList[0]->RotateX(ReadFixedVar(iPitch));
        partList[0]->RotateOneY(heading);
        TranslatePart(partList[0], location[0], location[1], location[2]);
        partList[0]->MoveDone();
        LinkPartBoxes();

        partCount = 1;
        goalAction = ReadLongVar(iGoalAction);
        goalScore = ReadLongVar(iGoalScore);
    }

    return this;
}

short CGoal::GetBallSnapPoint(long theGroup,
    Fixed *ballLocation,
    Fixed *snapDest,
    Fixed *delta,
    CSmartPart **hostPart) {
    Fixed distanceEstimate;
    Fixed globDelta[3];

    if (theGroup & group) {
        delta[0] = deltas[0];
        delta[1] = deltas[1];
        delta[2] = deltas[2];

        *hostPart = partList[0];
        VectorMatrixProduct(1, (Vector *)delta, (Vector *)snapDest, &partList[0]->itsTransform);

        globDelta[0] = ballLocation[0] - snapDest[0];
        globDelta[1] = ballLocation[1] - snapDest[1];
        globDelta[2] = ballLocation[2] - snapDest[2];

        distanceEstimate = FDistanceEstimate(globDelta[0], globDelta[1], globDelta[2]);

        if (distanceEstimate < motionRange) {
            if (distanceEstimate < activeRange)
                return kSnapHere;
            else
                return kSnapAttract;
        }
    }

    return kSnapNot;
}

long CGoal::ReceiveSignal(long theSignal, void *miscData) {
    CBall *theBall;

    if (theSignal == kBallAttached) {
        theBall = (CBall *)miscData;

        DoSound(goalSound, location, goalVolume, FIX(1));
        itsGame->FlagMessage(goalMsg);

        itsGame->scoreReason = ksiScoreGoal;
        itsGame->Score(theBall->holderTeam, theBall->holderScoreId, goalScore, 0, teamColor, GetActorScoringId());
        if (goalAction) {
            ((CAbstractActor *)miscData)->ReceiveSignal(goalAction, 0);
        }
    }

    return CGlowActors::ReceiveSignal(theSignal, miscData);
}
