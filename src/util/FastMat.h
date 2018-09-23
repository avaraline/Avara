/*
    Copyright ©1992-1996, Juri Munkki
    All rights reserved.

    File: FastMat.h
    Created: Friday, October 16, 1992, 23:55
    Modified: Wednesday, August 14, 1996, 14:13
*/

#pragma once

#include "Types.h"

#define PI 3.141592653589793238463

static const Fixed FIX1 = 0x00010000;
static inline float ToFloat(Fixed a) {
    return (float)a / FIX1;
}
static inline Fixed ToFixed(float a) {
    float temp = a * FIX1;
    return (Fixed)temp;
}
static inline float Deg2Rad(float deg) { 
    return deg * (PI / 180); 
}

void InitMatrix();
void CloseMatrix();

typedef Fixed Vector[4]; /*	A vector consists of 4 fixed point numbers: x,y,z,w	*/
typedef Vector Matrix[4]; /*	A matrix consists of 4 vectors.						*/
typedef struct {
    Fixed w, x, y, z;
} Quaternion;

void QuaternionToMatrix(Quaternion *q, Matrix *m);
void MatrixToQuaternion(Matrix *m, Quaternion *q);

/*	The following "structures" make copying vectors and matrices a lot easier.		*/
typedef struct {
    Vector theVec;
} VectorStruct;
typedef struct {
    Matrix theMatrix;
} MatrixStruct;

#define VECTORCOPY(a, b) *(VectorStruct *)a = *(VectorStruct *)b
#define MATRIXCOPY(a, b) *(MatrixStruct *)a = *(MatrixStruct *)b

#define MAXFIXED 0x7FFFffffL
#define MINFIXED 0x80000000L

/*	FIX3 results in the n/1000 as a fixed point number								*/
#define FIX3(n) ((long)((n)*8192L / 125L))
/*	FIX results in the integer number as a fixed point number						*/
#define FIX(n) ((long)(n * 65536L))

/*	Prototypes for internal routines:												*/
void VectorMatrixProduct(long n, Vector *vs, Vector *vd, Matrix *m);
void CombineTransforms(Matrix *vs, Matrix *vd, Matrix *m);

void TransformMinMax(Matrix *m, Fixed *min, Fixed *max, Vector *dest);
void TransformBoundingBox(Matrix *m, Fixed *min, Fixed *max, Vector *dest);

void OneMatrix(Matrix *OneMatrix);
void Transpose(Matrix *aMatrix);

void MRotateX(Fixed s, Fixed c, Matrix *theMatrix);
void MRotateY(Fixed s, Fixed c, Matrix *theMatrix);
void MRotateZ(Fixed s, Fixed c, Matrix *theMatrix);
void MTranslate(Fixed xt, Fixed yt, Fixed zt, Matrix *theMatrix);

Fixed FMul(Fixed a, Fixed b);
Fixed FDiv(Fixed a, Fixed b);
Fixed FMulDiv(Fixed a, Fixed b, Fixed c);

#define FDivNZ FDiv
#define FMulDivNZ FMulDiv
#define FMulDivV FMulDiv
#define FMulDivVNZ FMulDiv

#define FHALFPI 102944
#define FONEPI 205887
#define FTWOPI 411775

Fixed FDegSin(Fixed a); //	Full circle is 360.0 (fixed point)
Fixed FDegCos(Fixed a);
Fixed FRadSin(Fixed a); //	Full circle is 2 Pi (fixed point, of course)
Fixed FRadCos(Fixed a);
Fixed FOneSin(Fixed a); //	Full circle is 1.0 (fixed point)
Fixed FOneCos(Fixed a);
Fixed FRadArcCos(Fixed n); //	Instead of FRadArcSin and FRadArcCos use FRadArcTan2
Fixed FRadArcSin(Fixed n); //	when you can! ArcSin and ArcCos are for small values only.
Fixed FRadArcTan2(Fixed x, Fixed y);
Fixed FOneArcTan2(Fixed x, Fixed y);
void FindSpaceAngle(Fixed *delta, Fixed *heading, Fixed *pitch);
void FindSpaceAngleAndNormalize(Fixed *delta, Fixed *heading, Fixed *pitch);

#define FRadToDeg(angle) FMul(angle, 3754936)
#define FDegToRad(angle) FMul(angle, 1144)
#define FOneToDeg(angle) ((angle)*360)
#define FDegToOne(angle) ((angle) / 360)
#define FOneToRad(angle) FMul(angle, 41175)
#define FRadToOne(angle) FMul(angle, 10430)

Fixed FSqroot(int *ab);
Fixed FSqrt(Fixed n);

void FSquareAccumulate(Fixed n, int *acc);
void FSquareSubtract(Fixed n, int *acc);

Fixed VectorLength(long n, Fixed *v);
Fixed NormalizeVector(long n, Fixed *v); //	Returns length

Fixed FRandomBeta();
Fixed FRandom();
Fixed FDistanceEstimate(Fixed dx, Fixed dy, Fixed dz);
Fixed FDistanceOverEstimate(Fixed dx, Fixed dy, Fixed dz);
void InverseTransform(Matrix *trans, Matrix *inv);

Fixed DistanceEstimate(Fixed x1, Fixed y1, Fixed x2, Fixed y2);

static Fixed FRandSeed;
static Fixed FRandSeedBeta;
