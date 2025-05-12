#pragma once

#include "CBSPPart.h"
#include "CViewParameters.h"
#include "CWorldShader.h"
#include "VertexData.h"

#include <memory>

class AbstractRenderer {
public:
    CViewParameters *viewParams;
    CWorldShader *skyParams;

    AbstractRenderer();
    virtual ~AbstractRenderer();

    /**
     * Add polygonal data for use in rendering the HUD.
     *
     * @param part The model we want to render.
     */
    virtual void AddHUDPart(CBSPPart *part) = 0;

    /**
     * Add polygonal data for use in rendering the world.
     *
     * @param part The model we want to render.
     */
    virtual void AddPart(CBSPPart *part) = 0;

    /**
     * Update lights in the scene with the currently configured lighting.
     */
    virtual void ApplyLights() = 0;

    /**
     * Update the camera with the currently configured resolution and FOV.
     */
    virtual void ApplyProjection() = 0;
    
    /**
     * Update the sky in the scene with the currently configured parameters.
     */
    virtual void ApplySky() = 0;

    /**
     * Reset the renderer's state back to its defaults.
     */
    virtual void LevelReset();

    /**
     * Creates and returns a new VertexData instance for use with this renderer.
     *
     * @return a new, empty VertexData instance
     */
    virtual std::unique_ptr<VertexData> NewVertexDataInstance() = 0;
    
    /**
     * Perform desired actions after level load.
     */
    virtual void PostLevelLoad() {};

    /**
     * Update the game window with the latest rendered frame.
     */
    virtual void RefreshWindow() = 0;

    /**
     * Remove polygonal data from HUD rendering.
     *
     * @param part The model we want to remove.
     */
    virtual void RemoveHUDPart(CBSPPart *part) = 0;

    /**
     * Remove polygonal data from world rendering.
     *
     * @param part The model we want to remove.
     */
    virtual void RemovePart(CBSPPart *part) = 0;

    /**
     * Execute all rendering instructions for the current frame.
     */
    virtual void RenderFrame() = 0;

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
     * Update the camera with the currently configured view parameters.
     *
     * @param width The width in pixels.
     * @param height The height in pixels.
     * @param pixelRatio The pixel ratio.
     */
    virtual void UpdateViewRect(int width, int height, float pixelRatio) = 0;
protected:
    float fov = 50.0f;
};

extern AbstractRenderer *gRenderer;
