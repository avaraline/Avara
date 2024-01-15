/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CBSPPart.c
    Created: Thursday, June 30, 1994, 10:28
    Modified: Monday, September 9, 1996, 00:15
*/

#include "CBSPPart.h"

#include "AssetManager.h"
#include "AvaraDefines.h"
#include "CViewParameters.h"
#include "Memory.h"
#include "RenderManager.h"

#include <fstream>
#include <iostream>
#include <json.hpp>
#include <set>

/*  The following stacks are used to eliminate recursion.
 */
short *bspIndexStack = 0;

Vector **bspPointTemp = 0;

#define MINIMUM_TOLERANCE FIX3(10)

ARGBColor ***bspColorLookupTable = 0;

CBSPPart *CBSPPart::Create(short resId) {
    CBSPPart *part = new CBSPPart;
    part->IBSPPart(resId);
    return part;
}


void CBSPPart::IBSPPart(short resId) {
    //SDL_Log("Loading BSP: %s\n", bspName);
    lightSeed = 0;
    nextTemp = NULL;
    // colorReplacements = NULL;    //  Use default colors.
    isTransparent = false;
    ignoreDirectionalLights = false;

    usesPrivateHither = false;
    usesPrivateYon = false;

    extraAmbient = 0;
    privateAmbient = -1;

    hither = FIX3(500); //  50 cm   I set these variables just in case some bozo
    yon = FIX(500); //  500 m   sets the flags above and forgets to set the values.
    userFlags = 0;

    auto json = AssetManager::GetBsp(resId);
    if (!json) {
        colorCount = 0;
        polyCount = 0;
        pointCount = 0;
        return;
    }

    // Fill in some default values in case values are missing.
    auto doc = **json;
    doc.emplace("radius1", 0.0);
    doc.emplace("radius2", 0.0);
    doc.emplace("center", json::array({0.0, 0.0, 0.0}));
    doc["bounds"].emplace("min", json::array({0.0, 0.0, 0.0}));
    doc["bounds"].emplace("max", json::array({0.0, 0.0, 0.0}));

    colorCount = static_cast<uint16_t>(doc["colors"].size());
    polyCount = static_cast<uint32_t>(doc["polys"].size());
    pointCount = static_cast<uint32_t>(doc["points"].size());

    enclosureRadius = ToFixed(doc["radius1"]);
    bigRadius = ToFixed(doc["radius2"]);

    enclosurePoint.x = ToFixed(doc["center"][0]);
    enclosurePoint.y = ToFixed(doc["center"][1]);
    enclosurePoint.z = ToFixed(doc["center"][2]);
    enclosurePoint.w = FIX1;

    float mnX = doc["bounds"]["min"][0];
    float mnY = doc["bounds"]["min"][1];
    float mnZ = doc["bounds"]["min"][2];

    minBounds.x = ToFixed(mnX);
    minBounds.y = ToFixed(mnY);
    minBounds.z = ToFixed(mnZ);
    minBounds.w = FIX1;

    float mxX = doc["bounds"]["max"][0];
    float mxY = doc["bounds"]["max"][1];
    float mxZ = doc["bounds"]["max"][2];

    maxBounds.x = ToFixed(mxX);
    maxBounds.y = ToFixed(mxY);
    maxBounds.z = ToFixed(mxZ);
    maxBounds.w = FIX1;

    origColorTable = std::make_unique<ARGBColor[]>(colorCount);
    currColorTable = std::make_unique<ARGBColor[]>(colorCount);
    pointTable = std::make_unique<Vector[]>(pointCount);
    polyTable = std::make_unique<PolyRecord[]>(polyCount);

    for (uint16_t i = 0; i < colorCount; i++) {
        nlohmann::json value = doc["colors"][i];
        ARGBColor color = ARGBColor::Parse(value)
            .value_or(ARGBColor(0x00ffffff)); // Fallback to invisible "white."
        origColorTable[i] = color;
        if (color == *ColorManager::getMarkerColor(0) ||
            color == *ColorManager::getMarkerColor(1) ||
            color == *ColorManager::getMarkerColor(2) ||
            color == *ColorManager::getMarkerColor(3)) {
            currColorTable[i] = color.WithA(0xff);
        } else {
            currColorTable[i] = color;
        }
    }
    
    CheckForAlpha();

    for (uint32_t i = 0; i < pointCount; i++) {
        nlohmann::json pt = doc["points"][i];
        pointTable[i][0] = ToFixed(pt[0]);
        pointTable[i][1] = ToFixed(pt[1]);
        pointTable[i][2] = ToFixed(pt[2]);
        pointTable[i][3] = FIX1;
    }

    totalPoints = 0;

    for (uint32_t i = 0; i < polyCount; i++) {
        nlohmann::json poly = doc["polys"][i];
        nlohmann::json pt;
        // Color
        polyTable[i].colorIdx = static_cast<uint16_t>(poly["color"]);
        // Normal
        nlohmann::json norms = doc["normals"];
        int idx = poly["normal"];
        nlohmann::json norm = norms[idx];
        polyTable[i].normal[0] = norm[0];
        polyTable[i].normal[1] = norm[1];
        polyTable[i].normal[2] = norm[2];
        // Triangle points
        polyTable[i].vis = static_cast<uint8_t>(poly["vis"]);
        polyTable[i].triCount = poly["tris"].size() / 3;
        polyTable[i].triPoints = std::make_unique<uint16_t[]>(poly["tris"].size());
        for (size_t j = 0; j < poly["tris"].size(); j += 3) {
            for (size_t k = 0; k < 3; k++) {
                pt = poly["tris"][j + k];
                polyTable[i].triPoints[j + k] = (uint16_t)pt;
                totalPoints += 1;
            }
        }
        if (poly.contains("front")) {
            polyTable[i].front = poly["front"];
        }
        else polyTable[i].front = uint16_t(-1);

        if (poly.contains("back")) {
            polyTable[i].back = poly["back"];
        }
        else polyTable[i].back = uint16_t(-1);
    }

    BuildBoundingVolumes();
    Reset();
    AvaraGLUpdateData(this);
}

