/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: LinkLoose.c
    Created: Tuesday, April 4, 1995, 00:47
    Modified: Tuesday, August 27, 1996, 04:33
*/

#include "LinkLoose.h"

#include "CAreaActor.h"
#include "CBall.h"
#include "CDome.h"
#include "CDoor2Actor.h"
#include "CDoorActor.h"
#include "CForceField.h"
#include "CFreeSolid.h"
#include "CGoal.h"
#include "CGoody.h"
#include "CGroundColorAdjuster.h"
#include "CGuardActor.h"
#include "CHologramActor.h"
#include "CIncarnator.h"
#include "CLogicAnd.h"
#include "CLogicBase.h"
#include "CLogicCounter.h"
#include "CLogicDelay.h"
#include "CLogicDistributor.h"
#include "CLogicTimer.h"
#include "CMineActor.h"
#include "CParasite.h"
#include "CPill.h"
#include "CRamp.h"
#include "CSkyColorAdjuster.h"
#include "CSolidActor.h"
#include "CSoundActor.h"
#include "CSphereActor.h"
#include "CSwitchActor.h"
#include "CTeleporter.h"
#include "CTextActor.h"
#include "CTriPyramidActor.h"
#include "CUfo.h"
#include "CWalkerActor.h"
#include "CWallDoor.h"
#include "CWallSolid.h"
#include "CYonBox.h"
#include "CYonSphere.h"

#include <SDL2/SDL.h>

#define LOOSESTRLIST 1024

static Handle objectNamesHandle = NULL;

enum {
    koNoObject = 0,
    koWalker,
    koSphere,
    koHologram,
    koSolid,
    koWallSolid,
    koFreeSolid,
    koDome,
    koRamp,
    koTriPyramid,
    koDoor,
    koDoor2,
    koWallDoor,
    koArea,
    koText,
    koForceField,
    koGoody,
    koSwitch,
    koGuard,
    koGroundColor,
    koSkyColor,
    koIncarnator,
    koTeleporter,
    koPill,
    koBall,
    koGoal,
    koMine,
    koParasite,
    koUfo,
    koSoundActor,

    koLogicTimer,
    koLogicDelay,
    koLogicCounter,
    koLogicAnd,
    koLogicDistributor,
    koLogicBase,

    koYonBox,
    koYonSphere,

    koLastObject
};

void *CreateObjectByIndex(short objectId) {
    switch (objectId) {
        case koWalker:
            return new CWalkerActor;
        case koSphere:
            return new CSphereActor;
        case koHologram:
            return new CHologramActor;
        case koSolid:
            return new CSolidActor;
        case koWallSolid:
            return new CWallSolid;
        case koFreeSolid:
            return new CFreeSolid;
        case koDome:
            return new CDome;
        case koRamp:
            return new CRamp;
        case koTriPyramid:
            return new CTriPyramidActor;
        case koDoor:
            return new CDoorActor;
        case koDoor2:
            return new CDoor2Actor;
        case koWallDoor:
            return new CWallDoor;
        case koArea:
            return new CAreaActor;
        case koText:
            return new CTextActor;
        case koForceField:
            return new CForceField;
        case koGoody:
            return new CGoody;
        case koSwitch:
            return new CSwitchActor;
        case koGuard:
            return new CGuardActor;
        case koGroundColor:
            return new CGroundColorAdjuster;
        case koSkyColor:
            return new CSkyColorAdjuster;
        case koIncarnator:
            return new CIncarnator;
        case koTeleporter:
            return new CTeleporter;
        case koPill:
            return new CPill;
        case koBall:
            return new CBall;
        case koGoal:
            return new CGoal;
        case koMine:
            return new CMineActor;
        case koParasite:
            return new CParasite;
        case koUfo:
            return new CUfo;
        case koSoundActor:
            return new CSoundActor;

        //	Logic objects:
        case koLogicTimer:
            return new CLogicTimer;
        case koLogicDelay:
            return new CLogicDelay;
        case koLogicAnd:
            return new CLogicAnd;
        case koLogicCounter:
            return new CLogicCounter;
        case koLogicDistributor:
            return new CLogicDistributor;
        case koLogicBase:
            return new CLogicBase;

        //	Speed optimizations
        case koYonBox:
            return new CYonBox;
        case koYonSphere:
            return new CYonSphere;

        default:
            SDL_Log("UNKNOWN OBJECT TYPE in CreateObjectByIndex(%d)\n", objectId);
            return NULL;
    }
}

void InitLinkLoose() {
    objectNamesHandle = GetResource('STR#', LOOSESTRLIST);
}

void *CreateNamedObject(StringPtr theName) {
    StringPtr string;
    StringPtr charp;
    short tokCount;
    short ind;
    short i;
    short stringLen;

    if (objectNamesHandle == NULL)
        InitLinkLoose();

    stringLen = *theName++;
    tokCount = ntohs(*(short *)(*objectNamesHandle));
    string = ((StringPtr)*objectNamesHandle) + 2;

    for (ind = 1; ind <= tokCount; ind++) {
        if (string[0] == stringLen) {
            charp = string + 1;

            for (i = 0; i < stringLen; i++) {
                if (*charp++ != theName[i]) {
                    break;
                }
            }

            if (i == stringLen) {
                break;
            }
        }

        string += string[0] + 1;
    }

    return CreateObjectByIndex(ind);
}
