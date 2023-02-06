/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CWeapon.c
    Created: Tuesday, February 13, 1996, 00:02
    Modified: Tuesday, September 17, 1996, 02:18
*/

#include "CWeapon.h"

#include "CDepot.h"
#include "CSmartPart.h"

#define GROUNDHITSOUNDID 211
#define DEFAULTMISSILESOUND 200
#define MINEBLOWSOUND 301

void CWeapon::IWeapon(CDepot *theDepot) {
    IAbstractActor();

    nextWeapon = NULL;

    maskBits |= kSolidBit | kTargetBit | kCollisionDamageBit | kCanPushBit;
    isActive = kIsActive;
    itsDepot = theDepot;
    itsGame = itsDepot->itsGame;
    blastSound = MINEBLOWSOUND;
    blastPower = FIX(1);
    sliverCounts[kSmallSliver] = 4;

    ResetWeapon();
}

void CWeapon::ResetWeapon() {
    speed[0] = speed[1] = speed[2] = speed[3] = 0;
    location[0] = location[1] = location[2] = location[3] = 0;

    clamp.me = this;
    clamp.next = NULL;

    pitch = yaw = roll = 0;

    hostIdent = 0;
    hostPart = NULL;
    OneMatrix(&relation);

    ownerSlot = 0;
    flyCount = 0;
    doExplode = false;

    InitLocationLinks();
    ident = 0;
    identLink = NULL;
}

void CWeapon::Deactivate() {
    if (itsSoundLink) {
        gHub->ReleaseLinkAndKillSounds(itsSoundLink);
        itsSoundLink = NULL;
    }

    itsDepot->RetireWeapon(this);
}

void CWeapon::LevelReset() {
    Deactivate();
}

void CWeapon::Locate() {}

void CWeapon::ReleaseAttachment() {
    CAbstractActor *host;

    if (hostPart) {
        host = itsGame->FindIdent(hostIdent);
        if (host) {
            host->Detach(&clamp);
            hostIdent = 0;
            hostPart = NULL;
        }
    }
}

void CWeapon::WasDestroyed() {
    CAbstractActor::WasDestroyed();

    itsGame->RemoveIdent(ident);
}

void CWeapon::Disarm() {
    ReleaseAttachment();
    UnlinkLocation();
    Deactivate();
}

long CWeapon::Arm(CSmartPart *aPart) {
    CAbstractActor *host;
    long result;

    ResetWeapon();
    AddToGame();
    result = ident;

    hostPart = aPart;
    host = (CAbstractActor *)hostPart->theOwner;
    clamp.me = this;
    hostIdent = host->Attach(&clamp);

    PlaceParts();

    ownerSlot = host->GetActorScoringId();
    teamColor = host->teamColor;
    teamMask = host->teamMask;

    host->searchCount = 1 + itsGame->searchCount;
    BuildPartSpheresProximityList(kSolidBit);

    if (DoCollisionTest(&proximityList.p)) {
        Disarm();
        result = 0;
    }

    return result;
}

void CWeapon::Fire() {
    Disarm();
}

void CWeapon::DoTargeting() {}

void CWeapon::ShowTarget() {}

void CWeapon::Explode() {
    Vector temp = {0, FIX(1), 0, 0};

    WasDestroyed();

    itsDepot->FireSlivers(
        15, location, temp, partList[0]->bigRadius >> 2, blastPower << 1, 160, 20, 0, itsDepot->smartSight);

    SecondaryDamage(teamColor, ownerSlot);
}

void CWeapon::PostMortemBlast(short scoreTeam, short scoreColor, Boolean doDispose) {
    CAbstractActor::PostMortemBlast(scoreTeam, scoreColor, false);

    Disarm();
}

void CWeapon::PreLoadSounds() {
    gHub->PreLoadSample(blastSound);
}

void CWeapon::Accelerate(Fixed *direction) {
    speed[0] += direction[0];
    speed[1] += direction[1];
    speed[2] += direction[2];
}
