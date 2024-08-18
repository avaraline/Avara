/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CBSPWorld.c
    Created: Saturday, July 16, 1994, 23:14
    Modified: Monday, August 12, 1996, 19:51
*/

#include "CBSPWorld.h"

#include "AbstractRenderer.h"
#include "CBSPPart.h"
#include "CViewParameters.h"
#include "Memory.h"

extern Vector **bspPointTemp;
extern short *bspIndexStack;

CBSPWorldImpl::CBSPWorldImpl(short initialObjectSpace) {
    partSpace = initialObjectSpace;
    partCount = 0;

    partList = (CBSPPart ***)NewHandle(sizeof(CBSPPart *) * (long)partSpace);
    visibleList = (CBSPPart ***)NewHandle(sizeof(CBSPPart *) * (long)partSpace);
}

CBSPWorldImpl::~CBSPWorldImpl() {
    DisposeParts();

    DisposeHandle((Handle)partList);
    DisposeHandle((Handle)visibleList);
}

void CBSPWorldImpl::AddPart(CBSPPart *thePart) {
    if (partSpace <= partCount) {
        partSpace = partCount + (partCount >> 3) + 1;

        SetHandleSize((Handle)partList, sizeof(CBSPPart *) * (long)partSpace);
        SetHandleSize((Handle)visibleList, sizeof(CBSPPart *) * (long)partSpace);
    }

    thePart->worldIndex = partCount;
    (*partList)[partCount++] = thePart;
}

void CBSPWorldImpl::DisposeParts() {
    short i;

    for (i = 0; i < partCount; i++) {
        delete (*partList)[i];
    }

    partCount = 0;
}

CBSPPart *CBSPWorldImpl::GetIndPart(uint16_t ind) {
    if (ind >= 0 && ind < partCount)
        return (*partList)[ind];
    else
        return NULL;
}

uint16_t CBSPWorldImpl::GetPartCount() {
    return partCount;
}

uint16_t CBSPWorldImpl::GetVisiblePartCount() {
    return visibleCount;
}

CBSPPart **CBSPWorldImpl::GetVisiblePartListPointer() {
    return visibleP;
}

void CBSPWorldImpl::OverheadPoint(Fixed *c, Fixed *e) {
    Fixed minX = FIX(9999),
          maxX = FIX(-9999),
          minZ = FIX(9999),
          maxZ = FIX(-9999),
          maxY = 0;
    for (int i = 0; i < partCount; i++) {
        Fixed *t = (*partList)[i]->itsTransform[3];
        minX = t[0] < minX ? t[0] : minX;
        maxX = t[0] > maxX ? t[0] : maxX;
        minZ = t[2] < minZ ? t[2] : minZ;
        maxZ = t[2] > maxZ ? t[2] : maxZ;
        maxY = t[1] > maxY ? t[1] : maxY;
    }
    c[0] = FDiv(minX + maxX, FIX(2));
    c[1] = maxY;
    c[2] = FDiv(minZ + maxZ, FIX(2));
    e[0] = minX;
    e[1] = maxX;
    e[2] = 0;
    e[3] = maxY;
    e[4] = minZ;
    e[5] = maxZ;
}

void CBSPWorldImpl::PrepareForRender() {
    int i;
    CBSPPart **sp, **sd;

    gRenderer->viewParams->DoLighting();

    visibleP = *visibleList;
    sd = visibleP;
    sp = *partList;
    visibleCount = 0;

    for (i = 0; i < partCount; i++) {
        if ((*sp)->PrepareForRender()) {
            *sd++ = *sp;
            visibleCount++;
        }

        sp++;
    }

    SortVisibleParts();
}

void CBSPWorldImpl::RemovePart(CBSPPart *thePart) {
    short i;
    CBSPPart **p;

    p = *partList;

    i = thePart->worldIndex;
    if (i >= 0 && i < partCount && p[i] == thePart) {
        partCount--;
        p[i] = p[partCount];
    } else {
        for (i = 0; i < partCount; i++) {
            if (*p == thePart) {
                partCount--;
                p[0] = (*partList)[partCount];
                break;
            } else
                p++;
        }
    }
}

