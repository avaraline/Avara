/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CBSPWorld.h
    Created: Saturday, July 16, 1994, 23:10
    Modified: Tuesday, February 20, 1996, 07:19
*/

#pragma once

#include "CDirectObject.h"
#include "Types.h"

class CBSPPart;
class CViewParameters;

class CBSPWorld {
public:
    virtual ~CBSPWorld() {};

    virtual void AddPart(CBSPPart *thePart) {};
    virtual void DisposeParts() {};
    virtual void OverheadPoint(Fixed *pt, Fixed *extent) {};
    virtual void PrepareForRender() {};
    virtual void RemovePart(CBSPPart *thePart) {};
};

class CBSPWorldImpl final : public CBSPWorld {
private:
    uint16_t partSpace;
    uint16_t partCount;
    uint16_t visibleCount;
    CBSPPart ***partList;
    CBSPPart ***visibleList;
    CBSPPart **visibleP; //	Used while rendering when visibleList is locked down.
public:
    CBSPWorldImpl(short initialObjectSpace);
    virtual ~CBSPWorldImpl();

    virtual void AddPart(CBSPPart *thePart);
    virtual void DisposeParts();
    virtual CBSPPart *GetIndPart(uint16_t ind);
    virtual uint16_t GetPartCount();
    virtual uint16_t GetVisiblePartCount();
    virtual CBSPPart **GetVisiblePartListPointer();
    virtual void OverheadPoint(Fixed *pt, Fixed *extent);
    virtual void PrepareForRender();
    virtual void RemovePart(CBSPPart *thePart);
    virtual void ScoreAndSort(CBSPPart **firstPart, short overlapCount);
    virtual void SortByZ();
    virtual void SortVisibleParts();
    virtual void VisibilitySort(CBSPPart **firstPart, short overlapCount);
};
