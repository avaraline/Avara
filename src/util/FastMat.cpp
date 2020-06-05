/*
    Copyright ©1992-1996, Juri Munkki
    All rights reserved.

    File: FastMat.c
    Created: Saturday, October 17, 1992, 01:45
    Modified: Tuesday, July 30, 1996, 08:40
*/

#include "FastMat.h"

#include "Memory.h"
#include "System.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <utility>

#define ARCUSTABLEBITS 9
#define ARCUSTABLESIZE (1 + (1 << ARCUSTABLEBITS))
#define TRIGTABLESIZE 4096L /*  Don't change this number!   */

static unsigned short *arcTanTable = 0;
static unsigned short *arcTanOneTable = 0;

/* FixMath */

Fixed FixMul(Fixed a, Fixed b) {
    return ((int64_t)a * (int64_t)b) >> 16;
}

Fixed FixDiv(Fixed a, Fixed b) {
    return ToFixed(ToFloat(a) / ToFloat(b));
}

Fixed FixATan2(Fixed x, Fixed y) {
    return ToFixed(atan2(ToFloat(y), ToFloat(x)));
}

void InitTrigTables() {
    long i;

    {
        arcTanTable = (unsigned short *)NewPtr(sizeof(unsigned short) * ARCUSTABLESIZE * 2);
        arcTanOneTable = arcTanTable + ARCUSTABLESIZE;

        for (i = 0; i < ARCUSTABLESIZE; i++) {
            arcTanTable[i] = FixATan2(ARCUSTABLESIZE - 1, i);
            arcTanOneTable[i] = FRadToOne(arcTanTable[i]);
        }
    }
}

void CloseTrigTables() {
    if (arcTanTable) {
        DisposePtr((Ptr)arcTanTable);
    }
}

void OneMatrix(Matrix *theMatrix) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            (*theMatrix)[i][j] = i == j ? FIX1 : 0;
        }
    }
}

Fixed VectorLength(long n, Fixed *v) {
    int acc[2] = {0, 0};

    while (n--) {
        FSquareAccumulate(*v++, acc);
    }

    return FSqroot(acc);
}

Fixed NormalizeVector(long n, Fixed *v) {
    Fixed l;

    l = VectorLength(n, v);
    if (l)
        while (n--) {
            *v = FDivNZ(*v, l);
            v++;
        }

    return l;
}

void InitMatrix() {
    InitTrigTables();
    FRandSeed = TickCount();
}

void CloseMatrix() {
    CloseTrigTables();
}

Fixed FSqroot(int *ab) {
    // return ToFixed(sqrt(*ab));
    Fixed root = 0; // d0
    unsigned int remHi = 0;
    unsigned int remLo = *ab++;
    unsigned int remLoLo = *ab++;
    unsigned int testDiv;
    int count = 31;

    do {
        /* get 2 bits of arg  */
        remHi = (remHi << 2) | (remLo >> 30);
        remLo = (remLo << 2) | (remLoLo >> 30);
        remLoLo <<= 2;

        root <<= 1; /* Get ready for the next bit in the root */
        testDiv = (root << 1) + 1; /* Test divisor */
        if (remHi >= testDiv) {
            remHi -= testDiv;
            root += 1;
        }
    } while (count-- != 0);

    return root;
}

Fixed FRandomBeta() {
    // I don't think this is right but it works
    return ToFixed((float)(rand() % 1000) / 1000.0);
}

Fixed DistanceEstimate(Fixed x1, Fixed y1, Fixed x2, Fixed y2) {
    Fixed a = x1 > y1 ? x1 - y1 : y1 - x1;
    Fixed b = x2 > y2 ? x2 - y2 : y2 - x2;
    return FSqrt(FMul(a, a) + FMul(b, b));
}

