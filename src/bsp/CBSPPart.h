/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CBSPPart.h
    Created: Thursday, June 30, 1994, 10:24
    Modified: Monday, August 12, 1996, 18:22
*/

#pragma once
#include "ARGBColor.h"
#include "CDirectObject.h"
#include "ColorManager.h"
#include "FastMat.h"
#include "VertexData.h"

#include <memory>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#define MAXLIGHTS 4

//	hither/yon cliplist maximum size:
#define MAXCROSSPOINTS 32
#define INDEXSTACKDEPTH 2048 /*	For removal of recursion	*/

class CBSPPart;
class CViewParameters;

enum { noLineClip = 0, touchFrontClip = 1, touchBackClip = 2, lineVisibleClip = 4, edgeExitsFlag = 0x8000 };

struct FixedPoint {
    Fixed x = 0;
    Fixed y = 0;
    Fixed z = 0;
    Fixed w = 0;
    FixedPoint() {};
    FixedPoint(Fixed x, Fixed y, Fixed z, Fixed w) : x(x), y(y), z(z), w(w) {}
    std::string Format() {
        std::ostringstream oss;
        oss << "[" << x << ", " << y << ", " << z << ", " << w << "]";
        return oss.str();
    }
};

struct FloatNormal {
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;
    FloatNormal() {};
    FloatNormal(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct ColorRecord {
    ARGBColor original;
    ARGBColor current;
    ColorRecord(ARGBColor original, ARGBColor current) : original(original), current(current) {}
};

struct PolyRecord {
    FloatNormal normal;
    uint32_t triCount;
    std::unique_ptr<uint32_t[]> triPoints;
    uint32_t front;
    uint32_t back;
    uint16_t colorIdx;
    uint8_t vis;
};

namespace CBSPUserFlags {
    constexpr short kIsAmbient = 1;
    constexpr short kIsStatic = 2;
}

/*
**	Special macros for rotations and translations for speed (unfortunately):
*/
#define TranslatePart(part, dx, dy, dz) \
    part->modelTransform[3][0] += dx; \
    part->modelTransform[3][1] += dy; \
    part->modelTransform[3][2] += dz

#define TranslatePartX(part, delta) part->modelTransform[3][0] += delta
#define TranslatePartY(part, delta) part->modelTransform[3][1] += delta
#define TranslatePartZ(part, delta) part->modelTransform[3][2] += delta

#define InitialRotatePartZ(part, angle) \
    { \
        Fixed tempAngle = angle; \
        part->modelTransform[0][0] = part->modelTransform[1][1] = FOneCos(tempAngle); \
        part->modelTransform[1][0] = -(part->modelTransform[0][1] = FOneSin(tempAngle)); \
    }

#define InitialRotatePartY(part, angle) \
    { \
        Fixed tempAngle = angle; \
        part->modelTransform[0][0] = part->modelTransform[2][2] = FOneCos(tempAngle); \
        part->modelTransform[0][2] = -(part->modelTransform[2][0] = FOneSin(tempAngle)); \
    }

#define InitialRotatePartX(part, angle) \
    { \
        Fixed tempAngle = angle; \
        part->modelTransform[1][1] = part->modelTransform[2][2] = FOneCos(tempAngle); \
        part->modelTransform[2][1] = -(part->modelTransform[1][2] = FOneSin(tempAngle)); \
    }

#define NegateTransformRow(part, row) \
    part->modelTransform[row][0] = -part->modelTransform[row][0]; \
    part->modelTransform[row][1] = -part->modelTransform[row][1]; \
    part->modelTransform[row][2] = -part->modelTransform[row][2];

#define PreFlipX(part) \
    NegateTransformRow(part, 1); \
    NegateTransformRow(part, 2);
#define PreFlipY(part) \
    NegateTransformRow(part, 0); \
    NegateTransformRow(part, 2);
#define PreFlipZ(part) \
    NegateTransformRow(part, 0); \
    NegateTransformRow(part, 1);

class CBSPPart {
public:
    /*
    Handle				itsBSPResource;
    FaceVisibility		**faceTemp;
    BSPResourceHeader	header;
    */
    // Moved here from BSPResourceHeader
    FixedPoint enclosurePoint = {0, 0, 0, 0};
    Fixed enclosureRadius = 0;
    FixedPoint minBounds = {0, 0, 0, 0}; //  Bounding box minimums for x, y, z
    FixedPoint maxBounds = {0, 0, 0, 0}; //  Bounding box maximums for x, y, z
    enum { frontVisible = 1, backVisible, bothVisible };

    // Handle				colorReplacements;	//	Table of colors that replace defaults.

    Matrix modelTransform = {{0}}; //	Transforms to world coordinates. (model)
    Matrix invModelTransform = {{0}}; // (inverse model)

    Matrix modelViewTransform = {{0}}; // modelview
    Matrix invModelViewTransform = {{0}}; // inverse modelview

    Fixed hither = FIX3(500); // 50 cm
    Fixed yon = FIX(500);     // 500 m
    Fixed extraAmbient = 0;
    Fixed privateAmbient = -1;

    //	Bounding volumes:
    Vector sphereCenter = {0}; //	In view coordinates.
    Vector sphereGlobCenter = {0}; //	In global coordinates.
    Vector localViewOrigin = {0}; //	View origin point in local coordinates

    Fixed bigRadius = 0; //	Bounding sphere radius from origin of object coordinates
    Fixed tolerance = 0;

    Fixed minZ = 0; //	In view coordinates
    Fixed maxZ = 0; //	In view coordinates

    Fixed minX = 0; //	In screen coordinates.
    Fixed maxX = 0;
    Fixed minY = 0;
    Fixed maxY = 0;

    //	members used during rendering:
    std::vector<ColorRecord> colorTable;
    std::vector<FixedPoint> pointTable;
    std::vector<PolyRecord> polyTable;
    std::unique_ptr<VertexData> vData;

    //	Lighting vectors in object space
    long lightSeed = 0; //	Seed value to detect lights change
    Vector objLights[MAXLIGHTS]; //	Object space lights.

    //	Used by a CBSPWorld to score and sort objects:
    short worldIndex = 0;
    Boolean worldFlag = 0;
    CBSPPart *nextTemp = nullptr; //	Used temporarily for linked lists.

    Boolean invGlobDone = 0;
    Boolean usesPrivateHither = false;
    Boolean usesPrivateYon = false;
    Boolean isTransparent = false;
    Boolean ignoreDepthTesting = false;
    Boolean ignoreDirectionalLights = false;
    short userFlags = 0; //	Can be used for various flags by user.

    static CBSPPart *Create(short resId);
    virtual void BuildBoundingVolumes();
    virtual ~CBSPPart();

    virtual void ReplaceColor(ARGBColor origColor, ARGBColor newColor);
    virtual void ReplaceAllColors(ARGBColor newColor);

    virtual Boolean PrepareForRender();

    virtual void PreRender();
    void PostRender();
    virtual Boolean InViewPyramid();
    virtual void TransformLights();

    //	Matrix operations on this object:
    virtual void Reset();
    virtual void MoveDone();
    //	virtual	void		Translate(Fixed xt, Fixed yt, Fixed zt);
    virtual void RotateX(Fixed angle); //	Rotate around X axis by angle degrees.
    virtual void RotateY(Fixed angle);
    virtual void RotateZ(Fixed angle);
    virtual void RotateRadX(Fixed angle); //	Rotate around X axis by angle (radians).
    virtual void RotateRadY(Fixed angle);
    virtual void RotateRadZ(Fixed angle);
    virtual void RotateOneX(Fixed angle); //	Rotate around X axis by angle (one-based).
    virtual void RotateOneY(Fixed angle);
    virtual void RotateOneZ(Fixed angle);

    virtual void CopyTransform(Matrix *m); //	modelTransform = m
    virtual void ApplyMatrix(Matrix *m); //	modelTransform = modelTransform * m
    virtual void PrependMatrix(Matrix *m); //	modelTransform = m * modelTransform
    virtual Matrix *GetInverseTransform();

    virtual bool HasAlpha() const;
    virtual void SetScale(Fixed x, Fixed y, Fixed z);
    virtual void ResetScale();
    Vector scale = {FIX1, FIX1, FIX1, FIX1};
    bool hasScale = false;

    //	Compare with another part to see which one is in front:
    virtual Boolean Obscures(CBSPPart *other);
    virtual Boolean HopeNotObscure(CBSPPart *other, Vector *otherCorners);
    virtual Boolean HopeDoesObscure(CBSPPart *other, Vector *otherCorners);

    void PrintMatrix(Matrix *m);
protected:
    bool hasAlpha = false;
    virtual void CheckForAlpha();
    CBSPPart() {}
    virtual void IBSPPart(short resId);
};
