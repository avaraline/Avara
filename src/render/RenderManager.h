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

    /**
     * Add polygonal data for use in rendering the HUD.
     *
     * @param part The model we want to render.
     */
    void AddHUDPart(CBSPPart *part);

    /**
     * Add polygonal data for use in rendering the world.
     *
     * @param part The model we want to render.
     */
    void AddPart(CBSPPart *part);

    /**
     * Reset the render manager's state back to its defaults.
     */
    void LevelReset();

    /**
     * Populate the provided Fixed arrays with 1) the center point of the level and 2) how far the
     * level extends.
     *
     * @param pt The array of Fixed values (x, y, and z coordinates) to populate with the "center" point of a level.
     * @param extent The array of Fixed values (min X, max X, min Y, max Y, min Z, max Z) to populate with coordinates that represent the "extent" of a level's boundaries.
     */
    void OverheadPoint(Fixed *pt, Fixed *extent);

    /**
     * Update the game window with the latest rendered frame.
     */
    void RefreshWindow();

    /**
     * Remove polygonal data from HUD rendering.
     *
     * @param part The model we want to remove.
     */
    void RemoveHUDPart(CBSPPart *part);

    /**
     * Remove polygonal data from world rendering.
     *
     * @param part The model we want to remove.
     */
    void RemovePart(CBSPPart *part);

    /**
     * Execute all rendering instructions for the current frame.
     */
    void RenderFrame();

    /**
     * Update the camera's field of view.
     *
     * @param fov The field of view we want to use.
     */
    void SetFOV(float fov);

    /**
     * Update view parameters based on window resolution.
     *
     * @param width The width.
     * @param height The height.
     * @param pixelRatio The pixel ratio.
     */
    void UpdateViewRect(int width, int height, float pixelRatio);
private:
    /**
     * Reset light values back to their defaults.
     */
    void ResetLights();

    SDL_Window *window;
    NVGcontext *nvg;
    float fov = 50.0f;
    std::shared_ptr<CHUD> ui;
    CBSPWorldImpl *staticWorld;
    CBSPWorldImpl *dynamicWorld;
    CBSPWorldImpl *hudWorld;
};

extern RenderManager *gRenderer;
