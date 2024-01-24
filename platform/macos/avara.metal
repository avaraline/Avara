//
//  avara.metal
//  Avara
//

#include <metal_stdlib>
#include "MetalShaderTypes.h"
using namespace metal;

typedef struct
{
    float3 position [[attribute(AttribPosition)]];
    float4 color [[attribute(AttribColor)]];
    float3 normal [[attribute(AttribNormal)]];
} Vertex;

typedef struct
{
    float4 position [[position]];
    float4 fragmentColor;
    float3 fragmentNormal;
} ColorInOut;

vertex ColorInOut vertexShader(Vertex in [[stage_in]],
                               constant ObjUniforms &u [[ buffer(1) ]])
{
    ColorInOut out;

    out.position = u.mvp * float4(in.position, 1.0);
    out.fragmentColor = in.color;
    out.fragmentNormal = in.normal * u.normal_transform;
    return out;
}

float3 diffuse_light(float3 normal, float3 light, float3 lightColor) {
    return max(dot(normal, light), 0.0) * lightColor;
}

fragment half4 fragmentShader(ColorInOut in [[stage_in]],
                              constant ObjUniforms &u [[ buffer(1) ]])
{
    float3 diffuse = diffuse_light(in.fragmentNormal, u.light0, u.light0Color) +
                     diffuse_light(in.fragmentNormal, u.light1, u.light1Color) +
                     diffuse_light(in.fragmentNormal, u.light2, u.light2Color) +
                     diffuse_light(in.fragmentNormal, u.light3, u.light3Color);
    
    float3 unlit = u.ambient * u.ambientColor;
    float3 lit = unlit + diffuse * u.lights_active;
    float4 color = float4(lit, 1.0) * in.fragmentColor;
    return half4(color);
}

typedef struct
{
    float3 position [[attribute(0)]];
} SkyVertex;

typedef struct
{
    float4 position [[position]];
    float3 tex_coord;
} SkyInOut;

vertex SkyInOut vtxSky(SkyVertex in [[stage_in]],
                       constant SkyUniforms &u [[ buffer(1) ]])
{
    SkyInOut out;

    out.position = u.proj * u.view * float4(in.position, 1.0);
    out.tex_coord = in.position;
    return out;
}

fragment half4 frgSky(SkyInOut in [[stage_in]],
                      constant SkyUniforms &u [[ buffer(1) ]])
{
    float phi = normalize(in.tex_coord).y;
    
    float3 color;
    if (phi > u.highAlt) {
        color = u.skyColor;
    } else if (phi <= 0.0) {
        color = u.groundColor;
    } else {
        float p = (phi / u.highAlt);
        color = u.skyColor * p + u.horizonColor * (1. - p);
    }
    return half4(half3(color), 1.0);
}