void CBSPPart::PostRender() {}

void CBSPPart::TransformLights() {
    auto vp = RenderManager::viewParams;
    if (!ignoreDirectionalLights) {
        if (lightSeed != vp->lightSeed) {
            lightSeed = vp->lightSeed;

            VectorMatrixProduct(vp->lightSourceCount, vp->lightSources, objLights, &invGlobTransform);
        }
    }

    localViewOrigin[0] = invFullTransform[3][0];
    localViewOrigin[1] = invFullTransform[3][1];
    localViewOrigin[2] = invFullTransform[3][2];
}

void CBSPPart::DrawPolygons(Shader shader) {
    AvaraGLDrawPolygons(this, shader);
}

Boolean CBSPPart::InViewPyramid() {
    Fixed radius;
    Fixed distance;
    Fixed x, y;
    Fixed z;

    auto vp = RenderManager::viewParams;

    if (hither >= yon)
        return false;

    z = vp->viewMatrix[3][2];
    z += FMul(sphereGlobCenter[0], vp->viewMatrix[0][2]);
    z += FMul(sphereGlobCenter[1], vp->viewMatrix[1][2]);
    z += FMul(sphereGlobCenter[2], vp->viewMatrix[2][2]);
    minZ = z - enclosureRadius;
    maxZ = z + enclosureRadius;

    if (minZ > yon || maxZ < hither) {
        return false;
    }

    VectorMatrixProduct(1, &sphereGlobCenter, &sphereCenter, &vp->viewMatrix);
    z = sphereCenter[2];
    x = sphereCenter[0];
    radius = -enclosureRadius;

    distance = FMul(x, vp->normal[0][0]);
    distance += FMul(z, vp->normal[0][2]);
    if (distance < radius)
        return false;

    distance = FMul(x, vp->normal[1][0]);
    distance += FMul(z, vp->normal[1][2]);
    if (distance < radius)
        return false;

    y = sphereCenter[1];
    distance = FMul(y, vp->normal[2][1]);
    distance += FMul(z, vp->normal[2][2]);
    if (distance < radius)
        return false;

    distance = FMul(y, vp->normal[3][1]);
    distance += FMul(z, vp->normal[3][2]);
    if (distance < radius)
        return false;

    return true;
}

