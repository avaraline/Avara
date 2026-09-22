/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CSmartBox.c
    Created: Thursday, November 9, 1995, 12:20
    Modified: Wednesday, August 7, 1996, 21:37
*/

#include "CSmartBox.h"
#include "ColorManager.h"

#include "Memory.h"
#include "AssetManager.h"

#define DIMEPSILON 16


void CSmartBox::ScaleTemplate(Fixed *dimensions, Fixed baseSize) {
    Fixed x, y, z;
    Vector normalAdjust;
    Vector newNormal;

    x = dimensions[0];
    y = dimensions[1];
    z = dimensions[2];

    if (y == 0)
        y = baseSize;

    for (auto &point : pointTable) {
        point.x = FMulDiv(point.x, x, baseSize);
        point.y = FMulDiv(point.y, y, baseSize);
        point.z = FMulDiv(point.z, z, baseSize);
    }

    if (dimensions[0] > DIMEPSILON && dimensions[1] > DIMEPSILON && dimensions[2] > DIMEPSILON) {
        normalAdjust[0] = FDiv(FIX1, x);
        normalAdjust[1] = FDiv(FIX1, y);
        normalAdjust[2] = FDiv(FIX1, z);
        NormalizeVector(3, normalAdjust);

        for (auto &poly : polyTable) {
            x = poly.normal.x;
            y = poly.normal.y;
            z = poly.normal.z;

            if ((x && (y || z)) || (y && z)) {
                newNormal[0] = FMul(x, normalAdjust[0]);
                newNormal[1] = FMul(y, normalAdjust[1]);
                newNormal[2] = FMul(z, normalAdjust[2]);
                NormalizeVector(3, newNormal);
                poly.normal.x = ToFloat(newNormal[0]);
                poly.normal.y = ToFloat(newNormal[1]);
                poly.normal.z = ToFloat(newNormal[2]);
            }
        }
    }
    FindEnclosure();
}

void CSmartBox::StretchTemplate(Fixed *dimensions, Fixed baseSize) {
    Fixed x, y, z;
    Fixed dx, dy, dz;
    Fixed stretchBound;

    stretchBound = baseSize >> 1;
    dx = dimensions[0] - baseSize;
    dy = dimensions[1] - baseSize;
    dz = dimensions[2] - baseSize;

    for (auto &point : pointTable) {
        x = point.x;
        y = point.y;
        z = point.z;

        if (x < -stretchBound) {
            point.x -= dx;
        } else if (x > stretchBound) {
            point.x += dx;
        }

        if (y < -stretchBound) {
            point.y -= dy;
        } else if (y > stretchBound) {
            point.y += dy;
        }

        if (z < -stretchBound) {
            point.z -= dz;
        } else if (z > stretchBound) {
            point.z += dz;
        }
    }
    FindEnclosure();
}

CSmartBox::CSmartBox(
    short resId,
    Fixed *dimensions,
    Material material,
    Material altMaterial,
    CAbstractActor *anActor,
    short aPartCode
) {
    Fixed baseSize = FIX1;
    Boolean stretchFlag = false;

    auto boxTmpl = AssetManager::GetBsp(resId);
    if (!boxTmpl) {
        resId = dimensions[1] ? BOXTEMPLATERESOURCE : PLATETEMPLATERESOURCE;
    }

    CSmartPart::ISmartPart(resId, anActor, aPartCode);

    auto bsps = AssetManager::GetBspScale(resId);
    if (bsps) {
        baseSize = (**bsps).baseSize;
        stretchFlag = (**bsps).scaleStyle;
    }
    
    if (stretchFlag) {
        ScaleTemplate(dimensions, baseSize);
    } else {
        StretchTemplate(dimensions, baseSize);
    }

    ReplaceMaterialForColor(*ColorManager::getMarkerColor(0), material);
    ReplaceMaterialForColor(*ColorManager::getMarkerColor(1), altMaterial);

    MoveDone();

    rSquare[0] = 0;
    rSquare[1] = 0;
    FSquareAccumulate(enclosureRadius, rSquare);
}

void CSmartBox::FindEnclosure() {
    //	Uses algorithm from Graphics Gems I
    Fixed xspan, yspan, zspan;
    Fixed maxspan;
    Fixed rad;

    FixedPoint xmin, xmax, ymin, ymax, zmin, zmax;
    FixedPoint dia1, dia2, cen;
    FixedPoint dx, dy, dz;

    xmin.x = ymin.y = zmin.z = 0x7fFFffFF;
    xmax.x = ymax.y = zmax.z = -0x7fFFffFF;

    for (auto &p : pointTable) {
        if (p.x < xmin.x)
            xmin = p;
        if (p.y < ymin.y)
            ymin = p;
        if (p.z < zmin.z)
            zmin = p;
        if (p.x > xmax.x)
            xmax = p;
        if (p.y > ymax.y)
            ymax = p;
        if (p.z > zmax.z)
            zmax = p;
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

    for (auto &p : pointTable) {
        Fixed newrad;

        dx.x = p.x - cen.x;
        dx.y = p.y - cen.y;
        dx.z = p.z - cen.z;

        if (FDistanceOverEstimate(dx.x, dx.y, dx.z) > rad) {
            newrad = VectorLength(3, &dx.x);
            if (newrad > rad) {
                Fixed oldtonew;

                rad = (rad + newrad) / 2;

                oldtonew = newrad - rad;

                cen.x = FMulDiv(rad, cen.x, newrad) + FMulDiv(oldtonew, p.x, newrad);
                cen.y = FMulDiv(rad, cen.y, newrad) + FMulDiv(oldtonew, p.y, newrad);
                cen.z = FMulDiv(rad, cen.z, newrad) + FMulDiv(oldtonew, p.z, newrad);
            }
        }
    }

    enclosurePoint = cen;

    xspan = 0;
    for (auto &p : pointTable) {
        if (FDistanceOverEstimate(p.x, p.y, p.z) > xspan) {
            Fixed temp;

            temp = VectorLength(3, &p.x);
            if (temp > xspan) {
                xspan = temp;
            }
        }
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