/*  FDistanceEstimate
**
**  May underestimate distance by about 15%.
**
**  The method is based on a Graphics Gem, only adapted to 3D.
**
**  Absolute values of the deltas are first calculated, then
**  the deltas are sorted from greatest to smallest (D0, D1, D2).
**  An estimate is calculated with (D0 + D1/2 + D2/4) * (1-1/8-1/256)
*/
Fixed FDistanceEstimate(Fixed dx, Fixed dy, Fixed dz) {
    // Let's see if doing it accurately affects anything.
    // return FSqrt(FMul(dx, dx) + FMul(dy, dy) + FMul(dz, dz));

    Fixed r0 = dx, r1 = dy, r2 = dz;
/*
    move.l  dx,D0
    bpl.s   @noXNeg
    neg.l   D0
*/  
    if (r0 < 0) r0 = -r0;
/*
@noXNeg:
        move.l  dy,D1
        bpl.s   @noYNeg
        neg.l   D1
*/
    if (r1 < 0) r1 = -r1;
/*
@noYNeg:
        cmp.l   D0,D1
        blt.s   @noXYSwap
        exg.l   D0,D1
*/
    if (r0 < r1) std::swap(r0, r1);
/*
@noXYSwap:
        move.l  dz,D2
        bpl.s   @noZNeg
        neg.l   D2
*/
    if (r2 < 0) r2 = -r2;
/*
@noZNeg:
        cmp.l   D1,D2
        blt.s   @noYZSwap
        exg.l   D1,D2
*/
    if (r1 < r2) std::swap(r1, r2);
/*
        cmp.l   D0,D1
        blt.s   @noXZSwap
        exg.l   D0,D1
*/
    if (r0 < r1) std::swap(r0, r1);
/*
@noXZSwap:
@noYZSwap:
        lsr.l   #1,D2
        add.l   D2,D1
        lsr.l   #1,D1
        add.l   D1,D0
*/
    r2 = r2 >> 1;
    r1 = r1 + r2;
    r1 = r1 >> 1;
    r0 = r1 + r0;
/*
        move.l  D0,D1   // scale by (1-1/8-1/256) to keep estimate from being too large.
        lsr.l   #3,D1
        sub.l   D1,D0
        lsr.l   #5,D1
        sub.l   D1,D0
*/
    r1 = r0;
    r1 = r1 >> 3;
    r0 = r0 - r1;
    r1 = r1 >> 5;
    r0 = r0 - r1;

    return r0;
}

/*  FDistanceEstimate
**
**  May overestimate distance by about 15%.
**
**  The method is based on a Graphics Gem, only adapted to 3D.
**
**  Absolute values of the deltas are first calculated, then
**  the deltas are sorted from greatest to smallest (D0, D1, D2).
**  An estimate is calculated with (D0 + D1/2 + D2/4)
*/
Fixed FDistanceOverEstimate(Fixed dx, Fixed dy, Fixed dz) {
    // Let's see if doing it accurately affects anything.
    //return FSqrt(FMul(dx, dx) + FMul(dy, dy) + FMul(dz, dz));
    Fixed r0 = dx, r1 = dy, r2 = dz;
/*
        move.l  dx,D0
        bpl.s   @noXNeg
        neg.l   D0
*/

    if (r0 < 0) r0 = -r0;
/*
@noXNeg:
        move.l  dy,D1
        bpl.s   @noYNeg
        neg.l   D1
*/
    if (r1 < 0) r1 = -r1;
/*
@noYNeg:
        cmp.l   D0,D1
        blt.s   @noXYSwap
        exg.l   D0,D1
*/

    if (r0 < r1) std::swap(r0, r1);
/*
@noXYSwap:
        move.l  dz,D2
        bpl.s   @noZNeg
        neg.l   D2
*/
    if (r2 < 0) r2 = -r2;
/*
@noZNeg:
        cmp.l   D1,D2
        blt.s   @noYZSwap
        exg.l   D1,D2
*/
    if (r1 < r2) std::swap(r1, r2);
/*
        cmp.l   D0,D1
        blt.s   @noXZSwap
        exg.l   D0,D1
*/

    if (r0 < r1) std::swap(r0, r1);
/*
@noXZSwap:
@noYZSwap:
        lsr.l   #1,D2
        add.l   D2,D1
        lsr.l   #1,D1
        add.l   D1,D0
*/
    r2 = r2 >> 1;
    r1 = r1 + r2;
    r1 = r1 >> 1;
    r0 = r0 + r1;

    return r0;
}

Fixed FMul(Fixed a, Fixed b) {
    return FixMul(a, b);
}

Fixed FDiv(Fixed a, Fixed b) {
    return FixDiv(a, b);
}

Fixed FMulDiv(Fixed a, Fixed b, Fixed c) {
    // return FDiv(FMul(a, b), c);
    return (long)(((double)a) * b / c);
}