/*
**  Lock handles and make pointers to data valid.
*/
void CBSPPart::PreRender() {}

void CBSPPart::PrintMatrix(Matrix *m) {
    for (int i = 0; i < 4; i++) {
        SDL_Log("%.4f %.4f %.4f %.4f\n",
            ToFloat((*m)[0][i]),
            ToFloat((*m)[1][i]),
            ToFloat((*m)[2][i]),
            ToFloat((*m)[3][i]));
    }
}

/*
**  See if the part is in the viewing pyramid and do calculations
**  in preparation to shading.
*/
Boolean CBSPPart::PrepareForRender() {
    auto vp = RenderManager::viewParams;
    Boolean inPyramid = vp->showTransparent || !isTransparent;

    if (inPyramid) {

        if (!usesPrivateHither)
            hither = vp->hitherBound;
        if (!usesPrivateYon)
            yon = vp->yonBound;

        inPyramid = InViewPyramid();

        if (inPyramid) {
            // SDL_Log("itsTransform:\n");
            // PrintMatrix(&itsTransform);

            CombineTransforms(&itsTransform, &fullTransform, &vp->viewMatrix);

            InverseTransform(&fullTransform, &invFullTransform);

            // SDL_Log("fullTransform:\n");
            // PrintMatrix(&fullTransform);
            // PrintMatrix(invFullTransform);

            if (!invGlobDone) {
                InverseTransform(&itsTransform, &invGlobTransform);
                invGlobDone = true;
            }

            TransformLights();

            // transform all the points before rendering
            //VectorMatrixProduct(pointCount, pointTable, transformedPoints, &fullTransform);
        }
    }

    return inPyramid;
}

/*
**  Normally you would create a CBSPWorld and attach the
**  part to that world. However, if you only have a single
**  CBSPPart, you can call Render and you don't need a
**  CBSPWorld. Even then it is recommended that you use a
**  CBSPWorld, since it really doesn't add any significant
**  overhead.
*/
void CBSPPart::Render(Shader shader) {
    RenderManager::viewParams->DoLighting();

    if (PrepareForRender()) {
        DrawPolygons(shader);
        PostRender();
    }
}

/*
**  Reset the part to the origin and to its natural
**  orientation.
*/
void CBSPPart::Reset() {
    lightSeed = 0;
    OneMatrix(&itsTransform);
}

//  invalidates data & calcs sphereGlobCenter
void CBSPPart::MoveDone() {
    VectorMatrixProduct(1, (Vector *)&enclosurePoint, &sphereGlobCenter, &itsTransform);
    invGlobDone = false;
    lightSeed = 0;
}

#ifdef TRANSLATE_PART
/*
**  Move by xt, yt, zt
*/
void CBSPPart::Translate(Fixed xt, Fixed yt, Fixed zt) {
    itsTransform[3][0] += xt;
    itsTransform[3][1] += yt;
    itsTransform[3][2] += zt;
}
#endif

void CBSPPart::RotateX(Fixed angle) {
    angle = FDegToOne(angle);
    MRotateX(FOneSin(angle), FOneCos(angle), &itsTransform);
}

void CBSPPart::RotateY(Fixed angle) {
    angle = FDegToOne(angle);
    MRotateY(FOneSin(angle), FOneCos(angle), &itsTransform);
}

void CBSPPart::RotateZ(Fixed angle) {
    angle = FDegToOne(angle);
    MRotateZ(FOneSin(angle), FOneCos(angle), &itsTransform);
}

void CBSPPart::RotateRadX(Fixed angle) {
    angle = FRadToOne(angle);
    MRotateX(FOneSin(angle), FOneCos(angle), &itsTransform);
}

void CBSPPart::RotateRadY(Fixed angle) {
    angle = FRadToOne(angle);
    MRotateY(FOneSin(angle), FOneCos(angle), &itsTransform);
}

