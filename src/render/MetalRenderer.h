
#pragma once

#include "CHUD.h"
#include "Renderer.h"
#include "RenderManager.h"
#include "VertexData.h"

#include <memory>

class RenderManager;

class MetalRenderer final: public Renderer {
public:
    MetalRenderer(RenderManager *manager);
    ~MetalRenderer();

    virtual void ApplyLights();
    virtual void ApplyProjection();
    virtual void ApplyView();
    virtual void Clear();
    virtual std::unique_ptr<VertexData> NewVertexDataInstance();
    virtual void RefreshWindow();
    virtual void RenderFrame();
    virtual bool UsesStaticWorld() { return false; };
private:
    RenderManager *manager;
    void *_renderImpl;
};
