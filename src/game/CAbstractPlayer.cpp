/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CAbstractPlayer.c
    Created: Friday, March 10, 1995, 04:14
    Modified: Thursday, September 26, 1996, 00:06
*/

// #define ENABLE_FPS_DEBUG  // uncomment if you want to see FPS_DEBUG output for this file

#include "CAbstractPlayer.h"

#include "AvaraDefines.h"
#include "CBSPWorld.h"
#include "CScaledBSP.h"
#include "CDepot.h"
#include "CPlayerManager.h"
#include "CPlayerMissile.h"
#include "CScout.h"
#include "CSmartPart.h"
#include "CViewParameters.h"
//#include "CInfoPanel.h"
#include "InfoMessages.h"
#include "Messages.h"
//#include "Palettes.h"
#include "CApplication.h"
#include "CIncarnator.h"
#include "CRandomIncarnator.h"
#include "KeyFuncs.h"
//#include "LevelScoreRecord.h"
#include "CAbstractYon.h"
#include "CGrenade.h"
#include "CScoreKeeper.h"
#include "CSmart.h"
#include "ComputerVoice.h"
#include "Parser.h"
#include "Preferences.h"
#include "FastMat.h"

#include "Debug.h"

#define MOUSESHOOTDELAY 8
// replaced by kFOV preference
//#define MAXFOV FIX(60)
#define MINFOV FIX(5)
#define FOVSTEP FpsCoefficient2(FIX3(1500))
#define MINSPEED FIX3(10) //    15 mm/second at 15 fps
#define BOOSTLENGTH (16 * 5)
#define MINIBOOSTTIME 32

Boolean debug2Flag = false;

void CAbstractPlayer::LoadParts() {}

void CAbstractPlayer::LoadHUDParts() {
    short i;
    CBSPWorld *hudWorld;

    hudWorld = itsGame->hudWorld;

    for (i = 0; i < 2; i++) {
        targetOns[i] = CBSPPart::Create(kTargetOk);
        targetOns[i]->ReplaceColor(0xffff2600, ColorManager::getPlasmaSightsOnColor());
        targetOns[i]->privateAmbient = SIGHTSAMBIENT;
        targetOns[i]->yon = LONGYON * 2;
        targetOns[i]->usesPrivateYon = true;
        targetOns[i]->ignoreDirectionalLights = true;
        targetOns[i]->isTransparent = true;
        hudWorld->AddPart(targetOns[i]);

        targetOffs[i] = CBSPPart::Create(kTargetOff);
        targetOffs[i]->ReplaceColor(0xff008f00, ColorManager::getPlasmaSightsOffColor());
        targetOffs[i]->privateAmbient = SIGHTSAMBIENT;
        targetOffs[i]->yon = LONGYON * 2;
        targetOffs[i]->usesPrivateYon = true;
        targetOffs[i]->ignoreDirectionalLights = true;
        targetOffs[i]->isTransparent = true;
        hudWorld->AddPart(targetOffs[i]);
    }

    dirArrowHeight = FIX3(750);
    dirArrow = CBSPPart::Create(kDirIndBSP);
    dirArrow->ReplaceColor(0xff000000, ColorManager::getLookForwardColor());
    dirArrow->ignoreDirectionalLights = true;
    dirArrow->privateAmbient = FIX1;
    dirArrow->isTransparent = true;
    hudWorld->AddPart(dirArrow);

    LoadDashboardParts();
}

void CAbstractPlayer::StartSystems() {
    //  Get systems running:
    reEnergize = false;
    maxEnergy = FIX(5);
    energy = maxEnergy;
    boostsRemaining = 3;
    boostEndFrame = 0;
    boostControlLink = NULL;

    missileLimit = 999;
    grenadeLimit = 999;
    boosterLimit = 999;

    weaponIdent = 0;
    missileCount = 0;
    grenadeCount = 0;
    lookDirection = 0;

    maxShields = FIX(3); // Maximum shields are 3 units
    shields = maxShields;

    motors[0] = 0;
    motors[1] = 0;

    viewYaw = 0;
    viewPitch = 0;
    minPitch = FIX(-15);
    maxPitch = FIX(15);
    maxYaw = FIX(120);

    mass = FIX(150);
    baseMass = mass;
    turningEffect = FDegToOne(FIX(3.5));
    movementCost = FIX3(10);
#define CLASSICMOTORFRICTION FIX3(750)
#define CLASSICACCELERATION FIX3(250)
    maxAcceleration = CLASSICACCELERATION;
    didBump = true;

    groundSlide[0] = 0;
    groundSlide[1] = 0;
    groundSlide[2] = 0;
    groundSlide[3] = 0;

    fullGunEnergy = FIX3(800); //   Maximum single shot power is 0.8 units
    activeGunEnergy = FIX3(250); // Minimum single shot power is 0.25 units

    mouseShootTime = 0;
    gunEnergy[0] = fullGunEnergy;
    gunEnergy[1] = fullGunEnergy;

    gunOffset[0] = FIX3(250); //    Other gun will be automatically mirrored
    gunOffset[1] = 0;
    gunOffset[2] = FIX3(750);
    gunOffset[3] = 0;

    viewOffset[0] = 0;
    viewOffset[1] = FIX3(-250);
    viewOffset[2] = 0;
    viewOffset[3] = 0;

    maxFOV = FIX(itsGame->itsApp->Number(kFOV));
    fieldOfView = maxFOV;

    debugView = false;
    scoutView = false;
    scoutIdent = 0;
    scoutBaseHeight = FIX3(2000);

    sliverCounts[kSmallSliver] = 12;
    sliverCounts[kMediumSliver] = 18;
    sliverCounts[kLargeSliver] = 2;

    hitScore = 100;
    destructScore = 500;

    chatMode = false;

    nextGrenadeLoad = 0;
    nextMissileLoad = 0;

    // variables in AdaptableSettings need to have "classic" counterparts in case they are changed in CWalkerActor::ReceiveConfig()
    classicGeneratorPower = FIX3(30);
    classicShieldRegen = FIX3(30);       //  Use 0.030 per frame to repair shields
    classicChargeGunPerFrame = FIX3(35); //    Charge gun at 0.035 units per frame
    classicMotorFriction = CLASSICMOTORFRICTION;
}

void CAbstractPlayer::LevelReset() {
    ResetCamera();
    CAbstractActor::LevelReset();
}

void CAbstractPlayer::LoadScout() {
    scoutCommand = kScoutNullCommand;

    itsScout = new CScout(this, teamColor, GetTeamColorOr(ColorManager::getDefaultTeamColor()));
    itsScout->BeginScript();
    FreshCalc();
    itsScout->EndScript();
}

void CAbstractPlayer::ReplacePartColors() {
    teamMask = 1 << teamColor;
    longTeamColor = GetTeamColorOr(ColorManager::getDefaultTeamColor());

    for (CSmartPart **thePart = partList; *thePart; thePart++) {
        (*thePart)->ReplaceColor(*ColorManager::getMarkerColor(0), longTeamColor);
    }
}

void CAbstractPlayer::SetSpecialColor(ARGBColor specialColor) {
    longTeamColor = specialColor;
    for (CSmartPart **thePart = partList; *thePart; thePart++) {
        (*thePart)->ReplaceColor(*ColorManager::getMarkerColor(0), specialColor);
    }

    if (itsScout) {
        itsScout->partList[0]->ReplaceColor(*ColorManager::getMarkerColor(0), specialColor);
    }
}

void CAbstractPlayer::BeginScript() {
    itsManager = NULL;
    maskBits |= kSolidBit + kTargetBit + kPlayerBit + kCollisionDamageBit + kBallSnapBit;

    yonBound = LONGYON;

    isOut = false;
    winFrame = -1;
    teamColor = kGreenTeam;
    StartSystems();

    CRealMovers::BeginScript();

    ProgramReference(iHitSound, iPlayerHitSoundDefault);

    ProgramLongVar(iWinSound, 412);
    ProgramLongVar(iWinVolume, 12);
    ProgramLongVar(iLoseSound, 413);
    ProgramLongVar(iLoseVolume, 6);
    ProgramReference(iLives, iDefaultLives);

    teleportSoundLink = gHub->GetSoundLink();
}

CAbstractActor *CAbstractPlayer::EndScript() {
    CRealMovers::EndScript();

    doIncarnateSound = true;
    incarnateSound = ReadLongVar(iIncarnateSound);
    incarnateVolume = ReadFixedVar(iIncarnateVolume);

    winSound = ReadLongVar(iWinSound);
    winVolume = ReadFixedVar(iWinVolume);

    loseSound = ReadLongVar(iLoseSound);
    loseVolume = ReadFixedVar(iLoseVolume);

    // Preload sounds.
    auto _ = AssetManager::GetOgg(incarnateSound);
    _ = AssetManager::GetOgg(winSound);
    _ = AssetManager::GetOgg(loseSound);

    lives = ReadLongVar(iLives);

    itsSoundLink = gHub->GetSoundLink();

    nextActor = itsGame->freshPlayerList;
    itsGame->freshPlayerList = this;

    limboCount = 0;
    isInLimbo = false;
    netDestruct = false;
    isActive = kIsActive;
    nextPlayer = NULL;
    distance = 0;
    headChange = 0;
    didBump = false;
    fireGun = false;
    oldYaw = viewYaw;
    oldPitch = viewPitch;
    didSelfDestruct = false;

    LoadParts();
    ReplacePartColors();
    LoadHUDParts();
    LoadScout();
    PlaceParts();
    LinkPartSpheres();

    grenadeLimit = grenadeCount = ReadLongVar(iMaxStartGrenades);
    missileLimit = missileCount = ReadLongVar(iMaxStartMissiles);
    boosterLimit = boostsRemaining = ReadLongVar(iMaxStartBoosts);

    return NULL;
}

void CAbstractPlayer::AdaptableSettings() {
    // any settings that are affected by frame rate should go here, also double-check CWalkerActor::ReceiveConfig
    generatorPower = FpsCoefficient2(classicGeneratorPower);
    shieldRegen = FpsCoefficient2(classicShieldRegen);
    chargeGunPerFrame = FpsCoefficient2(classicChargeGunPerFrame); //    Charge gun at 0.035 units per frame
    FpsCoefficients(classicMotorFriction, FMul(classicMotorFriction, classicMotorAcceleration),
                    &motorFriction, &motorAcceleration);
}