void CBSPPart::RotateRadZ(Fixed angle) {
    angle = FRadToOne(angle);
    MRotateZ(FOneSin(angle), FOneCos(angle), &itsTransform);
}

void CBSPPart::RotateOneX(Fixed angle) {
    MRotateX(FOneSin(angle), FOneCos(angle), &itsTransform);
}

void CBSPPart::RotateOneY(Fixed angle) {
    MRotateY(FOneSin(angle), FOneCos(angle), &itsTransform);
}
void CBSPPart::RotateOneZ(Fixed angle) {
    MRotateZ(FOneSin(angle), FOneCos(angle), &itsTransform);
}

void CBSPPart::CopyTransform(Matrix *m) {
    invGlobDone = false;
    *(MatrixStruct *)&itsTransform = *(MatrixStruct *)m;
}

void CBSPPart::ApplyMatrix(Matrix *m) {
    Matrix fullMatrix;

    CombineTransforms(&itsTransform, &fullMatrix, m);

    *((MatrixStruct *)&itsTransform) = *(MatrixStruct *)&fullMatrix;
}
void CBSPPart::PrependMatrix(Matrix *m) {
    Matrix fullMatrix;

    CombineTransforms(m, &fullMatrix, &itsTransform);

    *((MatrixStruct *)&itsTransform) = *(MatrixStruct *)&fullMatrix;
}

Matrix *CBSPPart::GetInverseTransform() {
    if (!invGlobDone) {
        invGlobDone = true;
        InverseTransform(&itsTransform, &invGlobTransform);
    }

    return &invGlobTransform;
}

void CBSPPart::ReplaceColor(ARGBColor origColor, ARGBColor newColor) {
    for (int i = 0; i < colorCount; i++) {
        if (origColorTable[i] == origColor) {
            currColorTable[i] = newColor;
        }
    }
    CheckForAlpha();
    AvaraGLUpdateData(this);
}

void CBSPPart::ReplaceAllColors(ARGBColor newColor) {
    for (int i = 0; i < colorCount; i++) {
        currColorTable[i] = newColor;
    }
    hasAlpha = (newColor.GetA() != 0xff);
    AvaraGLUpdateData(this);
}

void CBSPPart::BuildBoundingVolumes() {
    Fixed span;

    tolerance = maxBounds.x - minBounds.x;
    span = maxBounds.y - minBounds.y;

    if (span > MINIMUM_TOLERANCE && span < tolerance)
        tolerance = span;

    span = maxBounds.z - minBounds.z;
    if (span > MINIMUM_TOLERANCE && span < tolerance)
        tolerance = span;

    tolerance >>= 4;
    if (tolerance < MINIMUM_TOLERANCE)
        tolerance = MINIMUM_TOLERANCE;
}

CBSPPart::~CBSPPart() {
    if(polyCount < 1) {
        return;
    }
    if (AvaraGLIsRendering()) {
        glDataSize = 0;
        glDeleteVertexArrays(1, &vertexArray);
        glDeleteBuffers(1, &vertexBuffer);
    }
}

void CBSPPart::CheckForAlpha() {
    hasAlpha = false;
    for (uint16_t i = 0; i < colorCount; i++) {
        if (currColorTable[i].GetA() != 0xff) {
            hasAlpha = true;
            return;
        }
    }
}

bool CBSPPart::HasAlpha() {
    return hasAlpha;
}

void CBSPPart::SetScale(Fixed x, Fixed y, Fixed z) {
    hasScale = true;
    scale[0] = x;
    scale[1] = y;
    scale[2] = z;
}

void CBSPPart::ResetScale() {
    hasScale = false;
    scale[0] = FIX1;
    scale[1] = FIX1;
    scale[2] = FIX1;
    scale[3] = FIX1;
}

#define TESTBOUND(b, c) \
    (otherCorners[0][b] c boundary) && (otherCorners[1][b] c boundary) && (otherCorners[2][b] c boundary) && \
        (otherCorners[3][b] c boundary) && (otherCorners[4][b] c boundary) && (otherCorners[5][b] c boundary) && \
        (otherCorners[6][b] c boundary) && (otherCorners[7][b] c boundary)

