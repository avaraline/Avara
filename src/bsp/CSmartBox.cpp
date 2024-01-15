/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CSmartBox.c
    Created: Thursday, November 9, 1995, 12:20
    Modified: Wednesday, August 7, 1996, 21:37
*/

#include "CSmartBox.h"

#include "Memory.h"
#include "Resource.h"
#include "AvaraGL.h"

#define DIMEPSILON 16

typedef struct {
    Fixed baseSize;
    short scaleStyle;
} bspsResource;

/*
{
"400": {"1:1 size": 1.0,"Stretch/Scale (0/1)": 0},
"401": {"1:1 size": 1.0,"Stretch/Scale (0/1)": 0},
"722": {"1:1 size": 5.0,"Stretch/Scale (0/1)": 0}
}
*/

void CSmartBox::ScaleTemplate(Fixed *dimensions, Fixed baseSize) {
    Vector *p;
    short i;
    Fixed x, y, z;
    Vector normalAdjust;
    Vector newNormal;

    x = dimensions[0];
    y = dimensions[1];
    z = dimensions[2];

    if (y == 0)
        y = baseSize;

    p = pointTable.get();
    for (i = 0; i < pointCount; i++) {
        (*p)[0] = FMulDiv((*p)[0], x, baseSize);
        (*p)[1] = FMulDiv((*p)[1], y, baseSize);
        (*p)[2] = FMulDiv((*p)[2], z, baseSize);
        p++;
    }

    if (dimensions[0] > DIMEPSILON && dimensions[1] > DIMEPSILON && dimensions[2] > DIMEPSILON) {
        normalAdjust[0] = FDiv(FIX1, x);
        normalAdjust[1] = FDiv(FIX1, y);
        normalAdjust[2] = FDiv(FIX1, z);
        NormalizeVector(3, normalAdjust);

        for (i = 0; i < polyCount; i++) {
            x = polyTable[i].normal[0];
            y = polyTable[i].normal[1];
            z = polyTable[i].normal[2];

            if ((x && (y || z)) || (y && z)) {
                newNormal[0] = FMul(x, normalAdjust[0]);
                newNormal[1] = FMul(y, normalAdjust[1]);
                newNormal[2] = FMul(z, normalAdjust[2]);
                NormalizeVector(3, newNormal);
                polyTable[i].normal[0] = ToFloat(newNormal[0]);
                polyTable[i].normal[1] = ToFloat(newNormal[1]);
                polyTable[i].normal[2] = ToFloat(newNormal[2]);
            }
        }
    }
    FindEnclosure();
}

void CSmartBox::StretchTemplate(Fixed *dimensions, Fixed baseSize) {
    short i;
    Fixed x, y, z;
    Fixed dx, dy, dz;
    Fixed stretchBound;

    stretchBound = baseSize >> 1;
    dx = dimensions[0] - baseSize;
    dy = dimensions[1] - baseSize;
    dz = dimensions[2] - baseSize;

    for (i = 0; i < pointCount; i++) {
        x = pointTable[i][0];
        y = pointTable[i][1];
        z = pointTable[i][2];

        if (x < -stretchBound) {
            pointTable[i][0] -= dx;
        } else if (x > stretchBound) {
            pointTable[i][0] += dx;
        }

        if (y < -stretchBound) {
            pointTable[i][1] -= dy;
        } else if (y > stretchBound) {
            pointTable[i][1] += dy;
        }

        if (z < -stretchBound) {
            pointTable[i][2] -= dz;
        } else if (z > stretchBound) {
            pointTable[i][2] += dz;
        }
    }
    FindEnclosure();
}

CSmartBox::CSmartBox(
    short resId,
    Fixed *dimensions,
    ARGBColor color,
    ARGBColor altColor,
    CAbstractActor *anActor,
    short aPartCode
) {
    bspsResource **config;
    Fixed baseSize;
    Boolean stretchFlag;

    Handle res = GetResource(BSPTEMPLATETYPE, resId);
    if (res == NULL) {
        resId = dimensions[1] ? BOXTEMPLATERESOURCE : PLATETEMPLATERESOURCE;
    } else {
        ReleaseResource(res);
    }

    CSmartPart::ISmartPart(resId, anActor, aPartCode);

    auto scalingRes = GetResource(BSPSCALETYPE, resId);
    config = (bspsResource **)scalingRes;
    if (config) {
        stretchFlag = ntohs((*config)->scaleStyle);
        baseSize = ntohl((*config)->baseSize);
    } else {
        stretchFlag = false;
        baseSize = FIX1;
    }
    ReleaseResource(scalingRes);

    if (stretchFlag) {
        ScaleTemplate(dimensions, baseSize);
    } else {
        StretchTemplate(dimensions, baseSize);
    }

    // Just replace every color, because these shapes
    // always only have one. They're wall templates!
    ReplaceAllColors(color);

    MoveDone();

    rSquare[0] = 0;
    rSquare[1] = 0;
    FSquareAccumulate(enclosureRadius, rSquare);
    AvaraGLUpdateData(this);
}

