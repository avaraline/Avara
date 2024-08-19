#include "AbstractRenderer.h"
#include <glm/gtc/matrix_transform.hpp>

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

void AbstractRenderer::UpdateViewRect(int width, int height, float pixelRatio)
{
    viewParams->SetViewRect(width, height, width / 2, height / 2);
    viewParams->viewPixelRatio = pixelRatio;
    viewParams->CalculateViewPyramidCorners();
    ApplyProjection();
}

glm::vec3 AbstractRenderer::ScreenSpaceToWorldSpace(glm::vec4 *ss_vec)
{
    glm::mat4 proj = glm::scale(
        glm::perspective(
            glm::radians(fov),
            (float)viewParams->viewPixelDimensions.h / (float)viewParams->viewPixelDimensions.v,
            0.099f,
            1000.0f
        ),
        glm::vec3(-1, 1, -1)
    );
    glm::mat4 v = ToFloatMat(viewParams->viewMatrix);
    glm::mat4 pv = glm::inverse(proj * v);
    return glm::vec3(pv * (*ss_vec));
}

AbstractRenderer *gRenderer;
