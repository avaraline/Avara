#include "AbstractRenderer.h"

AbstractRenderer::AbstractRenderer()
{
    skyParams = new CWorldShader();
    skyParams->skyShadeCount = 12;

    viewParams = new CViewParameters();
    viewParams->hitherBound = FIX3(600);
    viewParams->yonBound = LONGYON;
    viewParams->horizonBound = FIX(16000); // 16 km
}

AbstractRenderer::~AbstractRenderer()
{
    if (viewParams) {
        delete viewParams;
    }
    if (skyParams) {
        delete skyParams;
    }
}

void AbstractRenderer::LevelReset()
{
    selectedPart = nullptr;
    skyParams->Reset();
    ResetLights();
}

void AbstractRenderer::ResetLights()
{
    viewParams->ambientLight = ToFixed(0.4f);
    viewParams->ambientLightColor = DEFAULT_LIGHT_COLOR;
    viewParams->SetLight(0, ToFixed(0.4f), ToFixed(45.0f), ToFixed(20.0f), DEFAULT_LIGHT_COLOR, kLightGlobalCoordinates);
    viewParams->SetLight(1, ToFixed(0.3f), ToFixed(20.0f), ToFixed(200.0f), DEFAULT_LIGHT_COLOR, kLightGlobalCoordinates);
    viewParams->SetLight(2, 0, 0, 0, DEFAULT_LIGHT_COLOR, kLightOff);
    viewParams->SetLight(3, 0, 0, 0, DEFAULT_LIGHT_COLOR, kLightOff);
    ApplyLights();
}

void AbstractRenderer::SetFOV(float fov)
{
    this->fov = fov;
    ApplyProjection();
}

void AbstractRenderer::SetSelectedPart(CSmartPart *part)
{
    selectedPart = part;
}

void AbstractRenderer::UpdateViewRect(int width, int height, float pixelRatio)
{
    viewParams->SetViewRect(width, height, width / 2, height / 2);
    viewParams->viewPixelRatio = pixelRatio;
    viewParams->CalculateViewPyramidCorners();
    ApplyProjection();
}

AbstractRenderer *gRenderer;