void CAbstractPlayer::Dispose() {
    short i;
    CBSPWorld *hudWorld;

    if (itsGame->nextPlayer == this) {
        itsGame->nextPlayer = nextPlayer;
    }

    if (itsManager) {
        itsManager->SetPlayer(NULL);
        itsManager = NULL;
    }

    if (scoutIdent) {
        itsScout = (CScout *)gCurrentGame->FindIdent(scoutIdent);
    }

    if (itsScout) {
        itsScout->Dispose();
        scoutIdent = 0;
    }

    hudWorld = itsGame->hudWorld;

    hudWorld->RemovePart(dirArrow);
    delete dirArrow;

    for (i = 0; i < 2; i++) {
        hudWorld->RemovePart(targetOns[i]);
        delete targetOns[i];
        hudWorld->RemovePart(targetOffs[i]);
        delete targetOffs[i];
    }
    DisposeDashboard();

    gHub->ReleaseLink(teleportSoundLink);

    CRealMovers::Dispose();
}

void CAbstractPlayer::DisposeDashboard() {
    if (!itsGame->showNewHUD) return;

    CBSPWorld *hudWorld;
    hudWorld = itsGame->hudWorld;

    if (itsGame->itsApp->Get(kHUDShowMissileLock)) {
        hudWorld->RemovePart(lockLight);
        delete lockLight;
    }

    hudWorld->RemovePart(groundDirArrow);
    delete groundDirArrow;

    hudWorld->RemovePart(groundDirArrowSlow);
    delete groundDirArrowSlow;

    hudWorld->RemovePart(groundDirArrowFast);
    delete groundDirArrowFast;

    hudWorld->RemovePart(energyLabel);
    delete energyLabel;

    hudWorld->RemovePart(shieldLabel);
    delete shieldLabel;

    hudWorld->RemovePart(grenadeLabel);
    delete grenadeLabel;

    hudWorld->RemovePart(missileLabel);
    delete missileLabel;

    hudWorld->RemovePart(boosterLabel);
    delete boosterLabel;

    hudWorld->RemovePart(livesLabel);
    delete livesLabel;

    hudWorld->RemovePart(shieldGauge);
    delete shieldGauge;

    hudWorld->RemovePart(shieldGaugeBackLight);
    delete shieldGaugeBackLight;

    hudWorld->RemovePart(energyGauge);
    delete energyGauge;

    hudWorld->RemovePart(energyGaugeBackLight);
    delete energyGaugeBackLight;


    for (int i = 0; i < 4; i++) {
        hudWorld->RemovePart(grenadeMeter[i]);
        delete grenadeMeter[i];

        hudWorld->RemovePart(grenadeBox[i]);
        delete grenadeBox[i];

        hudWorld->RemovePart(missileMeter[i]);
        delete missileMeter[i];

        hudWorld->RemovePart(missileBox[i]);
        delete missileBox[i];

        hudWorld->RemovePart(boosterMeter[i]);
        delete boosterMeter[i];

        hudWorld->RemovePart(boosterBox[i]);
        delete boosterBox[i];

        hudWorld->RemovePart(livesMeter[i]);
        delete livesMeter[i];

        hudWorld->RemovePart(livesBox[i]);
        delete livesBox[i];
    }
}

/*
**  We ran into something. This routines undos movement
**  until we no longer overlap with the other object.
*/
void CAbstractPlayer::AvoidBumping() {
    //  Subclass responsibility.
}

void CAbstractPlayer::PlaceHUDParts() {
    short i;
    Matrix m1, m2;
    Matrix *mt;
    CBSPPart *theSight;
    CAbstractActor *theActor;
    RayHitRecord theHit;
    CWeapon *weapon = NULL;

    if (itsGame->itsApp->Get(kHUDArrowStyle) == 1) {
        dirArrow->Reset();
        InitialRotatePartY(dirArrow, heading);
        TranslatePart(dirArrow, location[0], location[1] + dirArrowHeight, location[2]);
        dirArrow->isTransparent = scoutView; // Invisible if scout view is on.
        dirArrow->MoveDone();
    }

    if (weaponIdent)
        weapon = (CWeapon *)gCurrentGame->FindIdent(weaponIdent);

    if (weapon) {
        weapon->ShowTarget();
    } else {
        mt = &viewPortPart->itsTransform;

        if (debug2Flag) {
            theHit.direction[0] = FMul((*mt)[2][0], PLAYERMISSILESPEED) + speed[0];
            theHit.direction[1] = FMul((*mt)[2][1], PLAYERMISSILESPEED) + speed[1];
            theHit.direction[2] = FMul((*mt)[2][2], PLAYERMISSILESPEED) + speed[2];
        } else {
            theHit.direction[0] = FMul((*mt)[2][0], PLAYERMISSILESPEED);
            theHit.direction[1] = FMul((*mt)[2][1], PLAYERMISSILESPEED);
            theHit.direction[2] = FMul((*mt)[2][2], PLAYERMISSILESPEED);
        }

        theHit.direction[3] = 0;
        NormalizeVector(3, theHit.direction);

        for (i = 0; i < 2; i++) {
            OneMatrix(&m1);
            MTranslate(i ? gunOffset[0] : -gunOffset[0], gunOffset[1], gunOffset[2], &m1);

            CombineTransforms(&m1, &m2, mt);

            theHit.distance = PLAYERMISSILERANGE;
            theHit.closestHit = NULL;
            theHit.origin[0] = m2[3][0];
            theHit.origin[1] = m2[3][1];
            theHit.origin[2] = m2[3][2];

            RayTestWithGround(&theHit, kSolidBit);

            if (theHit.closestHit) {
                theActor = theHit.closestHit->theOwner;
            } else {
                theActor = NULL;

                if (theHit.distance >= PLAYERMISSILERANGE / 4) {
                    theHit.distance = PLAYERMISSILERANGE / 4;
                }
            }

            if (scoutView)
                theHit.distance -= FIX3(200);
            else
                theHit.distance += FIX3(100);

            if (theActor && (theActor->maskBits & kTargetBit)) {
                theSight = targetOns[i];

                itsGame->itsApp->BrightBox(itsGame->frameNumber, theActor->GetPlayerPosition());
            } else {
                theSight = targetOffs[i];
            }

            if (gunEnergy[i] < activeGunEnergy)
                theSight->extraAmbient = 0;
            else if (gunEnergy[i] < fullGunEnergy)
                theSight->extraAmbient = GUNREADYAMBIENT;
            else
                theSight->extraAmbient = GUNFULLAMBIENT;

            theSight->isTransparent = false;
            theSight->Reset();
            if (i)
                PreFlipZ(theSight);

            if (debug2Flag) {
                theSight->ApplyMatrix(&m2);
                TranslatePart(theSight,
                    FMul(theHit.distance, theHit.direction[0]),
                    FMul(theHit.distance, theHit.direction[1]),
                    FMul(theHit.distance, theHit.direction[2]));
            } else {
                TranslatePartZ(theSight, theHit.distance);
                theSight->ApplyMatrix(&m2);
            }

            theSight->MoveDone();
        }
    }
    RenderDashboard();
}

CScaledBSP* CAbstractPlayer::DashboardPart(uint16_t id, Fixed scale) {
    CScaledBSP* bsp = new CScaledBSP(scale, id, this, 0);
    bsp->ReplaceAllColors(ColorManager::getHUDColor());
    bsp->privateAmbient = FIX1;
    bsp->ignoreDirectionalLights = true;
    bsp->isTransparent = true;
    itsGame->hudWorld->AddPart(bsp);
    return bsp;
}

CScaledBSP* CAbstractPlayer::DashboardPart(uint16_t id) {
    return DashboardPart(id, FIX1);
}