Fixed FRadSin(Fixed a) {
    return (long)(65536L * sin(a / 65536.0));
}

Fixed FRadCos(Fixed a) {
    return (long)(65536L * cos(a / 65536.0));
}

Fixed FDegSin(Fixed a) {
    return (long)(65536.0 * sin(a / 3754936.206169363));
}

Fixed FDegCos(Fixed a) {
    return (long)(65536.0 * cos(a / 3754936.206169363));
}

Fixed FOneSin(Fixed a) {
    // Looks like sin(2pi * a)?
    return (long)(65536.0 * sin(a / 10430.3783505));
}

Fixed FOneCos(Fixed a) {
    return (long)(65536.0 * cos(a / 10430.3783505));
}

void Transpose(Matrix *a) {
    int i, j;
    Fixed temp;

    for (i = 0; i < 4; i++) {
        for (j = i + 1; j < 4; j++) {
            temp = (*a)[i][j];
            (*a)[i][j] = (*a)[j][i];
            (*a)[j][i] = temp;
        }
    }
}

Fixed FSqrt(Fixed n) {
    return ToFixed(sqrt(ToFloat(n)));
}

#define RANDCONST ((uint32_t)(0x41A7))
#define HIGH(x) ((uint16_t)(x >> 16))
#define LOW(x) ((uint16_t)x)

Fixed FRandom() {
    uint32_t temp;

    temp = RANDCONST * HIGH(FRandSeed) + HIGH(RANDCONST * LOW(FRandSeed));
    FRandSeed = ((temp & 0x7fff) << 16) + HIGH(temp << 1) + LOW(RANDCONST * FRandSeed);

    return LOW(FRandSeed);
}

/*
**  Move by xt, yt, zt
*/
void MTranslate(Fixed xt, Fixed yt, Fixed zt, Matrix *theMatrix) {
    (*theMatrix)[3][0] += xt;
    (*theMatrix)[3][1] += yt;
    (*theMatrix)[3][2] += zt;
}

void MRotateX(Fixed s, Fixed c, Matrix *theMatrix) {
    short i;
    Fixed *vm;
    Fixed *em;

    vm = &(*theMatrix)[0][0];
    em = vm + 16;

    do {
        Fixed t = vm[1];

        vm[1] = FMul(t, c) - FMul(vm[2], s);
        vm[2] = FMul(t, s) + FMul(vm[2], c);
        vm += 4;
    } while (vm < em);
}

void MRotateY(Fixed s, Fixed c, Matrix *theMatrix) {
    short i;

    for (i = 0; i < 4; i++) {
        Fixed t = (*theMatrix)[i][0];

        (*theMatrix)[i][0] = FMul(t, c) + FMul((*theMatrix)[i][2], s);
        (*theMatrix)[i][2] = FMul((*theMatrix)[i][2], c) - FMul(t, s);
    }
}

void MRotateZ(Fixed s, Fixed c, Matrix *theMatrix) {
    short i;

    for (i = 0; i < 4; i++) {
        Fixed t = (*theMatrix)[i][0];

        (*theMatrix)[i][0] = FMul(t, c) - FMul((*theMatrix)[i][1], s);
        (*theMatrix)[i][1] = FMul(t, s) + FMul((*theMatrix)[i][1], c);
    }
}

/*
**  If "t" is a matrix that only does translates and rotates,
**  the inverse is very easy to find. Find that inverse and
**  return the result in "i".
*/
void InverseTransform(Matrix *t, Matrix *i) {
    Vector temp;

    /*  Transpose 3x3 matrix
     */
    (*i)[0][0] = (*t)[0][0];
    (*i)[0][1] = (*t)[1][0];
    (*i)[0][2] = (*t)[2][0];
    (*i)[1][0] = (*t)[0][1];
    (*i)[1][1] = (*t)[1][1];
    (*i)[1][2] = (*t)[2][1];
    (*i)[2][0] = (*t)[0][2];
    (*i)[2][1] = (*t)[1][2];
    (*i)[2][2] = (*t)[2][2];

    (*i)[0][3] = (*i)[1][3] = (*i)[2][3] = 0;
    (*i)[3][3] = FIX(1);
    (*i)[3][0] = (*i)[3][1] = (*i)[3][2] = 0;

    /*  Find translate part of matrix:
     */
    VectorMatrixProduct(1, &((*t)[3]), &temp, i);
    (*i)[3][0] = -temp[0];
    (*i)[3][1] = -temp[1];
    (*i)[3][2] = -temp[2];
}

