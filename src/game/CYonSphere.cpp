/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CYonSphere.c
    Created: Thursday, August 22, 1996, 03:20
    Modified: Thursday, August 22, 1996, 05:20
*/

#include "CYonSphere.h"

void CYonSphere::BeginScript() {
    CAbstractYon::BeginScript();

    ProgramFixedVar(iRange, -1);
}

CAbstractActor *CYonSphere::EndScript() {
    if (CAbstractYon::EndScript()) {
        Fixed range;
        Fixed ignore1, ignore2;

        sphere[3] = GetDome(sphere, &ignore1, &ignore2);
        sphere[1] = ReadFixedVar(iHeight) + ReadFixedVar(iBaseHeight);

        range = ReadFixedVar(iRange);
        if (range > 0) {
            sphere[3] = range;
        } else {
            range = sphere[3];
        }

        minBounds[0] = sphere[0] - range;
        maxBounds[0] = sphere[0] + range;
        minBounds[1] = sphere[1] - range;
        maxBounds[1] = sphere[1] + range;
        minBounds[2] = sphere[2] - range;
        maxBounds[2] = sphere[2] + range;

        SetYon();

        return this;
    } else {
        return NULL;
    }
}