// Initialize dashboard parts
void CAbstractPlayer::LoadDashboardParts() {
    if (!itsGame->showNewHUD) return;

    dashboardSpinSpeed = ToFixed(100);
    dashboardSpinHeading = 0;

    int layout = itsGame->itsApp->Get(kHUDPreset);
    //float alpha = itsGame->itsApp->Get(kHUDAlpha);
    //Fixed hudAlpha = FIX1 * alpha;

    // Init components for PID Motion in the HUD
    // These scalars affect how quickly the two values converge
    pidReset(&pMotionY);
    pidReset(&pMotionX);
    pMotionX.P = -.1;
    pMotionX.I = -0.005;
    pMotionX.D = -.0004;
    pMotionX.angular = false;
    pMotionY.P = -.1;
    pMotionY.I = -0.09;
    pMotionY.D = -0.0014;
    pMotionY.angular = false;
    hudRestingX = 0;
    hudRestingY = 0;

    arrowDistance = itsGame->itsApp->Get(kHUDArrowDistance);
    arrowScale = itsGame->itsApp->Get(kHUDArrowScale);
    switch (layout-1) {
        case Close:
            gaugeBSP = kGaugeBSP;
            layoutScale = 1.0;
            boosterPosition[0] = 0.25f;
            boosterPosition[1] = -0.11f;
            livesPosition[0] = -.25f;
            livesPosition[1] = -0.11f;
            grenadePosition[0] = 0.19f;
            grenadePosition[1] = -0.26f;
            missilePosition[0] = -0.19f;
            missilePosition[1] = -0.26f;
            shieldPosition[0] = 0.23f;
            shieldPosition[1] = -0.14f;
            energyPosition[0] = -0.23f;
            energyPosition[1] = -0.14f;
            offsetMultiplier = .085f;
            boosterSpacing = 65.0f;
            livesSpacing = 65.0f;
            weaponSpacing = 35.0f;
            break;
        case Far:
            gaugeBSP = kGaugeBSP;
            layoutScale = 2.0;
            boosterPosition[0] = -0.87f;
            boosterPosition[1] = -0.13f;
            livesPosition[0] = 0.79f;
            livesPosition[1] = -0.23f;
            grenadePosition[0] = 0.96f;
            grenadePosition[1] = -0.35f;
            missilePosition[0] = 0.85f;
            missilePosition[1] = -0.35f;
            shieldPosition[0] = -0.91f;
            shieldPosition[1] = -0.20f;
            energyPosition[0] = -0.97f;
            energyPosition[1] = -0.20f;
            offsetMultiplier = .17f;
            boosterSpacing = 40.0f;
            livesSpacing = 40.0f;
            weaponSpacing = 16.0f;
            break;
    }

    if (itsGame->itsApp->Get(kHUDShowMissileLock)) {
        lockLight = DashboardPart(kLockLight, FIX3(600));
    }

    groundDirArrow = DashboardPart(kGroundDirArrow, FIX3(1000 * arrowScale));
    groundDirArrowSlow = DashboardPart(kGroundDirArrowSlow, FIX3(1000 * arrowScale));
    groundDirArrowFast = DashboardPart(kGroundDirArrowFast, FIX3(1000 * arrowScale));

    // Shields
    shieldLabel = DashboardPart(kShieldBSP, FIX3(70*layoutScale));
    shieldGauge = DashboardPart(gaugeBSP);
    shieldGauge->isMorphable = true;

    shieldGaugeBackLight = DashboardPart(gaugeBSP, ToFixed(layoutScale));
    shieldGaugeBackLight->privateAmbient = FIX3(80);

    // Energy
    energyLabel = DashboardPart(kEnergyBSP, FIX3(170*layoutScale));
    energyGauge = DashboardPart(gaugeBSP);
    energyGauge->isMorphable = true;

    energyGaugeBackLight = DashboardPart(gaugeBSP, ToFixed(layoutScale));
    energyGaugeBackLight->privateAmbient = FIX3(80);

    // Weapons
    grenadeLabel = DashboardPart(kGrenadeBSP, FIX3(700*layoutScale));
    missileLabel = DashboardPart(kMissileBSP, FIX3(700*layoutScale));
    boosterLabel = DashboardPart(kBoosterBSP, FIX3(20*layoutScale));
    livesLabel = DashboardPart(hullConfig.hullBSP, FIX3(140*layoutScale));
    for (int i = 0; i < 4; i++) {
        grenadeMeter[i] = DashboardPart(kFilledBox, FIX3(100*layoutScale));
        grenadeBox[i] = DashboardPart(kEmptyBox, FIX3(200*layoutScale));

        missileMeter[i] = DashboardPart(kFilledBox, FIX3(100*layoutScale));
        missileBox[i] = DashboardPart(kEmptyBox, FIX3(200*layoutScale));

        boosterMeter[i] = DashboardPart(kFilledBox, FIX3(40*layoutScale));
        boosterBox[i] = DashboardPart(kEmptyBox, FIX3(80*layoutScale));

        livesMeter[i] = DashboardPart(kFilledBox, FIX3(40*layoutScale));
        livesBox[i] = DashboardPart(kEmptyBox, FIX3(80*layoutScale));
    }
}

// Place parts on screen
void CAbstractPlayer::RenderDashboard() {
    if (!itsGame->showNewHUD) return;
    if (scoutView) {
        ResetDashboard();
        return;
    }

    float customInertia = itsGame->itsApp->Get(kHUDInertia);
    Vector relativeImpulse;

    relativeImpulse[0] = 0;
    relativeImpulse[1] = 0;
    relativeImpulse[2] = 0;
    if (dSpeed[0] != 0) {
        // Compare the vector of the incoming hit against which way the hector head is facing
        // Determine which quadrant the impact came from (TOP, BOTTOM) and (LEFT, RIGHT)
        // Lastly set relativeImpulse based on the impact location of the hit to bump the HUD
        Fixed hitAngle = FOneArcTan2(dSpeed[2], dSpeed[0]);
        Fixed angleDiff = hitAngle - viewYaw;
        
        if (angleDiff > 0) {
            // Hit from the right side
            relativeImpulse[0] = FIX(1.0);
        } else if (angleDiff < 0) {
            // Hit from the left side
            relativeImpulse[0] = FIX(-1.0);
        }

        if (dSpeed[1] > FIX(.5)) {
            // Hit from the top
            relativeImpulse[1] = FIX(1.0);
        } else if (dSpeed[1] < FIX(-.5)) {
            // Hit from the bottom
            relativeImpulse[1] = FIX(-1.0);
        }

        pidReset(&pMotionX);
        pidReset(&pMotionY);
    }

    // Push HUD elements away from resting position based on movement factors
    hudRestingX -= ToFloat(FMul(FIX(.005), dYaw) + relativeImpulse[0]) * customInertia; // Head (mouse) movement
    hudRestingY -= ToFloat(FMul(FIX(.02), dPitch - FMul(dElevation, FIX(10))) + relativeImpulse[1]) * customInertia; // Head (mouse) movement and jumping/falling

    // Reset delta impulse values so they only apply once
    // pidMotion handles the rest of the HUD movement after the impact is over
    dSpeed[0] = 0;
    dSpeed[1] = 0;
    dSpeed[2] = 0;
    relativeImpulse[0] = 0;
    relativeImpulse[1] = 0;
    relativeImpulse[2] = 0;

    // Clamp HUD movement values
    if (hudRestingX > 3) hudRestingX = 3;
    if (hudRestingX < -3) hudRestingX = -3;
    if (hudRestingY > 3) hudRestingY = 3;
    if (hudRestingY < -3) hudRestingY = -3;

    // Use PID Motion to move HUD elements back to resting position
    hudRestingX += pidUpdate(&pMotionX, .75, hudRestingX, 0.0);
    hudRestingY += pidUpdate(&pMotionY, .75, hudRestingY, 0.0);

    // Prevent jitter for values that are very close to zero
    // Specifically prevents alternating between 0 and -0
    if (abs(hudRestingX) < .01f) hudRestingX = abs(hudRestingX);
    if (abs(hudRestingY) < .01f) hudRestingY = abs(hudRestingY);

    // Reset pidMotion if movement has fully equalized
    if (hudRestingX == 0.0f && !pMotionX.fresh) pidReset(&pMotionX);
    if (hudRestingY == 0.0f && !pMotionY.fresh) pidReset(&pMotionY);

    CWeapon *weapon = NULL;
    dashboardSpinHeading += FpsCoefficient2(FDegToOne(dashboardSpinSpeed));

    if (weaponIdent)
        weapon = (CWeapon *)gCurrentGame->FindIdent(weaponIdent);

    if (itsGame->itsApp->Get(kHUDShowMissileLock)) {
        if (weapon && weapon->isTargetLocked) {
            DashboardPosition(lockLight, 0.0, -0.1);
            lockLight->Scale(FIX(.6));
        }
    }

    if (itsGame->itsApp->Get(kHUDArrowStyle) == 2) {
        CScaledBSP *arrow;
        if (distance > FIX3(650)) {
            arrow = groundDirArrowFast;
        } else if (distance > FIX3(250)) {
            arrow = groundDirArrowSlow;
        } else {
            arrow = groundDirArrow;
        }
        float dist = itsGame->itsApp->Get(kHUDArrowDistance);
        DashboardFixedPosition(arrow, dist, 0);
    }

    // Ammo Labels
    if (itsGame->itsApp->Get(kHUDShowGrenadeCount) && grenadeLimit != 0) {
        DashboardPosition(grenadeLabel, false, grenadePosition[0], grenadePosition[1]-(.09f*(layoutScale/2.0)), 0, dashboardSpinHeading, 0);
    }

    if (itsGame->itsApp->Get(kHUDShowMissileCount) && missileLimit != 0) {
        DashboardPosition(missileLabel, false, missilePosition[0], missilePosition[1]-(.09f*(layoutScale/2.0)), 0, dashboardSpinHeading, 0);
    }

    if (itsGame->itsApp->Get(kHUDShowBoosterCount) && boosterLimit != 0) {
        DashboardPosition(boosterLabel, false, boosterPosition[0], boosterPosition[1]-(.03f*(layoutScale/2.0)), FIX(90.0), FIX(60.0), 0);
    }

    if (itsGame->itsApp->Get(kHUDShowLivesCount) && lives > 0 && lives <= 10) {
        DashboardPosition(livesLabel, false, livesPosition[0], livesPosition[1]-(.045f*(layoutScale/2.0)), FIX(40.0), dashboardSpinHeading, 0);
    }

    // Ammo counts
    for (int i = 0; i < 4; i++) {
        if (itsGame->itsApp->Get(kHUDShowGrenadeCount) && 0 < grenadeLimit) {
            if (float(i)/4.0 < float(grenadeCount)/float(grenadeLimit)) {
                // Fill box
                DashboardPosition(grenadeMeter[i], true, grenadePosition[0], grenadePosition[1]+(float(i)/weaponSpacing));
            } else {
                // Empty box
                DashboardPosition(grenadeBox[i], true, grenadePosition[0], grenadePosition[1]+(float(i)/weaponSpacing));
            }
        }

        if (itsGame->itsApp->Get(kHUDShowMissileCount) && 0 < missileLimit) {
            if (i < missileCount) {
                // Fill box
                DashboardPosition(missileMeter[i], true, missilePosition[0], missilePosition[1]+(float(i)/weaponSpacing));
            } else {
                // Empty box
                DashboardPosition(missileBox[i], true, missilePosition[0], missilePosition[1]+(float(i)/weaponSpacing));
            }
        }

        if (itsGame->itsApp->Get(kHUDShowBoosterCount) && 0 < boosterLimit) {
            if (i < boostsRemaining) {
                // Fill box
                DashboardPosition(boosterMeter[i], true, boosterPosition[0], boosterPosition[1]+(float(i)/boosterSpacing));
            } else {
                // Empty box
                DashboardPosition(boosterBox[i], true, boosterPosition[0], boosterPosition[1]+(float(i)/boosterSpacing));
            }
        }

        if (itsGame->itsApp->Get(kHUDShowLivesCount) && lives > 0 && lives <= 10) {
            if (i < lives) {
                // Fill box
                DashboardPosition(livesMeter[i], true, livesPosition[0], livesPosition[1]+(float(i)/livesSpacing));
            } else {
                // Empty box
                DashboardPosition(livesBox[i], true, livesPosition[0], livesPosition[1]+(float(i)/livesSpacing));
            }
        }
    }

    // Shields
    if (itsGame->itsApp->Get(kHUDShowShieldGauge)) {
        DashboardPosition(shieldLabel, false, shieldPosition[0], shieldPosition[1]-(.22f*(layoutScale/2.0)));
        DashboardPosition(shieldGaugeBackLight, shieldPosition[0], shieldPosition[1]);

        Fixed shieldPercent = 0;
        if (maxShields > 0) shieldPercent = FDiv(shields, maxShields);

        if (shieldPercent <= FIX3(333)) {
            shieldGauge->ReplaceAllColors(ColorManager::getHUDCriticalColor());
        } else if (shieldPercent <= FIX3(666)) {
            shieldGauge->ReplaceAllColors(ColorManager::getHUDWarningColor());
        } else {
            shieldGauge->ReplaceAllColors(ColorManager::getHUDColor());
        }

        // Scale based on damage taken
        Fixed shieldHeight = FMul(ToFixed(layoutScale), shieldPercent);

        // Get the inverse of the shield percent
        // Gauge moves further when energy is lower
        float shieldYOffset = abs(ToFloat(FIX1 - shieldPercent));

        DashboardPosition(shieldGauge, shieldPosition[0], shieldPosition[1] - (offsetMultiplier * shieldYOffset));
        shieldGauge->ScaleXYZ(ToFixed(layoutScale), shieldHeight, ToFixed(layoutScale));
    }

    // Energy
    if (itsGame->itsApp->Get(kHUDShowEnergyGauge)) {
        DashboardPosition(energyLabel, false, energyPosition[0], energyPosition[1]-(.21f*(layoutScale/2.0)));
        DashboardPosition(energyGaugeBackLight, energyPosition[0], energyPosition[1]);

        Fixed energyPercent = 0;
        if (maxEnergy > 0) energyPercent = FDiv(energy, maxEnergy);

        if (boostEndFrame > itsGame->frameNumber) {
            energyGauge->ReplaceAllColors(ColorManager::getHUDPositiveColor());
        }
        else if (energyPercent <= FIX(.25)) {
            energyGauge->ReplaceAllColors(ColorManager::getHUDCriticalColor());
        } else {
            energyGauge->ReplaceAllColors(ColorManager::getHUDColor());
        }

        // Scale based on energy lost
        Fixed energyHeight = FMul(ToFixed(layoutScale), energyPercent);

        // Get the inverse of the energy percent
        // Gauge moves further when energy is lower
        float energyYOffset = abs(ToFloat(FIX1 - energyPercent));

        DashboardPosition(energyGauge, energyPosition[0], energyPosition[1] - (offsetMultiplier * energyYOffset ));
        energyGauge->ScaleXYZ(ToFixed(layoutScale), energyHeight, ToFixed(layoutScale));
    }
}

