#pragma once

#include <simd/simd.h>

#ifdef __METAL_VERSION__
typedef float3 f3;
#else
typedef simd_float3 f3;
#endif

enum {
    AttribPosition = 0,
    AttribColor = 1,
    AttribNormal = 2,
};

typedef struct
{
    matrix_float4x4 mvp;
    matrix_float3x3 normal_transform;
    f3 light0;
    f3 light0Color;
    f3 light1;
    f3 light1Color;
    f3 light2;
    f3 light2Color;
    f3 light3;
    f3 light3Color;
    float ambient;
    f3 ambientColor;
    float lights_active;
} ObjUniforms;

typedef struct
{
    matrix_float4x4 view;
    matrix_float4x4 proj;
    f3 groundColor;
    f3 horizonColor;
    f3 skyColor;
    float lowAlt;
    float highAlt;
} SkyUniforms;