/*
**  Converts a unit quaternion into a rotation matrix.
**  Algorithm according to Ken Shoemake, SIGGraph-85
**  "Animating Rotation with Quaternion Curves"
*/
void QuaternionToMatrix(Quaternion *q, Matrix *m) {
    Fixed x2, y2, z2;
    Fixed wx2, yx2, zx2, xx2;
    Fixed wy2, yy2, zy2;
    Fixed wz2, zz2;

    x2 = q->x << 1;
    y2 = q->y << 1;
    z2 = q->z << 1;

    wx2 = FMul(x2, q->w);
    xx2 = FMul(x2, q->x);
    yx2 = FMul(x2, q->y);
    zx2 = FMul(x2, q->z);

    wy2 = FMul(y2, q->w);
    yy2 = FMul(y2, q->y);
    zy2 = FMul(y2, q->z);

    wz2 = FMul(z2, q->w);
    zz2 = FMul(z2, q->z);

    (*m)[0][0] = FIX(1) - yy2 - zz2;
    (*m)[1][1] = FIX(1) - xx2 - zz2;
    (*m)[2][2] = FIX(1) - xx2 - yy2;

    (*m)[0][1] = yx2 + wz2;
    (*m)[0][2] = zx2 - wy2;
    (*m)[1][0] = yx2 - wz2;
    (*m)[1][2] = zy2 + wx2;
    (*m)[2][0] = zx2 + wy2;
    (*m)[2][1] = zy2 - wx2;
}

#define EPSILON 4

/*
**  Convert a rotation matrix (really 3x3) into
**  quaternion format.
**
**  Algorithm according to Ken Shoemake, SIGGraph-85
**  "Animating Rotation with Quaternion Curves"
*/
void MatrixToQuaternion(Matrix *m, Quaternion *q) {
    Fixed squared;

    squared = 4 * (FIX(1) + (*m)[0][0] + (*m)[1][1] + (*m)[2][2]);

    if (squared > EPSILON) {
        q->w = FSqrt(squared);
        q->x = FDiv((*m)[1][2] - (*m)[2][1], q->w);
        q->y = FDiv((*m)[2][0] - (*m)[0][2], q->w);
        q->z = FDiv((*m)[0][1] - (*m)[1][0], q->w);

        q->w /= 4;
    } else {
        q->w = 0;
        squared = -(*m)[1][1] - (*m)[2][2];
        squared += squared;

        if (squared > EPSILON) {
            q->x = FSqrt(squared);
            q->y = FDiv((*m)[0][1], q->x);
            q->z = FDiv((*m)[0][2], q->x);
            q->x /= 2;
        } else {
            q->x = 0;
            squared = FIX(1) - (*m)[2][2];

            if (squared > EPSILON) {
                q->y = FSqrt(squared);
                q->z = FDiv((*m)[1][2], q->y);
                q->y /= 2;
            } else {
                q->y = 0;
                q->z = FIX(1);
            }
        }
    }
}

/*  Uses a Taylor series expansion: Not accurate
**  Instead of FRadArcSin and FRadArcCos, use FRadArcTan2 when you can!
*/
Fixed FRadArcSin(Fixed n) {
    return n + FMul(n, FMul(n, n)) / 6;
}

//  Uses the same expansion, but adjusts the result, of course
Fixed FRadArcCos(Fixed n) {
    return FHALFPI - n - FMul(n, FMul(n, n)) / 6;
}

Fixed FRadArcTan2(Fixed x, Fixed y) {
    Fixed absX, absY;
    Fixed ratio;
    Fixed result;

    if (x || y) {
        absX = x < 0 ? -x : x;
        absY = y < 0 ? -y : y;
        ratio = absX > absY ? FDivNZ(absY, absX) : FDivNZ(absX, absY);
        ratio += 1 << (15 - ARCUSTABLEBITS);
        ratio >>= 16 - ARCUSTABLEBITS;

        result = arcTanTable[ratio];

        if (absY > absX)
            result = FHALFPI - result;
        if (x < 0)
            result = FONEPI - result;
        if (y < 0)
            result = -result;
    } else {
        result = 0;
    }

    return result;
}

