/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: LevelLoader.h
    Created: Sunday, November 20, 1994, 19:48
    Modified: Tuesday, September 3, 1996, 21:44
*/

#pragma once
#include "Types.h"
#include <stdint.h>
#include <string>
#include <vector>

#include "ARGBColor.h"
#include "ColorManager.h"
#include "Material.h"
#include "Parser.h"

class Material;

enum {
    iDesignerName,
    iLevelInformation,
    iTimeLimit,

    iGravity,
    iCustomGravity,

    iShape,
    iAltShape,
    iScale,
    iDefaultYon,
    iYon,
    iWallYon,
    iHasFloor,

    iWallTemplateResource,
    iFloorTemplateResource,

    iHeight,
    iMask,
    iTeam,
    iWallHeight,
    iWallAltitude,
    iBaseHeight,
    iPixelToThickness,
    iMass,
    iVisible,
    iThickness,
    iWinTeam,

    iTargetGroup,

    //	Sounds:
    iHitSound,
    iHitVolume,
    iHitSoundDefault,
    iShieldHitSoundDefault,
    iPlayerHitSoundDefault,
    iBlastSound,
    iBlastVolume,
    iBlastSoundDefault,

    iStepSound,
    iGroundStepSound,

    iIsTarget,

    iSound,
    iOpenSound,
    iCloseSound,
    iStopSound,
    iVolume,

    iDefaultLives,
    iLives,

    iIncarnateSound,
    iIncarnateVolume,

    iWinSound,
    iWinVolume,

    iLoseSound,
    iLoseVolume,

    //	Scoring
    iDestructScore,
    iHitScore,
    iFriendlyHitMultiplier,

    //	Explosions and damage:
    iSlivers0,
    iSlivers1,
    iSlivers2,
    iSliverL0,
    iSliverL1,
    iSliverL2,
    iCanGlow,

    //	Messages sent when hit/destroyed.
    iDestructMessage,
    iHitMessage,

    //	Other messages.
    iStepOnMessage,

    //	Motion
    iAccelerate,

    //	Toggle switches
    iTogglePower,
    iBlastToggle,

    //	Doors & such
    iOpenMsg,
    iCloseMsg,
    iDidOpen,
    iDidClose,
    iOpenDelay,
    iCloseDelay,
    iGuardDelay,
    iStatus,
    iOpenSpeed,
    iCloseSpeed,

    iPitch,
    iYaw,
    iRoll,
    iDeltaX,
    iDeltaY,
    iDeltaZ,

    //	Door2 specific:
    iMidState,
    iMidPitch,
    iMidYaw,
    iMidRoll,
    iMidX,
    iMidY,
    iMidZ,

    // Misc
    iPower,
    iMaxPower,
    iDrain,

    //	Guns
    iFireMsg,
    iTrackMsg,
    iStopTrackMsg,
    iSpeed,
    iShotPower,

    //  Ufos
    iCheckPeriod,
    iAttack,
    iDefense,
    iVisionScore,
    iHideScore,
    iMotionRange,
    iPitchRange,
    iVerticalRangeMin,
    iVerticalRangeMax,
    iBurstLength,
    iBurstSpeed,
    iBurstCharge,

    iHomeSick,
    iHomeRange,
    iHomeBase,

    //	Mines
    iShieldEnergy,
    iActivateEnergy,
    iRange,
    iPhase,
    iActiveTimer,
    iBoom,

    iIdleShapeTimer,
    iIdleAltShapeTimer,
    iActiveShapeTimer,
    iActiveAltShapeTimer,

    iActivateSound,
    iActivateVolume,

    //	Teleporters:
    iGroup,
    iDestGroup,
    iSpinFlag,
    iFragmentFlag,
    iWin,
    iDeadRange,
    iShowAlways,

    //	Activators: (areas, etc.)
    iWatchMask,
    iFrequency,
    iEnter,
    iExit,

    //	Text
    iText,
    iShowEveryone,
    iAlignment,

    //	Goody
    iGrenades,
    iMissiles,
    iBoosters,
    iBoostTime,

    //	Ball & goal
    iGoalMsg,
    iGoalAction,
    iGoalScore,

    iEjectPitch,
    iEjectPower,
    iEjectSound,
    iEjectVolume,

    iShieldChargeRate,
    iMaxShield,
    iShootShield,
    iGrabShield,

    iCarryScore,
    iDropEnergy,
    iChangeHolderPower,
    iChangeOwnerTime,

    iChangeOwnerSound,
    iChangeOwnerVolume,

    iSnapSound,
    iSnapVolume,

    //	Sound objects:
    iIsMusic,
    iIsAmbient,
    iIsPlaced,
    iRate,
    iLoopCount,
    iVolume0,
    iVolume1,
    iStartMsg,
    iStopMsg,
    iKillMsg,

    //	Logic:
    iInVar,
    iFirstIn,
    iLastIn = iFirstIn + 9,

    iOutVar,
    iFirstOut,
    iLastOut = iFirstOut + 9,

    iRestartFlag,
    iCount,
    iTimer,

    //	Lights
    iAmbient,
    iAmbientColor,
    iLightsTable,
    iLightsTableEnd = iLightsTable + 19, //	a 4 x 5 table

    //  Materials
    iDefaultMaterialSpecular,
    iDefaultMaterialShininess,
    iBaseMaterialSpecular,
    iBaseMaterialShininess,

    // Advanced weapons
    iGrenadePower,
    iMissilePower,
    iMissileTurnRate,
    iMissileAcceleration,
    iMissileArmedColor,
    iMissileLaunchedColor,

    iMaxStartGrenades,
    iMaxStartMissiles,
    iMaxStartBoosts,

    //	Hulls
    iHull01,
    iHull02,
    iHull03,
    iHull04,
    iHull05,
    iHull06,
    iHull07,
    iHull08,
    iHull09,
    iHull10,
    iHullName01,
    iHullName02,
    iHullName03,
    iHullName04,
    iHullName05,
    iHullName06,
    iHullName07,
    iHullName08,
    iHullName09,
    iHullName10,

    //	Traction/friction control:
    iDefaultTraction,
    iDefaultFriction,

    iWallTraction,
    iWallFriction,

    iTraction,
    iFriction,

    iWallShields,
    iWallPower,

    iVarInternalVariableCount
};

enum {
    TextBegin = 150, //	6	TTxtPicRec	Begin text function
    TextEnd = 151, //	0	NIL	End text function
    StringBegin = 152, //	0	NIL	Begin string delimitation
    StringEnd = 153 //	0	NIL	End string delimitation
};

bool LoadALF(std::string levelPath);
void GetLastArcLocation(Fixed *theLoc);
Fixed GetLastArcDirection();
Fixed GetDome(Fixed *theLoc, Fixed *startAngle, Fixed *spanAngle);
Material GetDefaultMaterial();
Material GetBaseMaterial();
Material GetPixelMaterial();
Material GetOtherPixelMaterial();
Material GetTertiaryMaterial();
Material GetQuaternaryMaterial();
Fixed GetLastOval(Fixed *theLoc);