void CAbstractPlayer::DashboardPosition(CScaledBSP *part, float x, float y) {
    DashboardPosition(part, false, x, y, 0, 0, 0);
}
void CAbstractPlayer::DashboardPosition(CScaledBSP *part, bool autoRot, float x, float y) {
    DashboardPosition(part, autoRot, x, y, 0, 0, 0);
}
void CAbstractPlayer::DashboardPosition(CScaledBSP *part, bool autoRot, float x, float y, Fixed x_rot, Fixed y_rot, Fixed z_rot) {
    // Draw a part on the dashboard
    // Coordinates on the screen are roughly described as a percentage of the screen away from the bottom and the left
    // (-1.0, -1.0) is the bottom left of the screen
    // (1.0, 1.0) is the top right of the screen


    // TODO: Adjust these until it looks good, then go multiply 
    // all the DashboardPosition parameters by these numbers, and
    // then delete these
    float scale_x = 11.12;
    float scale_y = 8.23;
    Fixed hud_dist = (FIX3(6000) * 25)/8;

    // Turn elements toward the center of the screen for a better 3d look
    // Being further away from the center results in greater rotation
    float yAng = 0;

    if (autoRot) {
        if (x < 0.0) {
            yAng = -60*x + 15;
        }

        if (x > 0.0) {
            yAng = -60*x - 15;
        }
    }

    //InitialRotatePartX(part, FDegToRad(x_rot));
    //InitialRotatePartY(part, FDegToRad(y_rot + ToFixed(yAng)));
    //InitialRotatePartZ(part, FDegToRad(z_rot));
    part->isTransparent = false;
    part->Reset();
    part->RotateOneX(FDegToOne(x_rot));
    part->RotateOneZ(FDegToOne(z_rot));
    part->RotateOneY(FDegToOne(ToFixed(yAng) + y_rot));
    TranslatePart(part, -ToFixed((x * scale_x) + hudRestingX), ToFixed((y * scale_y) + hudRestingY), hud_dist);
    part->ApplyMatrix(&viewPortPart->itsTransform);
    part->MoveDone();
}

void CAbstractPlayer::DashboardFixedPosition(CScaledBSP *part, float dist, Fixed angle) {
    DashboardFixedPosition(part, dist, angle, 0, 0, 0, 0);
}
void CAbstractPlayer::DashboardFixedPosition(CScaledBSP *part, float dist, Fixed angle, float height, Fixed x_rot, Fixed y_rot, Fixed z_rot) {
    if (part == nullptr) return;

    // Place a part in a fixed position relative to the HECTOR.
    // Part rotates with the HECTOR facing instead of the head
    Fixed finalAngle = heading - FDegToOne(angle);
    part->isTransparent = false;
    part->Reset();
    part->RotateOneY(finalAngle);

    TranslatePart(part, location[0] + FMul(FIX(dist), FOneSin(finalAngle)),
                        location[1] + FIX(height),
                        location[2] + FMul(FIX(dist), FOneCos(finalAngle)));
    part->MoveDone();
}

void CAbstractPlayer::ResetDashboard() {
    if (!itsGame->showNewHUD) return;

    if (itsGame->itsApp->Get(kHUDShowMissileLock)) {
        lockLight->isTransparent = true;
    }
    groundDirArrow->isTransparent = true;
    groundDirArrowSlow->isTransparent = true;
    groundDirArrowFast->isTransparent = true;
    shieldLabel->isTransparent = true;
    energyLabel->isTransparent = true;
    grenadeLabel->isTransparent = true;
    missileLabel->isTransparent = true;
    boosterLabel->isTransparent = true;
    livesLabel->isTransparent = true;
    shieldGauge->isTransparent = true;
    shieldGaugeBackLight->isTransparent = true;
    energyGauge->isTransparent = true;
    energyGaugeBackLight->isTransparent = true;

    for (int i = 0; i < 4; i++) {
        grenadeMeter[i]->isTransparent = true;
        grenadeBox[i]->isTransparent = true;

        missileMeter[i]->isTransparent = true;
        missileBox[i]->isTransparent = true;

        boosterMeter[i]->isTransparent = true;
        boosterBox[i]->isTransparent = true;

        livesMeter[i]->isTransparent = true;
        livesBox[i]->isTransparent = true;
    }
}

void CAbstractPlayer::ControlSoundPoint() {
    Fixed theRight[] = {FIX(-1), 0, 0};
    Matrix *m;

    m = &viewPortPart->itsTransform;
    theRight[0] = -(*m)[0][0];
    theRight[1] = -(*m)[0][1];
    theRight[2] = -(*m)[0][2];

    gHub->SetMixerLink(itsSoundLink);
    gHub->UpdateRightVector(theRight);
}

void CAbstractPlayer::ControlViewPoint() {
    CViewParameters *theView;

    theView = itsGame->itsView;

    if (!isInLimbo)
        PlaceHUDParts();

    if (scoutView && scoutIdent) // && winFrame < 0)
    {
        dirArrow->isTransparent = true;

        if (scoutIdent && !debugView) {
            itsScout = (CScout *)itsGame->FindIdent(scoutIdent);
            if (itsScout)
                itsScout->ControlViewPoint();
        }
    } else {
        MATRIXCOPY(&theView->viewMatrix, viewPortPart->GetInverseTransform());
        MTranslate(viewOffset[0], viewOffset[1], viewOffset[2], &theView->viewMatrix);

        if (lookDirection) {
            Fixed a;

            a = FMul(lookDirection, 43690);
            MRotateY(FOneSin(a), FOneCos(a), &theView->viewMatrix);
        }

        theView->inverseDone = false;
    }

    RecalculateViewDistance();

    // SetPort(itsGame->itsWindow);
    ControlSoundPoint();
}

void CAbstractPlayer::RecalculateViewDistance() {
    CViewParameters *theView;
    Fixed viewDist;
    Fixed frameYon;

    theView = itsGame->itsView;

    viewDist = FMulDivNZ(theView->viewWidth, FDegCos(fieldOfView), 2 * FDegSin(fieldOfView));

    if (itsGame->yonList) {
        frameYon = itsGame->yonList->AdjustYon((*theView->GetInverseMatrix())[3], yonBound);
    } else {
        frameYon = yonBound;
    }

    if (viewDist != theView->viewDistance || frameYon != theView->yonBound) {
        if (frameYon != theView->yonBound) {
            theView->yonBound = frameYon;
        }
        theView->viewDistance = viewDist;
        theView->Recalculate();
        theView->CalculateViewPyramidCorners();
    }
}

void CAbstractPlayer::ResetCamera() {
    fieldOfView = maxFOV;
    AvaraGLSetFOV(ToFloat(fieldOfView));
    RecalculateViewDistance();
}

void CAbstractPlayer::ReturnWeapon(short theKind) {
    switch (theKind) {
        case kweSmart:
            missileCount++;
            break;
        case kweGrenade:
            grenadeCount++;
            break;
    }
}

void CAbstractPlayer::ArmSmartMissile() {
    CWeapon *theWeapon;
    short oldKind = -1;
    Boolean didDetach;

    didDetach = SignalAttachments(kBallReleaseSignal, this);

    if (weaponIdent) {
        theWeapon = (CWeapon *)itsGame->FindIdent(weaponIdent);
        if (theWeapon) {
            oldKind = theWeapon->weaponKind;
            theWeapon->Disarm();
        }

        weaponIdent = 0;
    }

    if (!didDetach && oldKind != kweSmart && missileCount) {
        if (nextMissileLoad < itsGame->frameNumber) {
            theWeapon = itsGame->itsDepot->AquireWeapon(kweSmart);
            weaponIdent = theWeapon->Arm(viewPortPart);
            if (weaponIdent) {
                missileCount--;
                nextMissileLoad = itsGame->FramesFromNow(3);
            }
        } else
            fireGun = false;
    }

    ReturnWeapon(oldKind);
}

