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
    static CViewParameters *viewParams;
    static CWorldShader *skyParams;
    
    static void Init(SDL_Window *window, NVGcontext *nvg);
    static void AddHUDPart(CBSPPart *part);
    static void AddPart(CBSPPart *part);
    static void LevelReset();
    static void OverheadPoint(Fixed *pt, Fixed *extent);
    static void RemoveHUDPart(CBSPPart *part);
    static void RemovePart(CBSPPart *part);
    static void RenderFrame();
    static void UpdateViewRect(int width, int height, float pixelRatio);
private:
    static void ResetLights();

    static SDL_Window *window;
    static NVGcontext *nvg;
    static std::shared_ptr<CHUD> ui;
    static CBSPWorldImpl *staticWorld;
    static CBSPWorldImpl *dynamicWorld;
    static CBSPWorldImpl *hudWorld;
};