void CSmartBox::FindEnclosure() {
    //	Uses algorithm from Graphics Gems I
    long i;

    Fixed xspan, yspan, zspan;
    Fixed maxspan;
    Fixed rad;

    FixedPoint xmin, xmax, ymin, ymax, zmin, zmax;
    FixedPoint dia1, dia2, cen;
    FixedPoint dx, dy, dz;
    FixedPoint *p;

    xmin.x = ymin.y = zmin.z = 0x7fFFffFF;
    xmax.x = ymax.y = zmax.z = -0x7fFFffFF;

    p = (FixedPoint *)pointTable.get();
    for (i = 0; i < pointCount; i++) {
        if (p->x < xmin.x)
            xmin = *p;
        if (p->y < ymin.y)
            ymin = *p;
        if (p->z < zmin.z)
            zmin = *p;
        if (p->x > xmax.x)
            xmax = *p;
        if (p->y > ymax.y)
            ymax = *p;
        if (p->z > zmax.z)
            zmax = *p;

        p++;
    }

    minBounds.x = xmin.x;
    minBounds.y = ymin.y;
    minBounds.z = zmin.z;

    maxBounds.x = xmax.x;
    maxBounds.y = ymax.y;
    maxBounds.z = zmax.z;

    dx.x = xmax.x - xmin.x;
    dx.y = xmax.y - xmin.y;
    dx.z = xmax.z - xmin.z;
    dy.x = ymax.x - ymin.x;
    dy.y = ymax.y - ymin.y;
    dy.z = ymax.z - ymin.z;
    dz.x = zmax.x - zmin.x;
    dz.y = zmax.y - zmin.y;
    dz.z = zmax.z - zmin.z;

    xspan = VectorLength(3, &dx.x);
    yspan = VectorLength(3, &dy.x);
    zspan = VectorLength(3, &dz.x);

    maxspan = xspan;
    dia1 = xmin;
    dia2 = xmax;

    if (yspan > maxspan) {
        maxspan = yspan;
        dia1 = ymin;
        dia2 = ymax;
    }

    if (zspan > maxspan) {
        maxspan = zspan;
        dia1 = zmin;
        dia2 = zmax;
    }

    cen.x = (dia1.x + dia2.x) / 2;
    cen.y = (dia1.y + dia2.y) / 2;
    cen.z = (dia1.z + dia2.z) / 2;

    rad = maxspan / 2;

    p = (FixedPoint *)pointTable.get();
    for (i = 0; i < pointCount; i++) {
        Fixed newrad;

        dx.x = p->x - cen.x;
        dx.y = p->y - cen.y;
        dx.z = p->z - cen.z;

        if (FDistanceOverEstimate(dx.x, dx.y, dx.z) > rad) {
            newrad = VectorLength(3, &dx.x);
            if (newrad > rad) {
                Fixed oldtonew;

                rad = (rad + newrad) / 2;

                oldtonew = newrad - rad;

                cen.x = FMulDiv(rad, cen.x, newrad) + FMulDiv(oldtonew, p->x, newrad);
                cen.y = FMulDiv(rad, cen.y, newrad) + FMulDiv(oldtonew, p->y, newrad);
                cen.z = FMulDiv(rad, cen.z, newrad) + FMulDiv(oldtonew, p->z, newrad);
            }
        }
        p++;
    }

    enclosurePoint = cen;

    p = (FixedPoint *)pointTable.get();
    xspan = 0;
    for (i = 0; i < pointCount; i++) {
        if (FDistanceOverEstimate(p->x, p->y, p->z) > xspan) {
            Fixed temp;

            temp = VectorLength(3, &p->x);
            if (temp > xspan) {
                xspan = temp;
            }
        }

        p++;
    }

    enclosurePoint.w = FIX1; // xspan;
    if (xspan < rad) {
        rad = xspan;
        enclosurePoint.x = 0;
        enclosurePoint.y = 0;
        enclosurePoint.z = 0;
    }

    enclosureRadius = rad;
}