void CAbstractPlayer::ArmGrenade() {
    CWeapon *theWeapon;
    short oldKind = -1;
    Boolean didDetach;

    didDetach = SignalAttachments(kBallReleaseSignal, this);

    if (weaponIdent) {
        theWeapon = (CWeapon *)itsGame->FindIdent(weaponIdent);

        if (theWeapon) {
            oldKind = theWeapon->weaponKind;
            theWeapon->Disarm();
        }

        weaponIdent = 0;
    }

    if (!didDetach && oldKind != kweGrenade && grenadeCount) {
        if (nextGrenadeLoad < itsGame->frameNumber) {
            theWeapon = itsGame->itsDepot->AquireWeapon(kweGrenade);
            weaponIdent = theWeapon->Arm(viewPortPart);
            if (weaponIdent) {
                grenadeCount--;
                nextGrenadeLoad = itsGame->FramesFromNow(2);
            }
        } else
            fireGun = false;
    }

    ReturnWeapon(oldKind);
}

void CAbstractPlayer::KeyboardControl(FunctionTable *ft) {
    if (ft) {
        short motionFlags;
        Fixed yaw = viewYaw;
        Fixed pitch = viewPitch;

        viewYaw -= ft->mouseDelta.h * FIX(.0625);
        viewPitch += ft->mouseDelta.v * FIX(.03125);

        if (viewYaw < -maxYaw)
            viewYaw = -maxYaw;
        if (viewYaw > maxYaw)
            viewYaw = maxYaw;
        if (viewPitch < minPitch)
            viewPitch = minPitch;
        if (viewPitch > maxPitch)
            viewPitch = maxPitch;

        dYaw = yaw - viewYaw;
        dPitch = pitch - viewPitch;

        if (TESTFUNC(kfuAimForward, ft->held)) {
            Fixed scale = FpsCoefficient1(FIX(0.5));
            viewPitch = FMul(viewPitch, scale);
            viewYaw = FMul(viewYaw, scale);
        }

        fireGun = false;

        if (!isInLimbo) {
            classicMotorAcceleration = FDivNZ(baseMass, GetTotalMass());
            classicMotorAcceleration = FMul(classicMotorAcceleration, classicMotorAcceleration);
            classicMotorAcceleration = FMul(CLASSICACCELERATION, classicMotorAcceleration);

            // This is a tricky one (maybe a minor bug from the original code?)...
            // Because the friction (f) was applied after adding/subtracting acceleration
            // the implied acceleration (a) is actualy f*a.  Mathematically,
            //    motor = (motor + a)*f = f*motor + f*a
            // We will use the more common equation,
            //    motor = f*motor + a
            // But we have to adjust how we calculate multipliers
            FpsCoefficients(classicMotorFriction, FMul(classicMotorFriction, classicMotorAcceleration),
                           &motorFriction, &motorAcceleration, &fpsMotorOffset);

            if (itsGame->isClassicFrame) {
                FPS_DEBUG("----------------------------------------------------" << std::endl);
                FPS_DEBUG("baseMass = " << baseMass << ", totalmass = " << GetTotalMass() << ", classicFriction = " << classicMotorFriction << ", friction = " << motorFriction << ", classicAcceleration = " << classicMotorAcceleration << ", acceleration = " << motorAcceleration << ", fpsMotorOffset = " << fpsMotorOffset << std::endl);
            }
            FPS_DEBUG("frameNumber = " << itsGame->frameNumber << std::endl);
            FPS_DEBUG("initial location = " << FormatVector(location, 3) << ", heading = " << ToFloat(heading)*360 << ", speed = " << FormatVector(speed, 3) << std::endl);

            FPS_DEBUG("   motors before = " << FormatVector(motors, 2) << std::endl);

            motors[0] = FMul(motors[0], motorFriction);
            motors[1] = FMul(motors[1], motorFriction);
            FPS_DEBUG("   motors after fric = " << FormatVector(motors, 2) << std::endl);

            motionFlags = 0;

            if (TESTFUNC(kfuForward, ft->held)) {
                motionFlags |= 1 + 2; // +left, +right
                FPS_DEBUG("  ** kfuForward **");
            }
            if (TESTFUNC(kfuReverse, ft->held)) {
                motionFlags |= 4 + 8; // -left, -right
                FPS_DEBUG("  ** kfuReverse **");
            }
            if (TESTFUNC(kfuLeft, ft->held)) {
                motionFlags |= 2 + 4; // +right, -left
                FPS_DEBUG("  ** kfuLeft **");
            }
            if (TESTFUNC(kfuRight, ft->held)) {
                motionFlags |= 1 + 8; // +left, -right
                FPS_DEBUG("  ** kfuRight **");
            }

            if (motionFlags & 1) {
                if (motors[0] <= 0) { // switching direction or starting to move
                    motors[0] += fpsMotorOffset;
                }
                motors[0] += motorAcceleration; // left leg forward
            }
            if (motionFlags & 2) {
                if (motors[1] <= 0) {
                    motors[1] += fpsMotorOffset;
                }
                motors[1] += motorAcceleration; // right leg forward
            }
            if (motionFlags & 4) {
                if (motors[0] >= 0) {
                    motors[0] -= fpsMotorOffset;
                }
                motors[0] -= motorAcceleration; // left leg backward
            }
            if (motionFlags & 8) {
                if (motors[1] >= 0) {
                    motors[1] -= fpsMotorOffset;
                }
                motors[1] -= motorAcceleration; // right leg backward
            }

            FPS_DEBUG("   motors after keyb  = " << FormatVector(motors, 2) << std::endl);

            if (TESTFUNC(kfuBoostEnergy, ft->down) && boostsRemaining && (boostEndFrame < itsGame->frameNumber)) {
                CBasicSound *theSound;

                boostsRemaining--;
                boostEndFrame = itsGame->FramesFromNow(BOOSTLENGTH);

                if (!boostControlLink)
                    boostControlLink = gHub->GetSoundLink();

                boostControlLink->meta = metaNoData;
                theSound = gHub->GetSoundSampler(hubRate, 413);
                theSound->SetVolume(FIX(2));
                theSound->SetSoundLink(itsSoundLink);
                theSound->SetControlLink(boostControlLink);
                theSound->SetSoundLength((BOOSTLENGTH * CLASSICFRAMETIME) << 6);
                theSound->Start();
            }

            if (mouseShootTime-- <= 0) {
                if (ft->buttonStatus & kbuIsDown)
                    fireGun = true;
                if (TESTFUNC(kfuFireWeapon, ft->held))
                    fireGun = true;
            }
            if (ft->buttonStatus & kbuWentDown)
                fireGun = true;
            if (TESTFUNC(kfuFireWeapon, ft->down))
                fireGun = true;

            if (TESTFUNC(kfuLoadGrenade, ft->down))
                ArmGrenade();

            if (TESTFUNC(kfuLoadMissile, ft->down))
                ArmSmartMissile();
        }
        else if(lives == 0) {
            if (itsManager->IsLocalPlayer() && TESTFUNC(kfuSpectateNext, ft->down)) {
                itsGame->SpectateNext();
            }
            if (itsManager->IsLocalPlayer() && TESTFUNC(kfuSpectatePrevious, ft->down)) {
                itsGame->SpectatePrevious();
            }
        }

        if (itsManager->IsLocalPlayer()) {
            if(TESTFUNC(kfuScoreboard, ft->down))
                itsManager->SetShowScoreboard(!itsManager->GetShowScoreboard());
        }

        if (TESTFUNC(kfuPauseGame, ft->down)) {
            if(lives > 0) {
                itsGame->statusRequest = kPauseStatus;
                itsGame->pausePlayer = itsManager->Slot();
            }
        }
        if (TESTFUNC(kfuAbortGame, ft->down)) {
            if (limboCount > 0 || lives == 0) {
                if (didSelfDestruct || winFrame >= 0 || lives == 0) {
                    isOut = true;
                    lives = 0;
                    itsManager->AbortRequest();
                    itsGame->itsApp->DrawUserInfoPart(itsManager->Slot(), kipDrawColorBox);
                }
            } else {
                if (itsManager->IsLocalPlayer()) {
                    itsGame->itsApp->MessageLine(kmSelfDestruct, MsgAlignment::Center);
                    if (lives > 1)
                        itsGame->itsApp->MessageLine(kmSelfDestruct2, MsgAlignment::Center);
                }

                WasDestroyed();
                itsGame->scoreReason = ksiSelfDestructBlast;
                SecondaryDamage(teamColor, GetActorScoringId(), ksiSelfDestructBlast);
                didSelfDestruct = true;
            }
        }

        if (winFrame < 0) {
            Boolean doRelease = false;

            if (TESTFUNC(kfuScoutView, ft->down)) {
                if (!scoutView && !scoutIdent) {
                    doRelease = true;
                    scoutCommand = kScoutFollow;
                }

                scoutView = !scoutView;
            }

            if (TESTFUNC(kfuScoutControl, ft->down)) {
                scoutCommand = scoutIdent ? kScoutDown : kScoutFollow;
            }

            if (TESTFUNC(kfuScoutControl, ft->held)) {
                if (TESTFUNC(kfuForward, ft->down))
                    scoutCommand = kScoutLead;
                if (TESTFUNC(kfuReverse, ft->down))
                    scoutCommand = kScoutFollow;
                if (TESTFUNC(kfuLeft, ft->down))
                    scoutCommand = kScoutLeft;
                if (TESTFUNC(kfuRight, ft->down))
                    scoutCommand = kScoutRight;
                if (TESTFUNC(kfuAimForward, ft->down))
                    scoutCommand = kScoutUp;
            }

            if ((doRelease || TESTFUNC(kfuScoutControl, ft->up)) && scoutCommand != kScoutNullCommand) {
                if (scoutIdent) {
                    itsScout = (CScout *)itsGame->FindIdent(scoutIdent);
                }
                if (itsScout)
                    itsScout->ToggleState(scoutCommand);

                scoutCommand = kScoutNullCommand;
            }
        }

        if (TESTFUNC(kfuDebug1, ft->down))
            debugView = !debugView;
        if (TESTFUNC(kfuDebug2, ft->down))
            debug2Flag = !debug2Flag;

        if (TESTFUNC(kfuZoomIn, ft->held))
            fieldOfView -= FOVSTEP;
        if (TESTFUNC(kfuZoomOut, ft->held))
            fieldOfView += FOVSTEP;

#define LOOKSTEP FpsCoefficient2(0x1000L)
#define MAXSIDELOOK 0x8000L

        if (TESTFUNC(kfuLookLeft, ft->held)) {
            lookDirection -= LOOKSTEP;
            if (lookDirection < -MAXSIDELOOK)
                lookDirection = -MAXSIDELOOK;
        } else if (TESTFUNC(kfuLookRight, ft->held)) {
            lookDirection += LOOKSTEP;
            if (lookDirection > MAXSIDELOOK)
                lookDirection = MAXSIDELOOK;
        } else {
            if (lookDirection < -LOOKSTEP) {
                lookDirection += LOOKSTEP;
            } else if (lookDirection > LOOKSTEP) {
                lookDirection -= LOOKSTEP;
            } else {
                lookDirection >>= 1;
            }
        }

        if (fieldOfView < MINFOV)
            fieldOfView = MINFOV;
        if (fieldOfView > maxFOV)
            fieldOfView = maxFOV;

        if (itsManager->IsLocalPlayer() &&
            (TESTFUNC(kfuZoomOut, ft->held) || TESTFUNC(kfuZoomIn, ft->held)))
            AvaraGLSetFOV(ToFloat(fieldOfView));

        if (fireGun)
            mouseShootTime = FpsFramesPerClassic(MOUSESHOOTDELAY);

        if (TESTFUNC(kfuTypeText, ft->down)) {
            chatMode = !chatMode;
            itsGame->itsApp->DrawUserInfoPart(itsManager->Slot(), kipDrawColorBox);
            if (chatMode) {
                CBasicSound *theSound;

                theSound = gHub->GetSoundSampler(hubRate, itsManager->IsLocalPlayer() ? 151 : 152);
                theSound->SetVolume(FIX3(250));
                theSound->Start();
            }
        }

        if (ft->msgChar) {
            itsManager->GameKeyPress(ft->msgChar);
        }
    }
}

