/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CWallActor.c
    Created: Sunday, November 20, 1994, 19:43
    Modified: Sunday, September 1, 1996, 20:36
*/

#include "CWallActor.h"

#include "CSmartBox.h"
#include "Parser.h"

#define WALLBASERESINDEX 400
CWallActor *lastWallActor = 0;

#define kNorthWall 1
#define kSouthWall 2
#define kEastWall 4
#define kWestWall 8

void CWallActor::MakeWallFromRect(Rect *theRect, Fixed height, short decimateWalls, Boolean isOrigWall) {
    Boolean tooBig;
    Fixed centerX, centerZ;
    Vector dim;
    Fixed addAlt;
    CSmartBox *box;
    short resId;

    maskBits |= kSolidBit + kDoorIgnoreBit;

    if (isOrigWall)
        FreshCalc();

    addAlt = ReadFixedVar(iBaseHeight) + ReadFixedVar(iWallAltitude);
    partYon = ReadFixedVar(iWallYon);

    do {
        dim[0] = FDivNZ(theRect->right - theRect->left, 72);
        dim[1] = 0;
        dim[2] = FDivNZ(theRect->bottom - theRect->top, 72);

        tooBig =
            dim[0] > LOCATORRECTSIZE / 5 || dim[2] > LOCATORRECTSIZE / 5; // VectorLength(3, dim) > LOCATORRECTSIZE/5;
        if (tooBig) {
            CWallActor *otherWall;
            Rect smallRect;
            short newDecim;

            smallRect = *theRect;
            if (smallRect.right - smallRect.left > smallRect.bottom - smallRect.top) {
                smallRect.left += (smallRect.right - smallRect.left) >> 1;
                theRect->right = smallRect.left;
                newDecim = decimateWalls | kEastWall;
                decimateWalls |= kWestWall;
            } else {
                smallRect.top += (smallRect.bottom - smallRect.top) >> 1;
                theRect->bottom = smallRect.top;
                newDecim = decimateWalls | kNorthWall;
                decimateWalls |= kSouthWall;
            }

            otherWall = new CWallActor;
            otherWall->MakeWallFromRect(&smallRect, height, newDecim, false);
        }
    } while (tooBig);

    centerX = FMulDivNZ(theRect->right + theRect->left, FIX(5), 144);
    centerZ = FMulDivNZ(theRect->bottom + theRect->top, FIX(5), 144);

    dim[0] = dim[0] * 5 / 2;
    dim[2] = dim[2] * 5 / 2;
    if (height) {
        dim[1] = (1 + height) >> 1;
        //dim[1] = (1 + height * ReadFixedVar(iPixelToThickness)) >> 1;
    } else {
        dim[1] = ReadFixedVar(iWallHeight) >> 1;
    }

    resId = ReadLongVar(dim[1] == 0 ? iFloorTemplateResource : iWallTemplateResource);

    partCount = 1;
    box = new CSmartBox(resId, dim, GetPixelColor(), GetOtherPixelColor(), this, 0);
    box->Reset();
    TranslatePart(box, centerX, addAlt + dim[1], centerZ);
    box->MoveDone();
    if (partYon) {
        box->yon = partYon;
        box->usesPrivateYon = true;
    }

    partList[0] = box;

    if (partList[0]) {
        if (isOrigWall) {
            if (addAlt != 0) {
                ProgramLongVar(iWallAltitude, 0);
            }
        }
        LinkPartBoxes();

        shields = ReadFixedVar(iWallShields);
        blastPower = ReadFixedVar(iWallPower);

        traction = ReadFixedVar(iWallTraction);
        friction = ReadFixedVar(iWallFriction);

        hitSoundId = ReadLongVar(iHitSoundDefault);
        hitSoundVolume = DEFAULTHITVOLUME;
        itsGame = gCurrentGame;
        itsGame->AddActor(this);

        stepSound = ReadLongVar(iStepSound);

        // Preload sounds.
        auto _ = AssetManager::GetOgg(stepSound);

        lastWallActor = this;

        /*
        if(shields < 0 && decimateWalls)
        {	Handle		decimator;
            short		*p;

            decimator = GetResource('BSPD', resId);
            if(decimator)
            {	short		dir;
                PolyRecord	*polyTable;
                UniquePoint	*normalTable;

                polyTable = (PolyRecord *)((*box->itsBSPResource)+box->header.polyOffset);

                p = *(short **)decimator;
                while(dir = *p++)
                {	if(dir & decimateWalls)
                    {	polyTable[*p++].visibility = 0;
                    }
                    else
                    {	p++;
                    }
                }
            }
        }
        */
    } else {
        Dispose();
    }
}