Fixed FOneArcTan2(Fixed x, Fixed y) {
    Fixed absX, absY;
    Fixed ratio;
    Fixed result;

    if (x || y) {
        absX = x < 0 ? -x : x;
        absY = y < 0 ? -y : y;
        ratio = absX > absY ? FDivNZ(absY, absX) : FDivNZ(absX, absY);
        ratio += 1 << (15 - ARCUSTABLEBITS);
        ratio >>= 16 - ARCUSTABLEBITS;

        result = arcTanOneTable[ratio];

        if (absY > absX)
            result = 0x4000L - result;
        if (x < 0)
            result = 0x8000L - result;
        if (y < 0)
            result = -result;
    } else
        result = 0;

    return result;
}

void FindSpaceAngle(Fixed *delta, Fixed *heading, Fixed *pitch) {
    Fixed headingA;
    Fixed sideLen;

    headingA = FOneArcTan2(delta[2], delta[0]);

    if ((headingA - 0x2000) & 0x4000)
        sideLen = FDivNZ(delta[2], FOneCos(headingA));
    else
        sideLen = FDivNZ(delta[0], FOneSin(headingA));

    *heading = headingA;
    *pitch = FOneArcTan2(sideLen, -delta[1]);
}

void FindSpaceAngleAndNormalize(Fixed *delta, Fixed *heading, Fixed *pitch) {
    Fixed headingA;
    Fixed pitchA;
    Fixed sideLen;
    Fixed tempCos, tempSin;

    headingA = FOneArcTan2(delta[2], delta[0]);

    tempCos = FOneCos(headingA);
    tempSin = FOneSin(headingA);

    if ((headingA - 0x2000) & 0x4000)
        sideLen = FDivNZ(delta[2], tempCos);
    else
        sideLen = FDivNZ(delta[0], tempSin);

    pitchA = FOneArcTan2(sideLen, delta[1]);

    *heading = headingA;
    *pitch = -pitchA;

    sideLen = FOneCos(pitchA);
    delta[2] = FMul(sideLen, tempCos);
    delta[1] = FOneSin(pitchA);
    delta[0] = FMul(sideLen, tempSin);
}

/*
    Given a matrix and minimum and maximum coordinates to define an axis-aligned
    box, the box is transformed and the eight corner vertices are stored in dest.

    To do this, six vectors have to be transformed:

    left    minX    0       0       ->  (minX * m[0][0], minX * m[1][0], minX * m[2][0])
    right   maxX    0       0       ->  (maxX * m[0][0], maxX * m[1][0], maxX * m[2][0])

    bottom  0       minY    0       ->  (minY * m[0][1], minY * m[1][1], minY * m[2][1])
    top     0       maxY    0       ->  (maxY * m[0][1], maxY * m[1][1], maxY * m[2][1])

    front   0       0       minZ    ->  (minZ * m[0][2], minZ * m[1][2], minZ * m[2][2])
    back    0       0       maxZ    ->  (maxZ * m[0][2], maxZ * m[1][2], maxZ * m[2][2])

    These are added in different combinations with the bottom row of the matrix
    (which defines the new origin) to produce the eight different corners.
*/