/*
**	This is the old version. It doesn't work correctly in all cases.
*/
void CBSPWorldImpl::ScoreAndSort(CBSPPart **firstPart, short overlapCount) {
    CBSPPart *compPart;
    CBSPPart **endPart, **jPart, **iPart;

    endPart = firstPart + overlapCount;

    iPart = firstPart;

    while (iPart < endPart) {
        (*iPart++)->worldFlag = false;
    }

    for (iPart = firstPart; iPart < endPart; iPart++) {
        compPart = *iPart;

        if (!compPart->worldFlag)
            for (jPart = iPart + 1; jPart < endPart; jPart++) {
                if (compPart->Obscures(*jPart)) {
                    CBSPPart **kPart;

                    compPart = *jPart;
                    kPart = jPart;
                    while (kPart > iPart) {
                        kPart[0] = kPart[-1];
                        kPart--;
                    }

                    *iPart = compPart;
                    compPart->worldFlag = true;
                }
            }
    }
}

void CBSPWorldImpl::SortByZ() {
    short h;
    CBSPPart **ip, **jp, *vp;
    CBSPPart **lastP;
    CBSPPart **hp;
    Fixed vz;

    h = 1;
    lastP = visibleP + visibleCount;

    do {
        h += h + h + 1;
    } while (h <= visibleCount);

    do {
        h /= 3;
        hp = visibleP + h;

        for (ip = hp; ip < lastP; ip++) {
            vp = *ip;
            vz = vp->maxZ;

            jp = ip;

            while ((jp[-h])->maxZ < vz) {
                *jp = jp[-h];
                jp -= h;

                if (jp < hp)
                    break;
            }

            *jp = vp;
        }
    } while (h != 1);
}

void CBSPWorldImpl::SortVisibleParts() {
    CBSPPart **thisPart, **endPart;
    Fixed minZ, maxZ;
    short overlapCount = 0;

    //SortByZ();

    endPart = visibleP + visibleCount;
    thisPart = visibleP;
    minZ = maxZ = gRenderer->viewParams->yonBound;

    while (thisPart < endPart) {
        Fixed z;

        z = (*thisPart)->maxZ;

        if (z > minZ) {
            overlapCount++;
            z = (*thisPart)->minZ;
            if (z < minZ)
                minZ = z;
        } else {
            if (overlapCount > 1) {
                VisibilitySort(thisPart - overlapCount, overlapCount);
            }
            maxZ = z;
            minZ = (*thisPart)->minZ;
            overlapCount = 1;
        }

        thisPart++;
    }

    if (overlapCount > 1) {
        VisibilitySort(thisPart - overlapCount, overlapCount);
    }
}

/*
**	Visibility sort "overlapCount" objects. The number of objects
**	is guaranteed to be at least 2.
*/

#define FASTOBSCURETEST(a, b) (b->maxZ > a->minZ)
//static long totalObsCount = 0;

void CBSPWorldImpl::VisibilitySort(CBSPPart **parts, short overlapCount) {
    CBSPPart *thePart;
    CBSPPart *listStart;
    CBSPPart **prevLink;

    listStart = NULL;

    do {
        thePart = *parts++;
        thePart->worldFlag = true;
        thePart->nextTemp = listStart;

        listStart = thePart;
    } while (--overlapCount);

    do {
        Fixed maxZ = 0;
    newListStart:
        maxZ = listStart->maxZ;
        prevLink = &listStart->nextTemp;
        thePart = *prevLink;

        while (thePart) { //	The clean way would be to call the Obscures method and do all the work
            //	there. Unfortunately this is very costly because the stack is involved...
            if (maxZ > thePart->minZ && thePart->worldFlag && listStart->maxX > thePart->minX &&
                listStart->minX < thePart->maxX && listStart->maxY > thePart->minY && listStart->minY < thePart->maxY &&
                thePart->Obscures(listStart)) {
                *prevLink = thePart->nextTemp; //	Unlink current part
                thePart->nextTemp = listStart;
                listStart->worldFlag = false;
                listStart = thePart;
                goto newListStart;
            } else {
                prevLink = &thePart->nextTemp;
                thePart = *prevLink;
            }
        }

        *--parts = listStart;
        listStart = listStart->nextTemp;
    } while (listStart);
}
