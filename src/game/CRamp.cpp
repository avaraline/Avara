/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CRamp.c
    Created: Wednesday, January 24, 1996, 05:34
    Modified: Tuesday, September 10, 1996, 17:58
*/

#include "CRamp.h"

#include "CSmartBox.h"

extern Rect gLastBoxRect;
extern Fixed gLastBoxRounding;

static void SolveOrientation(Fixed x, Fixed y, Fixed h, Fixed *w, Fixed *angle) {
    Fixed guess;
    Fixed minGuess, maxGuess;
    short i;
    Fixed result;

    minGuess = -0x3C00;
    maxGuess = 0x3C00;

    for (i = 0; i < 20; i++) {
        Fixed sinGuess, cosGuess;

        guess = (minGuess + maxGuess) >> 1;
        sinGuess = FOneSin(guess);
        cosGuess = FOneCos(guess);

        *w = FDiv(x - FMul(h, sinGuess), cosGuess);
        result = FMul(*w, sinGuess) + FMul(h, cosGuess);

        if (result > y)
            maxGuess = guess;
        else
            minGuess = guess;
    }

    *angle = (minGuess + maxGuess) >> 1; //	guess;
}

void CRamp::BeginScript() {
    CPlacedActors::BeginScript();

    ProgramLongVar(iDeltaY, 1);
    ProgramFixedVar(iThickness, gLastBoxRounding);
    //ProgramOffsetMultiply(iThickness, iPixelToThickness, gLastBoxRounding);
}

CAbstractActor *CRamp::EndScript() {
    if (CPlacedActors::EndScript()) {
        Fixed deltaY;
        //Fixed dimX, dimZ;
        Fixed angle;
        Vector dims;
        CSmartBox *box;
        short resId;

        maskBits |= kSolidBit + kDoorIgnoreBit;

        deltaY = ReadFixedVar(iDeltaY);
        dims[0] = FMulDivNZ(gLastBoxRect.right - gLastBoxRect.left, FIX(5), 72);
        dims[1] = ReadFixedVar(iThickness);
        dims[2] = FMulDivNZ(gLastBoxRect.bottom - gLastBoxRect.top, FIX(5), 72);

        if (dims[0] > LOCATORRECTSIZE)
            dims[0] = LOCATORRECTSIZE;
        if (dims[2] > LOCATORRECTSIZE)
            dims[2] = LOCATORRECTSIZE;

        if (deltaY < 0) {
            deltaY = -deltaY;
            location[1] -= deltaY;
            heading += 0x8000;
        }

        location[0] = FMulDivNZ(gLastBoxRect.right + gLastBoxRect.left, FIX(5), 144);
        location[2] = FMulDivNZ(gLastBoxRect.bottom + gLastBoxRect.top, FIX(5), 144);

        partCount = 1;

        if ((heading - 0x2000) & 0x4000) {
            SolveOrientation(dims[2], deltaY, dims[1], &dims[2], &angle);
            dims[2] += dims[2] >> 9;
            dims[0] >>= 1;
            dims[1] >>= 1;
            dims[2] >>= 1;

            resId = ReadLongVar(dims[1] == 0 ? iFloorTemplateResource : iWallTemplateResource);

            box = CSmartBox::Create(resId, dims, GetPixelColor(), GetOtherPixelColor(), this, 0);
            box->Reset();
            InitialRotatePartX(box, ((heading - 0x2000) & 0x8000) ? angle : -angle);
            TranslatePart(box, location[0], location[1] + (deltaY >> 1), location[2]);
            box->MoveDone();
        } else {
            SolveOrientation(dims[0], deltaY, dims[1], &dims[0], &angle);
            dims[0] += dims[0] >> 9;
            dims[0] >>= 1;
            dims[1] >>= 1;
            dims[2] >>= 1;

            resId = ReadLongVar(dims[1] == 0 ? iFloorTemplateResource : iWallTemplateResource);

            box = CSmartBox::Create(resId, dims, GetPixelColor(), GetOtherPixelColor(), this, 0);
            box->Reset();
            InitialRotatePartZ(box, ((heading - 0x2000) & 0x8000) ? angle : -angle);
            TranslatePart(box, location[0], location[1] + (deltaY >> 1), location[2]);
            box->MoveDone();
        }

        if (partYon != 0 && box) {
            box->usesPrivateYon = true;
            box->yon = partYon;
        }

        partList[0] = box;
        LinkPartBoxes();

        return this;
    } else {
        return NULL;
    }
}