void TransformMinMax(Matrix *m, Fixed *min, Fixed *max, Vector *dest) {
    dest[0][0] = (Fixed)((((int64_t)min[0] * (int64_t)(*m)[0][0])) >> 16);
    dest[0][1] = (Fixed)((((int64_t)min[0] * (int64_t)(*m)[0][1])) >> 16);
    dest[0][2] = (Fixed)((((int64_t)min[0] * (int64_t)(*m)[0][2])) >> 16);
    dest[1][0] = (Fixed)((((int64_t)max[0] * (int64_t)(*m)[0][0])) >> 16);
    dest[1][1] = (Fixed)((((int64_t)max[0] * (int64_t)(*m)[0][1])) >> 16);
    dest[1][2] = (Fixed)((((int64_t)max[0] * (int64_t)(*m)[0][2])) >> 16);

    dest[2][0] = (Fixed)((((int64_t)min[1] * (int64_t)(*m)[1][0])) >> 16);
    dest[2][1] = (Fixed)((((int64_t)min[1] * (int64_t)(*m)[1][1])) >> 16);
    dest[2][2] = (Fixed)((((int64_t)min[1] * (int64_t)(*m)[1][2])) >> 16);
    dest[3][0] = (Fixed)((((int64_t)max[1] * (int64_t)(*m)[1][0])) >> 16);
    dest[3][1] = (Fixed)((((int64_t)max[1] * (int64_t)(*m)[1][1])) >> 16);
    dest[3][2] = (Fixed)((((int64_t)max[1] * (int64_t)(*m)[1][2])) >> 16);

    dest[4][0] = (Fixed)((((int64_t)min[2] * (int64_t)(*m)[2][0])) >> 16);
    dest[4][1] = (Fixed)((((int64_t)min[2] * (int64_t)(*m)[2][1])) >> 16);
    dest[4][2] = (Fixed)((((int64_t)min[2] * (int64_t)(*m)[2][2])) >> 16);
    dest[5][0] = (Fixed)((((int64_t)max[2] * (int64_t)(*m)[2][0])) >> 16);
    dest[5][1] = (Fixed)((((int64_t)max[2] * (int64_t)(*m)[2][1])) >> 16);
    dest[5][2] = (Fixed)((((int64_t)max[2] * (int64_t)(*m)[2][2])) >> 16);
}

void TransformBoundingBox(Matrix *m, Fixed *min, Fixed *max, Vector *dest) {
    Fixed *dp;
    Fixed f1 = FIX(1);
    Fixed x, y, z;
    Vector comp[6];

    TransformMinMax(m, min, max, comp);
    x = (*m)[3][0];
    y = (*m)[3][1];
    z = (*m)[3][2];

/*  The first part is now done and we have the six vectors transformed.
**  The rest is easy enough to with C.
*/
#define COMPCOPY(i, a, b, c, d) *dp++ = d + comp[a][i] + comp[b + 2][i] + comp[c + 4][i];
#define ALLCOMP(a, b, c) \
    COMPCOPY(0, a, b, c, x) \
    COMPCOPY(1, a, b, c, y) \
    COMPCOPY(2, a, b, c, z) \
    *dp++ = f1;
    dp = dest[0];

    ALLCOMP(0, 0, 0);
    ALLCOMP(1, 1, 1);
    ALLCOMP(0, 0, 1);
    ALLCOMP(1, 1, 0);
    ALLCOMP(0, 1, 0);
    ALLCOMP(1, 0, 1);
    ALLCOMP(1, 0, 0);
    ALLCOMP(0, 1, 1);
}

void FSquareAccumulate(Fixed n, int *acc) {
    //*acc += (ToFloat(n) * ToFloat(n));
    unsigned int *uacc = (unsigned int *)acc;
    int64_t wn = (int64_t)n * (int64_t)n;
    int64_t wacc = ((int64_t)uacc[0] << 32) | uacc[1];
    wacc += wn;
    acc[1] = wacc & 0xFFFFFFFF;
    acc[0] = wacc >> 32;
}

void FSquareSubtract(Fixed n, int *acc) {
    //*acc -= (ToFloat(n) * ToFloat(n));
    unsigned int *uacc = (unsigned int *)acc;
    int64_t wn = (int64_t)n * (int64_t)n;
    int64_t wacc = ((int64_t)uacc[0] << 32) | uacc[1];
    wacc -= wn;
    acc[1] = wacc & 0xFFFFFFFF;
    acc[0] = wacc >> 32;
}

/*
**  vd[i][j] = sum(k=0 to 3, vs[i][k] * m[k][j])
**  where   i=0 to n-1
**          j=0 to 3
**
**  This is effectively a vector to matrix multiplication
**  if n is 1 and a matrix with matrix multiplication when
**  n is 4.
*/
void VectorMatrixProduct(long n, Vector *vs, Vector *vd, Matrix *m) {
    int j, k;

    while (n--) {
        for (j = 0; j < 4; j++) {
            (*vd)[j] = 0;
            for (k = 0; k < 4; k++) {
                (*vd)[j] += FixMul((*vs)[k], (*m)[k][j]);
            }
        }
        vd++;
        vs++;
    }
}

void CombineTransforms(Matrix *vs, Matrix *vd, Matrix *m) {
    // BlockMoveData(vs, vd, sizeof(Matrix));
    VectorMatrixProduct(4, vs[0], vd[0], m);
}
