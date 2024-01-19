#include "RenderManager.h"

#include "AvaraDefines.h"
#include "CAvaraGame.h"
#include "FastMat.h"
#include "Preferences.h"

RenderManager::RenderManager(RenderMode mode, SDL_Window *window, std::optional<NVGcontext*> nvg)
{
    this->window = window;
    if (nvg) {
        this->nvg = *nvg;
        if (this->nvg) {
            ui = std::make_unique<CHUD>(gCurrentGame);
            ui->LoadImages(this->nvg);
        }
    }

    skyParams = new CWorldShader();
    skyParams->skyShadeCount = 12;

    viewParams = new CViewParameters();
    viewParams->hitherBound = FIX3(600);
    viewParams->yonBound = LONGYON;
    viewParams->horizonBound = FIX(16000); // 16 km

    staticWorld = new CBSPWorldImpl(100);
    dynamicWorld = new CBSPWorldImpl(100);
    hudWorld = new CBSPWorldImpl(30);

    switch (mode) {
        case RenderMode::GL3:
            renderer = std::make_unique<ModernOpenGLRenderer>(this);
            break;
        case RenderMode::Headless:
            renderer = std::make_unique<NullRenderer>(this);
            break;
    }

    ResetLights();
}

RenderManager::~RenderManager()
{
    delete skyParams;
    delete viewParams;
    delete staticWorld;
    delete dynamicWorld;
    delete hudWorld;
}

void RenderManager::AddHUDPart(CBSPPart *part)
{
    hudWorld->AddPart(part);
}

void RenderManager::AddPart(CBSPPart *part)
{
    dynamicWorld->AddPart(part);
}

void RenderManager::GetWindowSize(int &w, int &h)
{
    SDL_GetWindowSize(window, &w, &h);
}

void RenderManager::LevelReset()
{
    skyParams->Reset();
    staticWorld->DisposeParts();
    dynamicWorld->DisposeParts();
    hudWorld->DisposeParts();
    ResetLights();
}

std::unique_ptr<VertexData> RenderManager::NewVertexDataInstance()
{
    return renderer->NewVertexDataInstance();
}

void RenderManager::OverheadPoint(Fixed *pt, Fixed *extent)
{
    dynamicWorld->OverheadPoint(pt, extent);
}

void RenderManager::RefreshWindow()
{
    renderer->RefreshWindow();
}

void RenderManager::RemoveHUDPart(CBSPPart *part)
{
    hudWorld->RemovePart(part);
}

void RenderManager::RemovePart(CBSPPart *part)
{
    dynamicWorld->RemovePart(part);
}

void RenderManager::RenderFrame()
{
    renderer->Clear();
    renderer->ApplyView();
    renderer->RenderSky();
    if (renderer->UsesStaticWorld()) {
        renderer->RenderStaticWorld();
    }
    renderer->RenderDynamicWorld();
    renderer->RenderHUD();
}

void RenderManager::ResetLights()
{
    viewParams->ambientLight = 0.4f;
    viewParams->ambientLightColor = DEFAULT_LIGHT_COLOR;
    viewParams->SetLight(0, 0.4f, 45.0f, 20.0f, DEFAULT_LIGHT_COLOR, kLightGlobalCoordinates);
    viewParams->SetLight(1, 0.3f, 20.0f, 200.0f, DEFAULT_LIGHT_COLOR, kLightGlobalCoordinates);
    viewParams->SetLight(2, 0, 0, 0, DEFAULT_LIGHT_COLOR, kLightOff);
    viewParams->SetLight(3, 0, 0, 0, DEFAULT_LIGHT_COLOR, kLightOff);
    UpdateLights();
}

void RenderManager::SetFOV(float fov)
{
    this->fov = fov;
    UpdateProjection();
}

void RenderManager::UpdateLights()
{
    renderer->ApplyLights();
}

void RenderManager::UpdateProjection()
{
    renderer->ApplyProjection();
}

void RenderManager::UpdateViewRect(float pixelRatio)
{
    int w, h;
    GetWindowSize(w, h);
    
    viewParams->SetViewRect(w, h, w / 2, h / 2);
    viewParams->viewPixelRatio = pixelRatio;
    viewParams->CalculateViewPyramidCorners();
    UpdateProjection();
}

RenderManager *gRenderer;
