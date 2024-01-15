#include "RenderManager.h"

#include "AvaraDefines.h"
#include "CAvaraGame.h"
#include "FastMat.h"

// Create static variables.
SDL_Window *RenderManager::window = nullptr;
NVGcontext *RenderManager::nvg = nullptr;
std::shared_ptr<CHUD> RenderManager::ui = nullptr;
CWorldShader *RenderManager::skyParams = nullptr;
CViewParameters *RenderManager::viewParams = nullptr;
CBSPWorldImpl *RenderManager::staticWorld = nullptr;
CBSPWorldImpl *RenderManager::dynamicWorld = nullptr;
CBSPWorldImpl *RenderManager::hudWorld = nullptr;

void RenderManager::Init(SDL_Window *window, NVGcontext *nvg)
{
    RenderManager::window = window;
    RenderManager::nvg = nvg;

    ui = std::make_shared<CHUD>(gCurrentGame);
    ui->LoadImages(nvg);

    skyParams = new CWorldShader();
    skyParams->skyShadeCount = 12;

    viewParams = new CViewParameters();
    viewParams->hitherBound = FIX3(600);
    viewParams->yonBound = LONGYON;
    viewParams->horizonBound = FIX(16000); // 16 km
    ResetLights();

    staticWorld = new CBSPWorldImpl(100);
    dynamicWorld = new CBSPWorldImpl(100);
    hudWorld = new CBSPWorldImpl(30);
}

void RenderManager::AddHUDPart(CBSPPart *part)
{
    hudWorld->AddPart(part);
}

void RenderManager::AddPart(CBSPPart *part)
{
    dynamicWorld->AddPart(part);
}

void RenderManager::LevelReset()
{
    skyParams->Reset();
    staticWorld->DisposeParts();
    dynamicWorld->DisposeParts();
    hudWorld->DisposeParts();
    ResetLights();
}

void RenderManager::OverheadPoint(Fixed *pt, Fixed *extent)
{
    dynamicWorld->OverheadPoint(pt, extent);
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
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    AvaraGLShadeWorld(skyParams, viewParams);
    
    staticWorld->Render(Shader::World);
    dynamicWorld->Render(Shader::World);
    AvaraGLSetDepthTest(false);
    hudWorld->Render(Shader::HUD);

    if (true /* TODO: lookup/store showNewHud setting */) {
        ui->RenderNewHUD(nvg);
    } else {
        ui->Render(nvg);
    }

    AvaraGLSetDepthTest(true);

    // SDL_GL_SwapWindow(window);
}

void RenderManager::UpdateViewRect(int width, int height, float pixelRatio)
{
    viewParams->SetViewRect(width, height, width / 2, height / 2);
    viewParams->viewPixelRatio = pixelRatio;
    viewParams->CalculateViewPyramidCorners();
}

void RenderManager::ResetLights()
{
    viewParams->ambientLight = 0.4f;
    viewParams->ambientLightColor = DEFAULT_LIGHT_COLOR;
    viewParams->SetLight(0, 0.4f, 45.0f, 20.0f, DEFAULT_LIGHT_COLOR, kLightGlobalCoordinates);
    viewParams->SetLight(1, 0.3f, 20.0f, 200.0f, DEFAULT_LIGHT_COLOR, kLightGlobalCoordinates);
    viewParams->SetLight(2, 0, 0, 0, DEFAULT_LIGHT_COLOR, kLightOff);
    viewParams->SetLight(3, 0, 0, 0, DEFAULT_LIGHT_COLOR, kLightOff);
}