Boolean CBSPPart::HopeNotObscure(CBSPPart *other, Vector *otherCorners) {
    Fixed boundary;

    if (localViewOrigin[0] < minBounds.x) {
        boundary = minBounds.x + tolerance;
        if (TESTBOUND(0, <=))
            return false;
    }

    if (localViewOrigin[0] > maxBounds.x) {
        boundary = maxBounds.x - tolerance;
        if (TESTBOUND(0, >=))
            return false;
    }

    if (localViewOrigin[1] < minBounds.y) {
        boundary = minBounds.y + tolerance;
        if (TESTBOUND(1, <=))
            return false;
    }

    if (localViewOrigin[1] > maxBounds.y) {
        boundary = maxBounds.y - tolerance;
        if (TESTBOUND(1, >=))
            return false;
    }

    if (localViewOrigin[2] < minBounds.z) {
        boundary = minBounds.z + tolerance;
        if (TESTBOUND(2, <=))
            return false;
    }

    if (localViewOrigin[2] > maxBounds.z) {
        boundary = maxBounds.z - tolerance;
        if (TESTBOUND(2, >=))
            return false;
    }

    return true;
}

Boolean CBSPPart::HopeDoesObscure(CBSPPart *other, Vector *otherCorners) {
    Fixed boundary;

    if (localViewOrigin[0] > minBounds.x) {
        boundary = minBounds.x + tolerance;
        if (TESTBOUND(0, <=))
            return false;
    }

    if (localViewOrigin[0] < maxBounds.x) {
        boundary = maxBounds.x - tolerance;
        if (TESTBOUND(0, >=))
            return false;
    }

    if (localViewOrigin[1] > minBounds.y) {
        boundary = minBounds.y + tolerance;
        if (TESTBOUND(1, <=))
            return false;
    }

    if (localViewOrigin[1] < maxBounds.y) {
        boundary = maxBounds.y - tolerance;
        if (TESTBOUND(1, >=))
            return false;
    }

    if (localViewOrigin[2] > minBounds.z) {
        boundary = minBounds.z + tolerance;
        if (TESTBOUND(2, <=))
            return false;
    }

    if (localViewOrigin[2] < maxBounds.z) {
        boundary = maxBounds.z - tolerance;
        if (TESTBOUND(2, >=))
            return false;
    }

    return true;
}
#define DOESOBSCURE(a) a->visibilityScore = true;
#define DOESNOTOBSCURE(a) a->visibilityScore = false;

#define DO_PROFILE 0
#if DO_PROFILE
#define PROFILING_CODE(a) a

static long caseTable[256];
static long totalCases = 0;
#else
#define PROFILING_CODE(a)
#endif

