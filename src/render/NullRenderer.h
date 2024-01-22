#pragma once

#include "CHUD.h"
#include "Renderer.h"
#include "RenderManager.h"
#include "VertexData.h"

#include <memory>

class RenderManager;

class NullRenderer final: public Renderer {
public:
    NullRenderer(RenderManager *manager) {};
    ~NullRenderer() {};

    virtual void ApplyLights() {};
    virtual void ApplyProjection() {};
    virtual void ApplyView() {};
    virtual void Clear() {};
    virtual std::unique_ptr<VertexData> NewVertexDataInstance() { return nullptr; };
    virtual void RefreshWindow() {};
    virtual void RenderFrame() {};
    virtual bool UsesStaticWorld() { return false; };
};