void CAbstractPlayer::Slide(Fixed *direction) {
    groundSlide[0] = *direction++;
    groundSlide[1] = *direction++;
    groundSlide[2] = *direction++;
}

void CAbstractPlayer::TractionControl() {
}

void CAbstractPlayer::MotionControl() {
    Fixed avrgHeading;
    Fixed motorDir[2];
    Fixed slide[2];
    Fixed slideLen;
    Fixed supportFriction = this->supportFriction;

    distance = (motors[0] + motors[1]) >> 1;
    // solving for heading = heading + (motors[1] - motors[0]) * turningEffect
    headChange = FpsCoefficient2(FMul(motors[1] - motors[0], turningEffect));

    // probably to ignore rounding errors...
    if (headChange < 5 && headChange > -5)
        headChange = 0;

    avrgHeading = heading + (headChange >> 1);

    // in XZ plane, important to keep those indices straight when used with speed
    motorDir[0] = FMul(FOneSin(avrgHeading), distance);
    motorDir[1] = FMul(FOneCos(avrgHeading), distance);

    slide[0] = motorDir[0] - speed[0] + groundSlide[0];
    slide[1] = motorDir[1] - speed[2] + groundSlide[2];
    slideLen = VectorLength(2, slide);

    FPS_DEBUG("   motorDir = " << FormatVector(motorDir, 2) << std::endl);
    FPS_DEBUG("   speed = " << FormatVector(speed, 3) << std::endl);
    FPS_DEBUG("   groundSlide = " << FormatVector(groundSlide, 3) << std::endl);
    FPS_DEBUG("   slide = " << FormatVector(slide, 2) << std::endl);

    Fixed scale1, scale2;
    if (slideLen < supportTraction) {
        Fixed speedPortion = FIX(0.25);
        //   speed[] += (slide[] - (slide[] * speedPortion));
        // can be re-written as a function of speed[],
        //   speed[] = speed[] * speedPortion + (motorDir[] + groundSlide[]) * (1 - speedPortion);
        FpsCoefficients(speedPortion, FIX1-speedPortion, &scale1, &scale2);
    } else {
        //   speed[] += slide[] * supportFriction;
        // can be re-written as a function of speed[],
        //   speed[] = speed[] * (1-supportFriction) + (motorDir[] + groundSlide[]) * supportFriction;
        FpsCoefficients(FIX1 - supportFriction, supportFriction, &scale1, &scale2);
    }
    speed[0] = FMul(speed[0], scale1) + FMul(motorDir[0] + groundSlide[0], scale2);
    speed[2] = FMul(speed[2], scale1) + FMul(motorDir[1] + groundSlide[2], scale2);

    FPS_DEBUG("   headChange = " << headChange << ", supportTraction = " << supportTraction << "   slideLen = " << slideLen << std::endl);
    FPS_DEBUG("after slide speed = " << FormatVector(speed, 3) << std::endl);

    Fixed fric = FIX3(10); // FIX3(30);
    Fixed slowDown = FMul(fric, VectorLength(3, speed));
    slowDown = FIX1 - FpsCoefficient1(FIX1 - slowDown);
    speed[0] -= FMul(slowDown, speed[0]);
    speed[1] -= FMul(slowDown, speed[1]);
    speed[2] -= FMul(slowDown, speed[2]);

    heading += headChange;
    location[0] += FpsCoefficient2(speed[0]);
    location[1] += FpsCoefficient2((speed[1] + groundSlide[1]));
    location[2] += FpsCoefficient2(speed[2]);

    FPS_DEBUG("final location = " << FormatVector(location, 3) << ", heading = " << ToFloat(heading)*360 << ", speed = " << FormatVector(speed, 3) << std::endl);

    groundSlide[0] = 0;
    groundSlide[1] = 0;
    groundSlide[2] = 0;
}

void CAbstractPlayer::FrameAction() {
    if (!isOut) {
        CRealMovers::FrameAction();

        if (doIncarnateSound) {
            IncarnateSound();
        }

        // if a frag frame is specified with /dbg, force a frag on that frame by messing with FRandSeed
        int fragFrame = Debug::GetValue("frag");
        if (fragFrame > 0 && itsGame->frameNumber == fragFrame) {
            extern Fixed FRandSeed; // to intentionally cause frags below
            FRandSeed += 1;
        }
    }
}

void CAbstractPlayer::PlayerAction() {
    if (itsGame->frameNumber == 0 && itsManager->Presence() == kzSpectating) {
        lives = 0;
#define EXPLODING_SPECTATORS
#ifdef EXPLODING_SPECTATORS
        WasDestroyed();
#endif
        GoLimbo(0);  // hides the hector
    }
    if (lives) {
        itsGame->playersStanding++;
        // Send score updates to other players every 17 seconds worth of frames
        if ((itsGame->frameNumber & 255) == 255 && itsGame->playersStanding == 1 && itsManager->IsLocalPlayer()) {
            itsGame->scoreKeeper->NetResultsUpdate();
        }

        if ((teamMask & itsGame->teamsStandingMask) == 0) {
            itsGame->teamsStandingMask |= teamMask;
            itsGame->teamsStanding++;
        }
    }

    if (!isOut) {
        dirArrow->isTransparent = true;
        targetOns[0]->isTransparent = true; //  No HUD display by default
        targetOns[1]->isTransparent = true; //  So we hide all HUD parts now
        targetOffs[0]->isTransparent = true; //  And reveal them if necessary
        targetOffs[1]->isTransparent = true; // in PlaceHUDParts.
        ResetDashboard();

        if (isInLimbo) {
            if (!netDestruct)
                KeyboardControl(itsManager->GetFunctions());

            if (winFrame < 0) {
                if (limboCount-- <= 0) {
                    if (lives > 0) {
                        viewYaw = 0;
                        viewPitch = 0;
                        if (!scoutView) {
                            ResetCamera();
                        }
                        Reincarnate();
                    } else {
                        itsManager->DeadOrDone();

                        // Auto spectate another player if:
                        //   - The player runs out of lives
                        //   - The player being spectated runs out of lives
                        if ((itsGame->GetSpectatePlayer() == NULL && itsManager->IsLocalPlayer()) ||
                            (itsGame->GetSpectatePlayer() != NULL && itsGame->GetSpectatePlayer()->lives == 0)) {
                            itsGame->SpectateNext();
                        }
                    }
                }
            } else {
                if (winFrame >= 0)
                    WinAction();
            }
        } else {
            Fixed radius;

            oldYaw = viewYaw;
            oldPitch = viewPitch;

            fireGun = false;

            if (!netDestruct)
                KeyboardControl(itsManager->GetFunctions());

            UnlinkLocation();
            radius = proximityRadius + FDistanceOverEstimate(speed[0], speed[1], speed[2]);
            BuildPartProximityList(location, radius, kSolidBit);

            TractionControl();
            MotionControl();

            // FindBestMovement() will change speed if/when it intersects ground... need to make sure that accounts for fpsScale
            AvoidBumping();
            LinkPartSpheres();

            if (shields < maxShields) {
                Fixed regenRate;

                regenRate = FMulDivNZ(shieldRegen, energy, maxEnergy);

                if (boostEndFrame > itsGame->frameNumber)
                    shields += shieldRegen;

                shields += regenRate >> 3;
                if (shields > maxShields)
                    shields = maxShields;
                energy -= regenRate;
            }

            GunActions();

            if (itsGame->timeInSeconds < itsGame->loadedTimeLimit)
                energy += generatorPower;

            if (boostEndFrame > itsGame->frameNumber) {
                energy += 4 * generatorPower;
            }

            if (energy > maxEnergy)
                energy = maxEnergy;
            else if (energy < 0)
                energy = 0;
        }
    }

    if (teleportSoundLink->refCount == 1) {
        UpdateSoundLink(itsSoundLink, viewPortPart->itsTransform[3], speed, itsGame->soundTime);
    }
}

