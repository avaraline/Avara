/*
    Copyright ©1995-1996, Juri Munkki
    All rights reserved.

    File: CSmartBox.h
    Created: Thursday, November 9, 1995, 12:18
    Modified: Wednesday, August 7, 1996, 19:45
*/

#pragma once
#include "CSmartPart.h"
#include "Material.h"

#define BOXTEMPLATERESOURCE 400
#define PLATETEMPLATERESOURCE 401
#define BSPTEMPLATETYPE 'BSPT'
#define BSPSCALETYPE 'BSPS'

typedef struct {
    Fixed baseSize;
    short scaleStyle;
} BSPSRecord;

class CSmartBox final : public CSmartPart {
public:
    CSmartBox(short resId,
        Fixed *dimensions,
        Material material,
        Material altMaterial,
        CAbstractActor *anActor,
        short aPartCode);

    virtual void ScaleTemplate(Fixed *dimensions, Fixed baseSize);
    virtual void StretchTemplate(Fixed *dimensions, Fixed baseSize);

    virtual void FindEnclosure();
private:
    static void Create() {}
};