Boolean CBSPPart::Obscures(CBSPPart *other) {
    //Fixed *v;
    //Fixed delta;
    //Fixed sum;
    //Vector deltaV;
    Vector center;
    Vector otherCorners[8];
    Vector myCorners[8];
    Matrix combinedTransform;
    short didTests = 0;
    Fixed r;

    // SDL_Log("OBSCURES\n");

    PROFILING_CODE(short theCase = 0;)

    PROFILING_CODE(totalCases++;)

    // Never treat as obscuring if this shape has any translucency anywhere.
    if (HasAlpha()) {
        return false;
    }

    VectorMatrixProduct(1, &other->sphereGlobCenter, &center, &invGlobTransform);
    r = other->enclosureRadius - tolerance;

    if ((localViewOrigin[0] < minBounds.x && center[0] <= minBounds.x - r) ||
        (localViewOrigin[0] > maxBounds.x && center[0] >= maxBounds.x + r) ||
        (localViewOrigin[1] < minBounds.y && center[1] <= minBounds.y - r) ||
        (localViewOrigin[1] > maxBounds.y && center[1] >= maxBounds.y + r) ||
        (localViewOrigin[2] < minBounds.z && center[2] <= minBounds.z - r) ||
        (localViewOrigin[2] > maxBounds.z && center[2] >= maxBounds.z + r)) {
        PROFILING_CODE(caseTable[1]++;)
        return false;
    }

    VectorMatrixProduct(1, &sphereGlobCenter, &center, &other->invGlobTransform);
    r = enclosureRadius - other->tolerance;

    if ((other->localViewOrigin[0] > other->minBounds.x && center[0] <= other->minBounds.x - r) ||
        (other->localViewOrigin[0] < other->maxBounds.x && center[0] >= other->maxBounds.x + r) ||
        (other->localViewOrigin[1] > other->minBounds.y && center[1] <= other->minBounds.y - r) ||
        (other->localViewOrigin[1] < other->maxBounds.y && center[1] >= other->maxBounds.y + r) ||
        (other->localViewOrigin[2] > other->minBounds.z && center[2] <= other->minBounds.z - r) ||
        (other->localViewOrigin[2] < other->maxBounds.z && center[2] >= other->maxBounds.z + r)) {
        PROFILING_CODE(caseTable[2]++;)
        return false;
    }

    if ((localViewOrigin[0] < minBounds.x) || (localViewOrigin[0] > maxBounds.x) ||
        (localViewOrigin[1] < minBounds.y) || (localViewOrigin[1] > maxBounds.y) ||
        (localViewOrigin[2] < minBounds.z) || (localViewOrigin[2] > maxBounds.z)) {
        didTests = 1;

        CombineTransforms(&itsTransform, &combinedTransform, &other->invGlobTransform);

        TransformBoundingBox(&combinedTransform, &minBounds.x, &maxBounds.x, myCorners);
        if (!other->HopeDoesObscure(this, myCorners)) {
            PROFILING_CODE(caseTable[3]++;)
            return false;
        }
    }

    if ((other->localViewOrigin[0] > other->minBounds.x) || (other->localViewOrigin[0] < other->maxBounds.x) ||
        (other->localViewOrigin[1] > other->minBounds.y) || (other->localViewOrigin[1] < other->maxBounds.y) ||
        (other->localViewOrigin[2] > other->minBounds.z) || (other->localViewOrigin[2] < other->maxBounds.z)) {
        didTests |= 2;
        CombineTransforms(&other->itsTransform, &combinedTransform, &invGlobTransform);

        TransformBoundingBox(&combinedTransform, &other->minBounds.x, &other->maxBounds.x, otherCorners);

        if (!HopeNotObscure(other, otherCorners)) {
            PROFILING_CODE(caseTable[6]++;)
            return false;
        }
    }

#ifdef NOT_WORTH_IT

    if (visibilityScore) {
        if (((didTests & 1) && HopeDoesObscure(other, otherCorners)) &&
            ((didTests & 2) && other->HopeNotObscure(this, myCorners))) {
            visibilityScore = maxZ < other->maxZ;
        } else
            theCase = 4;
    }

    if (visibilityScore) {
        v = other->sphereCenter;

        deltaV[0] = sphereCenter[0] - v[0];
        deltaV[1] = sphereCenter[1] - v[1];
        deltaV[2] = sphereCenter[2] - v[2];

        delta = FDistanceEstimate(deltaV[0], deltaV[1], deltaV[2]);
        sum = enclosureRadius + other->enclosureRadius;

        if (delta >= sum) {
            delta = VectorLength(3, deltaV);
        }

        if (delta >= sum) {
            Fixed dist;

            dist = -FMulDivNZ(deltaV[0], v[0], delta);
            dist -= FMulDivNZ(deltaV[1], v[1], delta);
            dist -= FMulDivNZ(deltaV[2], v[2], delta);

            if (dist < other->enclosureRadius) {
                theCase = 5;
                visibilityScore = false; // return false;
            }
        }
    }
#endif

    PROFILING_CODE(caseTable[theCase]++;)

#if 1
    if ((didTests == 3) && HopeDoesObscure(other, otherCorners) && other->HopeNotObscure(this, myCorners)) {
#if DO_PROFILE
        if (maxZ < other->maxZ)
            caseTable[7]++;
        else
            caseTable[8]++;
#endif

        //  return maxZ < other->maxZ;
        return minZ + maxZ < other->minZ + other->maxZ;
    } else
#endif
    {
        PROFILING_CODE(caseTable[9]++;)
        return true;
    }
}