void CAbstractPlayer::GunActions() {
    short i;
    Matrix m1, m2;
    RayHitRecord theHit;
    CPlayerMissile *theMissile;
    Fixed charge;

    if (fireGun) {
        CWeapon *weapon = NULL;

        if (weaponIdent)
            weapon = (CWeapon *)itsGame->FindIdent(weaponIdent);

        if (weapon) {
            weapon->Fire();
            weaponIdent = 0;
        } else {
            i = gunEnergy[0] < gunEnergy[1];
            if (gunEnergy[i] >= activeGunEnergy) {
                Vector missileSpeed;

                OneMatrix(&m1);
                MTranslate(i ? gunOffset[0] : -gunOffset[0], gunOffset[1], gunOffset[2], &m1);
                CombineTransforms(&m1, &m2, &viewPortPart->itsTransform);
                MTranslate(speed[0], speed[1], speed[2], &m2);

                if (debug2Flag) {
                    theHit.direction[0] = FMul(m2[2][0], PLAYERMISSILESPEED) + speed[0];
                    theHit.direction[1] = FMul(m2[2][1], PLAYERMISSILESPEED) + speed[1];
                    theHit.direction[2] = FMul(m2[2][2], PLAYERMISSILESPEED) + speed[2];
                } else {
                    theHit.direction[0] = FMul(m2[2][0], PLAYERMISSILESPEED);
                    theHit.direction[1] = FMul(m2[2][1], PLAYERMISSILESPEED);
                    theHit.direction[2] = FMul(m2[2][2], PLAYERMISSILESPEED);
                }

                missileSpeed[0] = theHit.direction[0];
                missileSpeed[1] = theHit.direction[1];
                missileSpeed[2] = theHit.direction[2];

                theHit.direction[3] = 0;
                theHit.team = teamColor;
                theHit.playerId = GetActorScoringId();
                NormalizeVector(3, theHit.direction);

                theMissile = (CPlayerMissile *)itsGame->itsDepot->LaunchMissile(
                    kmiTurning, &m2, &theHit, this, gunEnergy[i], missileSpeed);

                gunEnergy[i] = 0;

                if (theMissile)
                    theMissile->SetSpin(i);
            }
        }
    }

    if (weaponIdent) {
        CWeapon *weapon = NULL;

        if (weaponIdent)
            weapon = (CWeapon *)itsGame->FindIdent(weaponIdent);

        if (weapon) {
            weapon->DoTargeting();
        } else {
            weaponIdent = 0;
        }
    }

    charge = FMulDivNZ(energy + generatorPower, chargeGunPerFrame,
                       maxEnergy);

    for (i = 0; i < 2; i++) {
        if (gunEnergy[i] < fullGunEnergy) {
            energy -= charge;
            if (gunEnergy[i] > activeGunEnergy) {
                gunEnergy[i] += FMul(charge, FIX3(850)); // Used to be: *2/3;
            } else {
                gunEnergy[i] += FMul(charge, FIX3(1050));
            }

            if (gunEnergy[i] > fullGunEnergy)
                gunEnergy[i] = fullGunEnergy;
        }
    }
}

short CAbstractPlayer::GetActorScoringId() {
    return itsManager ? itsManager->Slot() : -1;
}

void CAbstractPlayer::PostMortemBlast(short scoreTeam, short scoreColor, Boolean doDispose) {
    ActorAttachment *parasite;
    ActorAttachment *nextPara;

    CRealMovers::PostMortemBlast(scoreTeam, scoreColor, false);

    if (weaponIdent) {
        CWeapon *weapon;

        weapon = (CWeapon *)itsGame->FindIdent(weaponIdent);
        if (weapon) {
            weapon->doExplode = true;
            weapon->Locate();
        }
    }

    parasite = attachmentList;
    while (parasite) {
        nextPara = parasite->next;
        parasite->me->ReleaseAttachment();
        parasite = nextPara;
    }

    reEnergize = true;

    lives--;
    boostsRemaining = defaultConfig.numBoosters;
    missileCount = defaultConfig.numMissiles;
    grenadeCount = defaultConfig.numGrenades;
    GoLimbo(60);
    if (lives == 0 && itsManager->IsLocalPlayer()) {
        itsGame->itsApp->MessageLine(kmGameOver, MsgAlignment::Center);
    }

    itsGame->itsApp->DrawUserInfoPart(itsManager->Position(), kipDrawColorBox);
}

void CAbstractPlayer::GoLimbo(FrameNumber limboDelay) {
    CSmartPart **thePart;

    if (boostControlLink) {
        gHub->ReleaseLinkAndKillSounds(boostControlLink);
        boostControlLink = NULL;
    }
    limboCount = FpsFramesPerClassic(limboDelay);
    isInLimbo = true;
    maskBits &= ~kSolidBit;

    UnlinkLocation();

    for (thePart = partList; *thePart; thePart++) {
        (*thePart)->isTransparent = true;
    }
}

void CAbstractPlayer::IncarnateSound() {
    CBasicSound *aSound;
    SoundLink *aLink;

    doIncarnateSound = false;

    aSound = gHub->GetSoundSampler(hubRate, incarnateSound);
    aSound->SetVolume(incarnateVolume);
    aLink = gHub->GetSoundLink();
    PlaceSoundLink(aLink, location);
    aSound->SetSoundLink(aLink);
    aSound->Start();
    gHub->ReleaseLink(aLink);
}

void CAbstractPlayer::Reincarnate() {
    CIncarnator *placeList;
    long bestCount = LONG_MAX;

    // first, determine the count for the least-visited Incarnator
    for (placeList = itsGame->incarnatorList; placeList != nullptr; placeList = placeList->nextIncarnator) {
        if (placeList->enabled && (placeList->colorMask & teamMask) && (placeList->useCount < bestCount)) {
            bestCount = placeList->useCount;
        }
    }

    // try the least-visited Incarnators until one works
    for (placeList = itsGame->incarnatorList; placeList != nullptr; placeList = placeList->nextIncarnator) {
        if (placeList->enabled && (placeList->colorMask & teamMask) && (placeList->useCount == bestCount)) {
            if (itsManager->Presence() != kzSpectating && ReincarnateComplete(placeList)) {
                break;
            }
        }
    }

    // if couldn't find an available Incarnator above, try creating a random one
    if (placeList == nullptr) {
        // why 3 tries?  it's somewhat arbitrary but if there's a (high) 10% chance of not working,
        // then 3 tries will get that down to 0.1%.  In most levels, the not-working chance is probably
        // closer to 1% so 3 tries = 0.0001%
        for (int tries = 3; isInLimbo && tries > 0; tries--) {
            CRandomIncarnator waldo(itsGame->actorList);
            if (ReincarnateComplete(&waldo)) {
                break;
            }
        }
    }
}

bool CAbstractPlayer::ReincarnateComplete(CIncarnator* newSpot) {
    // increment useCount regardless of success, so the next player doesn't try to use this spot
    newSpot->useCount++;

    // make sure somebody or something isn't in this spot already, prepare for collision test
    location[0] = newSpot->location[0];
    location[1] = newSpot->location[1];
    location[2] = newSpot->location[2];
    speed[1] = 0;
    heading = newSpot->heading;

    // make player visible before the collision test
    for (CSmartPart **thePart = partList; *thePart; thePart++) {
        (*thePart)->isTransparent = false;
    }
    PlayerWasMoved();
    BuildPartProximityList(location, proximityRadius, kSolidBit);
    if (!DoCollisionTest(&proximityList.p)) {
        // looks like we're good to go
        isInLimbo = false;
        maskBits |= kSolidBit;

        LinkPartSpheres();

        if (reEnergize) {
            boostEndFrame = itsGame->FramesFromNow(MINIBOOSTTIME);
            reEnergize = false;
            if (shields < (maxShields >> 1))
                shields = maxShields >> 1;

            if (energy < (maxEnergy >> 1))
                energy = maxEnergy >> 1;
        }

        doIncarnateSound = true;
        motors[0] = 0;
        motors[1] = 0;
        speed[0] = 0;
        speed[1] = 0;
        speed[2] = 0;
        didSelfDestruct = false;

    } else {
        // make player invisible again since it failed the collision test
        for (CSmartPart **thePart = partList; *thePart; thePart++) {
            (*thePart)->isTransparent = true;
        }
        return false;
    }

    if (newSpot->colorMask != -1) {
        didIncarnateMasked = true;
    }

    return true;
}

Boolean CAbstractPlayer::TryTransport(Fixed *where, short soundId, Fixed volume, short options) {
    Vector oldLoc;
    Boolean couldMove;

    oldLoc[0] = location[0];
    location[0] = where[0];
    oldLoc[1] = location[1];
    location[1] = where[1];
    oldLoc[2] = location[2];
    location[2] = where[2];

#if 0
    sliverCounts[kSmallSliver] = 20;
    sliverCounts[kMediumSliver] = 20;
#endif
    PlayerWasMoved();
    LinkPartSpheres();
    BuildPartProximityList(location, proximityRadius, kSolidBit);

    couldMove = (DoCollisionTest(&proximityList.p) == NULL);
    location[0] = oldLoc[0];
    location[1] = oldLoc[1];
    location[2] = oldLoc[2];
    PlaceParts();

    if (couldMove) {
        Vector speedV;
        Fixed perFrames;
        CBasicSound *theSound;
        short count = 10;
        short life = 8;

        if (options & kFragmentOption) {
            location[0] = oldLoc[0];
            location[1] = oldLoc[1];
            location[2] = oldLoc[2];
            PlaceParts();
            Shatter(kSmallSliver, 1, &count, &life, FIX3(-300));

            location[0] = where[0];
            location[1] = where[1];
            location[2] = where[2];
            PlaceParts();
            Shatter(kSmallSliver, 1, &count, &life, FIX3(700));
        }

        theSound = gHub->GetSoundSampler(hubRate, soundId);
        theSound->SetVolume(volume);

        perFrames = theSound->sampleLen;
        perFrames -= perFrames >> 2;
        if (perFrames) {
            perFrames = FMulDivNZ(theSound->GetSampleRate(), itsGame->frameTime, perFrames) >> 10;
            speedV[0] = FMul(location[0] - oldLoc[0], perFrames);
            speedV[1] = 0; //   FMul(location[1] - oldLoc[1], perFrames);
            speedV[2] = FMul(location[2] - oldLoc[2], perFrames);

            UpdateSoundLink(itsSoundLink, oldLoc, speedV, itsGame->soundTime);
            theSound->SetSoundLink(itsSoundLink);
            theSound->SetControlLink(teleportSoundLink);
        }
        theSound->Start();

        if (options & kSpinOption) {
            // approximation for how much to scale the motors to acheive the same spin with High-FPS...
            // comes from taking the ratio of both geometric series limits of total spin for FPS and classic:
            //    scale = (F/(1-F)) / ((f/N)/(1-f))
            // F = classicMotorAcceleration, f = fpsMotorFriction, N = fps-frames per classic frame
            Fixed spinScale = (motorFriction == 0)
                ? FIX1
                : FDiv(FMul(classicMotorFriction, FIX1 - motorFriction),
                       FMul(FpsCoefficient2(motorFriction), FIX1 - classicMotorFriction));
            FPS_DEBUG("•••spinScale = " << ToFloat(spinScale) << "\n");
            motors[0] = FMul(maxAcceleration << 7, spinScale);
            motors[1] = -motors[0];
        } else {
            motors[0] = motors[1] = 0;
        }

        speed[0] = 0;
        speed[1] = 0;
        speed[2] = 0;
    } else {
        LinkPartSpheres();
    }

    return couldMove;
}

