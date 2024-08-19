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
     * Populate the provided Fixed arrays with 1) the center point of the level and 2) how far the
     * level extends.
     *
     * @param pt The array of Fixed values (x, y, and z coordinates) to populate with the "center" point of a level.
     * @param extent The array of Fixed values (min X, max X, min Y, max Y, min Z, max Z) to populate with coordinates that represent the "extent" of a level's boundaries.
     */
    virtual void OverheadPoint(Fixed *pt, Fixed *extent) = 0;

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
    
    /**
     * Get a worldspace vector from a screenspace vector
     *
     * @param ss_vec The screenspace vector
     */
    virtual glm::vec3 ScreenSpaceToWorldSpace(glm::vec4 *ss_vec);
protected:
    float fov = 50.0f;
};

extern AbstractRenderer *gRenderer;
