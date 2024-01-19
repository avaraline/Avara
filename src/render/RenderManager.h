#pragma once

#include "CBSPPart.h"
#include "CBSPWorld.h"
#include "CHUD.h"
#include "CWorldShader.h"
#include "CViewParameters.h"
#include "ModernOpenGLRenderer.h"
#include "NullRenderer.h"
#include "VertexData.h"

#include <SDL2/SDL.h>

#include <memory>

#ifdef __has_include
#  if __has_include(<optional>)                // Check for a standard library
#    include <optional>
#  elif __has_include(<experimental/optional>) // Check for an experimental version
#    include <experimental/optional>           // Check if __has_include is present
#  else                                        // Not found at all
#     error "Missing <optional>"
#  endif
#endif

enum struct RenderMode { Headless, GL3 };

class RenderManager final {
public:
    friend class ModernOpenGLRenderer;
    friend class NullRenderer;

    CViewParameters *viewParams;
    CWorldShader *skyParams;
    
    RenderManager(RenderMode mode, SDL_Window *window, std::optional<NVGcontext*> nvg = {});
    ~RenderManager();

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
     * Populate the provided two integers with the width and height coordinates of the window.
     *
     * @param w The int variable to hold the width.
     * @param h The int variable to hold the height.
     */
    void GetWindowSize(int &w, int &h);

    /**
     * Reset the render manager's state back to its defaults.
     */
    void LevelReset();

    /**
     * Creates and returns a new VertexData instance for use with the current renderer.
     *
     * @return a new, empty VertexData instance
     */
    std::unique_ptr<VertexData> NewVertexDataInstance();

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
     * Reset light values back to their defaults.
     */
    void ResetLights();

    /**
     * Update the camera's field of view.
     *
     * @param fov The field of view we want to use.
     */
    void SetFOV(float fov);

    /**
     * Update lights in the scene with the current configuration.
     */
    void UpdateLights();

    /**
     * Update the projection matrix for the currently configured resolution and FOV.
     */
    void UpdateProjection();

    /**
     * Update view parameters based on window resolution.
     *
     * @param pixelRatio The pixel ratio.
     */
    void UpdateViewRect(float pixelRatio);
private:
    SDL_Window *window;
    std::unique_ptr<Renderer> renderer;
    NVGcontext *nvg;
    float fov = 50.0f;
    std::unique_ptr<CHUD> ui;
    CBSPWorldImpl *staticWorld;
    CBSPWorldImpl *dynamicWorld;
    CBSPWorldImpl *hudWorld;
};

extern RenderManager *gRenderer;
