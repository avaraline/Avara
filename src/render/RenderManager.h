#pragma once

#include "CBSPPart.h"
#include "CBSPWorld.h"
#include "CHUD.h"
#include "CWorldShader.h"
#include "CViewParameters.h"

#include <SDL2/SDL.h>

#include <memory>

class RenderManager {
public:
    CViewParameters *viewParams;
    CWorldShader *skyParams;
    
    RenderManager(SDL_Window *window, NVGcontext *nvg);
    void AddHUDPart(CBSPPart *part);
    void AddPart(CBSPPart *part);
    void LevelReset();
    void OverheadPoint(Fixed *pt, Fixed *extent);
    void RemoveHUDPart(CBSPPart *part);
    void RemovePart(CBSPPart *part);
    void RenderFrame();
    void UpdateViewRect(int width, int height, float pixelRatio);
private:
    void ResetLights();

    SDL_Window *window;
    NVGcontext *nvg;
    std::shared_ptr<CHUD> ui;
    CBSPWorldImpl *staticWorld;
    CBSPWorldImpl *dynamicWorld;
    CBSPWorldImpl *hudWorld;
};

extern RenderManager *gRenderer;