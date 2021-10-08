/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CBSPPart.h
    Created: Thursday, June 30, 1994, 10:24
    Modified: Monday, August 12, 1996, 18:22
*/

#pragma once
#include "CDirectObject.h"
#include "FastMat.h"
#include "Types.h"

#include <SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>

#define MAXLIGHTS 4

//	hither/yon cliplist maximum size:
#define MAXCROSSPOINTS 32
#define INDEXSTACKDEPTH 2048 /*	For removal of recursion	*/

class CBSPPart;
class CViewParameters;

enum { noLineClip = 0, touchFrontClip = 1, touchBackClip = 2, lineVisibleClip = 4, edgeExitsFlag = 0x8000 };

typedef struct {
    Fixed x;
    Fixed y;
    Fixed z;
    Fixed w;
} FixedPoint;

typedef struct {
    uint32_t color;
    uint32_t origColor;
    float normal[3];
    uint16_t triCount;
    uint16_t *triPoints;
} PolyRecord;

typedef uint32_t ColorRecord;

namespace CBSPUserFlags {
    constexpr short kIsAmbient = 1;
    constexpr short kCullBackfaces = 2;
}

/*
**	Special macros for rotations and translations for speed (unfortunately):
*/
#define TranslatePart(part, dx, dy, dz) \
    part->itsTransform[3][0] += dx; \
    part->itsTransform[3][1] += dy; \
    part->itsTransform[3][2] += dz

#define TranslatePartX(part, delta) part->itsTransform[3][0] += delta
#define TranslatePartY(part, delta) part->itsTransform[3][1] += delta
#define TranslatePartZ(part, delta) part->itsTransform[3][2] += delta

#define InitialRotatePartZ(part, angle) \
    { \
        Fixed tempAngle = angle; \
        part->itsTransform[0][0] = part->itsTransform[1][1] = FOneCos(tempAngle); \
        part->itsTransform[1][0] = -(part->itsTransform[0][1] = FOneSin(tempAngle)); \
    }

#define InitialRotatePartY(part, angle) \
    { \
        Fixed tempAngle = angle; \
        part->itsTransform[0][0] = part->itsTransform[2][2] = FOneCos(tempAngle); \
        part->itsTransform[0][2] = -(part->itsTransform[2][0] = FOneSin(tempAngle)); \
    }

#define InitialRotatePartX(part, angle) \
    { \
        Fixed tempAngle = angle; \
        part->itsTransform[1][1] = part->itsTransform[2][2] = FOneCos(tempAngle); \
        part->itsTransform[2][1] = -(part->itsTransform[1][2] = FOneSin(tempAngle)); \
    }

#define NegateTransformRow(part, row) \
    part->itsTransform[row][0] = -part->itsTransform[row][0]; \
    part->itsTransform[row][1] = -part->itsTransform[row][1]; \
    part->itsTransform[row][2] = -part->itsTransform[row][2];

#define PreFlipX(part) \
    NegateTransformRow(part, 1); \
    NegateTransformRow(part, 2);
#define PreFlipY(part) \
    NegateTransformRow(part, 0); \
    NegateTransformRow(part, 2);
#define PreFlipZ(part) \
    NegateTransformRow(part, 0); \
    NegateTransformRow(part, 1);

class CBSPPart : public CDirectObject {
public:
    /*
    Handle				itsBSPResource;
    FaceVisibility		**faceTemp;
    BSPResourceHeader	header;
    */
    // Moved here from BSPResourceHeader
    FixedPoint enclosurePoint;
    Fixed enclosureRadius;
    FixedPoint minBounds; //  Bounding box minimums for x, y, z
    FixedPoint maxBounds; //  Bounding box maximums for x, y, z

    CViewParameters *currentView;

    GLData *glData;
    GLuint vertexArray, vertexBuffer;
    GLsizeiptr glDataSize;

    // Handle				colorReplacements;	//	Table of colors that replace defaults.

    Matrix itsTransform; //	Transforms to world coordinates. (model)
    Matrix invGlobTransform; // (inverse model)

    Matrix fullTransform; // modelview
    Matrix invFullTransform; // inverse modelview

    Fixed hither;
    Fixed yon;
    Fixed extraAmbient;
    Fixed privateAmbient;

    //	Bounding volumes:
    Vector sphereCenter; //	In view coordinates.
    Vector sphereGlobCenter; //	In global coordinates.
    Vector localViewOrigin; //	View origin point in local coordinates

    Fixed bigRadius; //	Bounding sphere radius from origin of object coordinates
    Fixed tolerance;

    Fixed minZ; //	In view coordinates
    Fixed maxZ; //	In view coordinates

    Fixed minX; //	In screen coordinates.
    Fixed maxX;
    Fixed minY;
    Fixed maxY;

    //	members used during rendering:
    Vector *pointTable;
    uint32_t pointCount;
    PolyRecord *polyTable;
    uint32_t polyCount;
    int totalPoints;

    //	Lighting vectors in object space
    long lightSeed; //	Seed value to detect lights change
    Vector objLights[MAXLIGHTS]; //	Object space lights.

    //	Used by a CBSPWorld to score and sort objects:
    short worldIndex;
    Boolean worldFlag;
    CBSPPart *nextTemp; //	Used temporarily for linked lists.

    Boolean invGlobDone;
    Boolean usesPrivateHither;
    Boolean usesPrivateYon;
    Boolean isTransparent;
    Boolean ignoreDirectionalLights;
    short userFlags; //	Can be used for various flags by user.

    virtual void IBSPPart(short resId);
    virtual void BuildBoundingVolumes();
    virtual void Dispose();

    virtual void ReplaceColor(int origColor, int newColor);

    virtual Boolean PrepareForRender(CViewParameters *vp);
    virtual void DrawPolygons();
    virtual void UpdateOpenGLData();

    virtual void PreRender();
    void PostRender();
    virtual void Render(CViewParameters *vp);
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

    virtual void CopyTransform(Matrix *m); //	itsTransform = m
    virtual void ApplyMatrix(Matrix *m); //	itsTransform = itsTransform * m
    virtual void PrependMatrix(Matrix *m); //	itsTransform = m * itsTransform
    virtual Matrix *GetInverseTransform();

    //	Compare with another part to see which one is in front:
    virtual Boolean Obscures(CBSPPart *other);
    virtual Boolean HopeNotObscure(CBSPPart *other, Vector *otherCorners);
    virtual Boolean HopeDoesObscure(CBSPPart *other, Vector *otherCorners);

    void PrintMatrix(Matrix *m);
};
