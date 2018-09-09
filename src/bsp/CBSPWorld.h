/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CBSPWorld.h
    Created: Saturday, July 16, 1994, 23:10
    Modified: Tuesday, February 20, 1996, 07:19
*/

#pragma once

#include "CDirectObject.h"

class CBSPPart;
class CViewParameters;

class CBSPWorld : public CDirectObject {
public:
    short partCount;
    short partSpace;
    short visibleCount;
    CBSPPart ***partList;
    CBSPPart ***visibleList;
    CBSPPart **visibleP; //	Used while rendering when visibleList is locked down.

    CViewParameters *currentView;

    virtual void IBSPWorld(short initialObjectSpace);
    virtual void DisposeParts();
    virtual void Dispose();

    virtual void AddPart(CBSPPart *thePart);
    virtual void RemovePart(CBSPPart *thePart);
    virtual void Render(CViewParameters *theView);

    virtual void SortVisibleParts();
    virtual void SortByZ();
    virtual void ScoreAndSort(CBSPPart **firstPart, short overlapCount);
    virtual void VisibilitySort(CBSPPart **firstPart, short overlapCount);

    virtual CBSPPart *GetIndPart(short ind);
    virtual short GetPartCount();
};
