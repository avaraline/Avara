#include "MetalRenderer.h"

#include "AssetManager.h"
#include "ColorManager.h"
#include "FastMat.h"
#include "RenderManager.h"
#include "MetalVertices.h"
#include "MetalShaderTypes.h"
#include "MetalRenderInterface.h"
#include <simd/simd.h>
#include <SDL2/SDL.h>

matrix_float4x4 matrix_perspective_right_hand(float fovyRadians, float aspect)
{
    float nearZ = .099f;
    float farZ = 1000.f;
    float ys = 1 / tan(fovyRadians * 0.5);
    float xs = ys / aspect;
    float zs = farZ / (nearZ - farZ);

    return (matrix_float4x4) {{
        { -xs,   0,          0,  0 },
        {  0,  ys,          0,  0 },
        {  0,   0,         -zs, 1 },
        {  0,   0, nearZ * zs,  0 }
    }};
}

MetalRenderer::MetalRenderer(RenderManager *manager) {
    void *metalView = SDL_GetWindowData(manager->window, "metalView");
    void *metalLayer = SDL_Metal_GetLayer(metalView);

    this->manager = manager;
    this->_renderImpl = MetalRenderCreate(metalLayer, this->manager->nvg);
}

MetalRenderer::~MetalRenderer() {
}

void MetalRenderer::ApplyLights() {
}

void MetalRenderer::ApplyProjection() {
}

void MetalRenderer::ApplyView(){
}

void MetalRenderer::Clear() {
}

std::unique_ptr<VertexData> MetalRenderer::NewVertexDataInstance() {
    return std::make_unique<MetalVertices>();
}

void MetalRenderer::RefreshWindow() {
}

void MetalRenderer::RenderFrame() {
    Matrix *trans = &(manager->viewParams->viewMatrix);
    simd_float4x4 m;
    matrix_float4x4 proj = matrix_perspective_right_hand(manager->fov, manager->viewParams->viewPixelRatio);

    for (int c = 0; c < 4; c++) {
        m.columns[c] = simd_make_float4(ToFloat((*trans)[c][0]),
                                        ToFloat((*trans)[c][1]),
                                        ToFloat((*trans)[c][2]),
                                        ToFloat((*trans)[c][3]));
    }
    // Get rid of the translation part
    m.columns[3].xyz = 0;
    
    float groundColorRGB[3];
    float lowSkyColorRGB[3];
    float highSkyColorRGB[3];
    manager->skyParams->groundColor.ExportGLFloats(groundColorRGB, 3);
    manager->skyParams->lowSkyColor.ExportGLFloats(lowSkyColorRGB, 3);
    manager->skyParams->highSkyColor.ExportGLFloats(highSkyColorRGB, 3);

    SkyUniforms su = {
        .view = m,
        .proj = proj,
        .groundColor = simd_make_float3(groundColorRGB[0], groundColorRGB[1], groundColorRGB[2]),
        .horizonColor = simd_make_float3(lowSkyColorRGB[0], lowSkyColorRGB[1], lowSkyColorRGB[2]),
        .skyColor = simd_make_float3(highSkyColorRGB[0], highSkyColorRGB[1], highSkyColorRGB[2]),
        .lowAlt = ToFloat(manager->skyParams->lowSkyAltitude),
        .highAlt = ToFloat(manager->skyParams->highSkyAltitude)
    };

    MetalRenderSky(this->_renderImpl, &su);
}
