#pragma once

#include "VertexData.h"

#include <memory>

class Renderer {
public:
    virtual ~Renderer() {};

    /**
     * Update lights in the scene with the currently configured lighting.
     */
    virtual void ApplyLights() = 0;

    /**
     * Update the camera with the currently configured resolution and FOV.
     */
    virtual void ApplyProjection() = 0;

    /**
     * Update the camera with the currently configured view parameters.
     */
    virtual void ApplyView() = 0;

    /**
     * Clear the screen.
     */
    virtual void Clear() = 0;

    /**
     * Creates and returns a new VertexData instance for use with this renderer.
     *
     * @return a new, empty VertexData instance
     */
    virtual std::unique_ptr<VertexData> NewVertexDataInstance() = 0;

    /**
     * Update the game window with the latest rendered frame.
     */
    virtual void RefreshWindow() = 0;

    /**
     * Render the skybox.
     */
    virtual void RenderSky() = 0;

    /**
     * Render the static world elements.
     */
    virtual void RenderStaticWorld() = 0;

    /**
     * Render the dynamic world elements.
     */
    virtual void RenderDynamicWorld() = 0;

    /**
     * Render the HUD.
     */
    virtual void RenderHUD() = 0;
    
    /**
     * Check to see if this renderer supports static world objects.
     *
     * @return whether the renderer supports a static world
     */
    virtual bool UsesStaticWorld() = 0;
};