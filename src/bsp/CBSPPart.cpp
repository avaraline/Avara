/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CBSPPart.c
    Created: Thursday, June 30, 1994, 10:28
    Modified: Monday, September 9, 1996, 00:15
*/

#include "AvaraGL.h"
#include "CBSPPart.h"

#include "CViewParameters.h"
#include "Memory.h"
#include "Resource.h"

#include <fstream>
#include <iostream>
#include <json.hpp>
#include <set>

/*  The following stacks are used to eliminate recursion.
 */
short *bspIndexStack = 0;

static long tempLockCount = 0;

static short colorLookupSize = 0;

Vector **bspPointTemp = 0;
static long pointTempMem = 0;

#define MINIMUM_TOLERANCE FIX3(10)

ColorRecord ***bspColorLookupTable = 0;

using json = nlohmann::json;

void CBSPPart::IBSPPart(short resId) {
    char relPath[256];
    snprintf(relPath, 256, "bsps/%d.json", resId);
    char *bspName = BundlePath(relPath);
    // SDL_Log("Loading BSP: %s\n", bspName);
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

    std::ifstream infile(bspName);
    if (infile.fail()) {
        SDL_Log("*** Failed to load BSP %d\n", resId);
        return;
    }

    json doc = json::parse(infile);
    polyCount = doc["polys"].size();
    pointCount = doc["points"].size();

    enclosureRadius = ToFixed(doc["radius1"]);
    bigRadius = ToFixed(doc["radius2"]);

    enclosurePoint.x = ToFixed(doc["center"][0]);
    enclosurePoint.y = ToFixed(doc["center"][1]);
    enclosurePoint.z = ToFixed(doc["center"][2]);
    enclosurePoint.w = FIX1;

    float minX = doc["bounds"]["min"][0];
    float minY = doc["bounds"]["min"][1];
    float minZ = doc["bounds"]["min"][2];

    minBounds.x = ToFixed(minX);
    minBounds.y = ToFixed(minY);
    minBounds.z = ToFixed(minZ);
    minBounds.w = FIX1;

    float maxX = doc["bounds"]["max"][0];
    float maxY = doc["bounds"]["max"][1];
    float maxZ = doc["bounds"]["max"][2];

    maxBounds.x = ToFixed(maxX);
    maxBounds.y = ToFixed(doc["bounds"]["max"][1]);
    maxBounds.z = ToFixed(doc["bounds"]["max"][2]);
    maxBounds.w = FIX1;

    float sigma = .001f;

    isDecal = (
        abs(maxX - minX) < sigma ||
        abs(maxY - minY) < sigma ||
        abs(maxZ - minZ) < sigma
    );

    pointTable = (Vector *)NewPtr(pointCount * sizeof(Vector));
    polyTable = (PolyRecord *)NewPtr(polyCount * sizeof(PolyRecord));

    for (int i = 0; i < pointCount; i++) {
        json pt = doc["points"][i];
        pointTable[i][0] = ToFixed(pt[0]);
        pointTable[i][1] = ToFixed(pt[1]);
        pointTable[i][2] = ToFixed(pt[2]);
        pointTable[i][3] = FIX1;
    }

    totalPoints = 0;

    for (int i = 0; i < polyCount; i++) {
        json poly = doc["polys"][i];
        // Color
        polyTable[i].color = poly["color"];
        polyTable[i].origColor = poly["color"];
        // Normal
        polyTable[i].normal[0] = poly["normal"][0];
        polyTable[i].normal[1] = poly["normal"][1];
        polyTable[i].normal[2] = poly["normal"][2];
        // Triangle points
        polyTable[i].triCount = poly["tris"].size();
        polyTable[i].triPoints = (uint16_t *)NewPtr(polyTable[i].triCount * 3 * sizeof(uint16_t));
        for (int j = 0; j < polyTable[i].triCount; j++) {
            json tri = poly["tris"][j];
            for (int k = 0; k < 3; k++) {
                polyTable[i].triPoints[(j * 3) + k] = (uint16_t)tri[k];
                totalPoints += 1;
            }
        }
    }

    BuildBoundingVolumes();
    Reset();
    UpdateOpenGLData();
}

void CBSPPart::PostRender() {}

void CBSPPart::UpdateOpenGLData() {
    if (!AvaraGLIsRendering()) return;
    glDataSize = totalPoints * sizeof(GLData);
    glData = (GLData *)NewPtr(glDataSize);

    glGenVertexArrays(1, &vertexArray);
    glGenBuffers(1, &vertexBuffer);

    PolyRecord *poly;
    float scale = 1.0; // ToFloat(currentView->screenScale);
    int p = 0;
    for (int i = 0; i < polyCount; i++) {
        poly = &polyTable[i];
        for (int v = 0; v < poly->triCount * 3; v++) {
            Vector *pt = &pointTable[poly->triPoints[v]];
            glData[p].x = ToFloat((*pt)[0]);
            glData[p].y = ToFloat((*pt)[1]);
            glData[p].z = ToFloat((*pt)[2]);
            glData[p].r = ((poly->color >> 16) & 0xFF) / 255.0;
            glData[p].g = ((poly->color >> 8) & 0xFF) / 255.0;
            glData[p].b = (poly->color & 0xFF) / 255.0;

            glData[p].nx = poly->normal[0];
            glData[p].ny = poly->normal[1];
            glData[p].nz = poly->normal[2];

            // SDL_Log("v(%f,%f,%f) c(%f,%f,%f) n(%f,%f,%f)\n", glData[p].x, glData[p].y, glData[p].z, glData[p].r,
            // glData[p].g, glData[p].b, glData[p].nx, glData[p].ny, glData[p].nz);
            p++;
        }
    }
}

void CBSPPart::TransformLights() {
    CViewParameters *vp;
    Matrix *invFull;

    vp = currentView;
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

void CBSPPart::DrawPolygons() {
    AvaraGLDrawPolygons(this);
}

Boolean CBSPPart::InViewPyramid() {
    CViewParameters *vp;
    short i;
    Vector *norms;
    Fixed radius;
    Fixed distance;
    Fixed x, y;
    Fixed z;

    //return true;

    vp = currentView;

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
Boolean CBSPPart::PrepareForRender(CViewParameters *vp) {
    Boolean inPyramid = !isTransparent;

    if (inPyramid) {
        currentView = vp;

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
void CBSPPart::Render(CViewParameters *vp) {
    vp->DoLighting();

    if (PrepareForRender(vp)) {
        DrawPolygons();
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

void CBSPPart::ReplaceColor(int origColor, int newColor) {
    for (int i = 0; i < polyCount; i++) {
        if (polyTable[i].origColor == origColor) {
            polyTable[i].color = newColor;
        }
    }
    UpdateOpenGLData();
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

void CBSPPart::Dispose() {
    for (int i = 0; i < polyCount; i++) {
        DisposePtr((Ptr)polyTable[i].triPoints);
    }

    DisposePtr((Ptr)pointTable);
    DisposePtr((Ptr)polyTable);
    if (AvaraGLIsRendering()) {
        DisposePtr((Ptr)glData);
        glDeleteVertexArrays(1, &vertexArray);
        glDeleteBuffers(1, &vertexBuffer);
    }
    CDirectObject::Dispose();
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
    Fixed *v;
    Fixed delta;
    Fixed sum;
    Vector deltaV;
    Vector center;
    Vector otherCorners[8];
    Vector myCorners[8];
    Matrix combinedTransform;
    short didTests = 0;
    Fixed r;

    // SDL_Log("OBSCURES\n");

    PROFILING_CODE(short theCase = 0;)

    PROFILING_CODE(totalCases++;)

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