void CAbstractPlayer::ResumeLevel() {
    CRealMovers::ResumeLevel();  // will ultimately call AdaptableSettings above

    nextPlayer = itsGame->playerList;
    itsGame->playerList = this;

    chatMode = false;
}

extern Fixed sliverGravity;

#define INTERPTIME Fixed(FpsFramesPerClassic(20))

void CAbstractPlayer::Win(long winScore, CAbstractActor *teleport) {
    short count = 16;
    short life = INTERPTIME * 2;
    Matrix tempMatrix;
    Fixed saveGravity;
    CBasicSound *theSound;
    Vector delta;

    ReleaseAllAttachments();

    itsGame->scoreReason = ksiExitBonus;
    itsGame->Score(teamColor, GetActorScoringId(), winScore, 0, 0, 0);

    if (teleport->partList[0]) {
        Boolean transparentSave;

        transparentSave = teleport->partList[0]->isTransparent;
        teleport->partList[0]->isTransparent = false;
        VECTORCOPY(delta, teleport->partList[0]->itsTransform[3]);
        delta[0] = location[0] - delta[0];
        delta[1] = location[1] - delta[1];
        delta[2] = location[2] - delta[2];
        teleport->OffsetParts(delta);

        delta[0] = -delta[0];
        delta[1] = -delta[1];
        delta[2] = -delta[2];

        saveGravity = sliverGravity;
        sliverGravity = FIX3(20); //-saveGravity;
        teleport->Shatter(kSmallSliver, 1, &count, &life, FIX3(-100));

        //  count = 5;
        //  Shatter(kSmallSliver, 1, &count, &life, FIX3(-70));

        sliverGravity = saveGravity;
        teleport->OffsetParts(delta);
        teleport->partList[0]->isTransparent = transparentSave;
    }

    GoLimbo(1);
    winFrame = itsGame->frameNumber;

    theSound = gHub->GetSoundSampler(hubRate, winSound);
    theSound->SetVolume(winVolume);
    theSound->SetSoundLink(itsSoundLink);
    theSound->Start();

    MatrixToQuaternion(&viewPortPart->itsTransform, &winStart);

    OneMatrix(&tempMatrix);
    MRotateX(FOneSin(0x4000), FOneCos(0x4000), &tempMatrix);
    MRotateY(FOneSin(heading + 0x8000), FOneCos(heading + 0x8000), &tempMatrix);
    MatrixToQuaternion(&tempMatrix, &winEnd);

    speed[0] = 0;
    speed[1] = 0;
    speed[2] = 0;

    if (itsManager->IsLocalPlayer()) {
        itsGame->itsApp->MessageLine(kmWin, MsgAlignment::Center);
    }

    // itsGame->itsApp->DrawUserInfoPart(itsManager->slot, kipDrawColorBox);
}

void CAbstractPlayer::WinAction() {
    Fixed interFrame;
    Fixed inter2;

    interFrame = itsGame->frameNumber - winFrame;
    if (interFrame <= INTERPTIME) {
        Quaternion interQ;

        inter2 = FDiv(interFrame * interFrame, INTERPTIME * INTERPTIME);
        interQ.w = winStart.w + FMul(winEnd.w - winStart.w, inter2);
        interQ.x = winStart.x + FMul(winEnd.x - winStart.x, inter2);
        interQ.y = winStart.y + FMul(winEnd.y - winStart.y, inter2);
        interQ.z = winStart.z + FMul(winEnd.z - winStart.z, inter2);
        NormalizeVector(4, (Fixed *)&interQ);
        QuaternionToMatrix(&interQ, &viewPortPart->itsTransform);

        speed[1] = FMul(inter2, FIX3(800));
        viewPortPart->itsTransform[3][1] += speed[1];
        viewPortPart->invGlobDone = false;
    } else {
        if (interFrame <= INTERPTIME * 2) {
            interFrame = 2 * INTERPTIME - interFrame;
            inter2 = FDiv(interFrame * interFrame, INTERPTIME * INTERPTIME);
            speed[1] = FMul(inter2, FIX3(800));
            viewPortPart->itsTransform[3][1] += speed[1];
            viewPortPart->invGlobDone = false;
        } else {
            if (interFrame == INTERPTIME * 4) {
                itsManager->DeadOrDone();
            }
            speed[1] = 0;
        }
    }
}

/*
void    CAbstractPlayer::FillGameResultRecord(
    TaggedGameResult    *res)
{
    if(winFrame >= 0)
    {   res->r.time = FMulDivNZ(winFrame, itsGame->frameTime, 10);
    }
    else
    {   res->r.time = -1;
    }

    res->r.score = itsGame->scores[itsManager->slot];
}
*/

void CAbstractPlayer::ReceiveConfig(PlayerConfigRecord *config) {
    if (itsGame->frameNumber == 0) {
        defaultConfig = *config;

        if (defaultConfig.numBoosters > boosterLimit)
            defaultConfig.numBoosters = boosterLimit;

        if (defaultConfig.numMissiles > missileLimit)
            defaultConfig.numMissiles = missileLimit;

        if (defaultConfig.numGrenades > grenadeLimit)
            defaultConfig.numGrenades = grenadeLimit;

        if (boostsRemaining > defaultConfig.numBoosters)
            boostsRemaining = defaultConfig.numBoosters;

        if (missileCount > defaultConfig.numMissiles)
            missileCount = defaultConfig.numMissiles;

        if (grenadeCount > defaultConfig.numGrenades)
            grenadeCount = defaultConfig.numGrenades;

        // Reload the livesLabel to reflect the hull shape.
        if (!itsGame->showNewHUD) return;
        CBSPWorld *hudWorld;
        hudWorld = itsGame->hudWorld;
        hudWorld->RemovePart(livesLabel);
        delete livesLabel;
        livesLabel = DashboardPart(hullConfig.hullBSP, FIX3(140*layoutScale));
    }
}

Fixed CAbstractPlayer::GetTotalMass() {
    return (FIX(boostsRemaining) << 2) + FIX(grenadeCount) + FIX(missileCount) + CRealMovers::GetTotalMass();
}

void CAbstractPlayer::PlayerWasMoved() {
    PlaceParts();
}

void CAbstractPlayer::TakeGoody(GoodyRecord *gr) {
    grenadeCount += gr->grenades;
    if (grenadeCount > grenadeLimit)
        grenadeCount = grenadeLimit;

    missileCount += gr->missiles;
    if (missileCount > missileLimit)
        missileCount = missileLimit;

    boostsRemaining += gr->boosters;
    if (boostsRemaining > boosterLimit)
        boostsRemaining = boosterLimit;
    lives += gr->lives;
    if (gr->lives) {
        itsGame->itsApp->DrawUserInfoPart(itsManager->Position(), kipDrawColorBox);
    }

    shields += gr->shield;
    if (shields > maxShields)
        shields = maxShields;

    energy += gr->power;
    if (energy > maxEnergy)
        energy = maxEnergy;

    if (gr->boostTime > 0 && (boostEndFrame < itsGame->frameNumber)) {
        CBasicSound *theSound;

        boostEndFrame = itsGame->FramesFromNow(gr->boostTime);

        if (!boostControlLink)
            boostControlLink = gHub->GetSoundLink();

        boostControlLink->meta = metaNoData;
        theSound = gHub->GetSoundSampler(hubRate, 413);
        theSound->SetVolume(FIX(2));
        theSound->SetSoundLink(itsSoundLink);
        theSound->SetControlLink(boostControlLink);
        theSound->SetSoundLength((gr->boostTime * CLASSICFRAMETIME) << 6);
        theSound->Start();
    }
}

short CAbstractPlayer::GetPlayerPosition() {
    return itsManager->Position();
}

short CAbstractPlayer::GetBallSnapPoint(long theGroup,
    Fixed *ballLocation,
    Fixed *snapDest,
    Fixed *delta,
    CSmartPart **hostPart) {
    Fixed distanceEstimate;

    if (!weaponIdent) {
        Fixed globDelta[3];

        delta[0] = 0;
        delta[1] = FIX3(-200);
        delta[2] = FIX3(1200);
        delta[3] = FIX1;

        *hostPart = viewPortPart;
        VectorMatrixProduct(1, (Vector *)delta, (Vector *)snapDest, &viewPortPart->itsTransform);

        globDelta[0] = ballLocation[0] - snapDest[0];
        globDelta[1] = ballLocation[1] - snapDest[1];
        globDelta[2] = ballLocation[2] - snapDest[2];

        distanceEstimate = FDistanceEstimate(globDelta[0], globDelta[1], globDelta[2]);

        if (distanceEstimate < FIX3(1200)) {
            if (distanceEstimate < FIX3(300))
                return kSnapHere;
            else
                return kSnapAttract;
        }
    }

    return kSnapNot;
}

void CAbstractPlayer::WasHit(RayHitRecord *theHit, Fixed hitEnergy) {
    SignalAttachments(kHostDamage, &hitEnergy);

    CRealMovers::WasHit(theHit, hitEnergy);
}
