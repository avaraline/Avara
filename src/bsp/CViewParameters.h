/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CViewParameters.h
    Created: Thursday, June 30, 1994, 10:45
    Modified: Thursday, August 22, 1996, 02:41
*/

#pragma once

#include "ARGBColor.h"
#include "CBSPPart.h"
#include "CDirectObject.h"

enum { kLightOff, kLightViewCoordinates, kLightGlobalCoordinates };

class CBSPPart;

class CViewParameters : public CDirectObject {
public:
    Vector fromPoint = {0, 0, 0};
    Vector atPoint = {0, 0, 0};

    Matrix viewMatrix = {{0}};
    Matrix inverseViewMatrix = {{0}};

    Point viewPixelDimensions = {0, 0};
    Point viewPixelCenter = {0, 0};
    float viewPixelRatio = 0;

    Fixed viewWidth = 0;
    Fixed viewDistance = 0;

    Fixed xOffset = 0;
    Fixed yOffset = 0;
    Fixed screenScale = 0;
    Fixed fWidth = 0;
    Fixed fHeight = 0;

    Vector normal[4] = {{0}}; //	Normals for viewing pyramid bounds.

    Vector corners[8] = {{0}}; //	Corners of truncated viewing pyramid.

    Vector lightSources[MAXLIGHTS] = {{0}};

    short lightMode[MAXLIGHTS] = {0};
    Vector internalLights[MAXLIGHTS] = {{0}}; //	Vector length == light source power.

    Fixed ambientLight = 0; //	Intensity of ambient (nondirectional) light
    ARGBColor ambientLightColor = 0xffffffff; // Color of ambient (nondirectional) light.

    Fixed yonBound = 0;
    Fixed hitherBound = 0;
    Fixed horizonBound = 0;

    long lightSeed = 0;
    short lightSourceCount = 0;
    Boolean dirtyLook = false;
    Boolean inverseDone = false;
    Boolean showTransparent = false;

    virtual void IViewParameters();
    virtual void CalculateViewPyramidCorners();
    virtual void CalculateViewPyramidNormals();
    virtual void Dispose();

    virtual void SetLightValues(short n, Fixed dx, Fixed dy, Fixed dz, short mode);
    virtual void SetLight(short n, Fixed angle1, Fixed angle2, Fixed intensity, short mode);
    virtual void DoLighting();

    virtual void SetViewRect(short width, short height, short centerX, short centerY);
    virtual void Recalculate();

    virtual void LookFrom(Fixed x, Fixed y, Fixed z);
    virtual void LookAt(Fixed x, Fixed y, Fixed z);
    virtual void LookFromPart(CBSPPart *thePart);
    virtual void LookAtPart(CBSPPart *thePart);
    virtual void PointCamera();

    virtual void SetMatrix(Matrix *aViewMatrix);
    virtual Matrix *GetInverseMatrix();
};
