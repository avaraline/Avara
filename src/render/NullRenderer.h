#pragma once

#include "AbstractRenderer.h"
#include "CBSPPart.h"
#include "VertexData.h"

#include <memory>

class NullRenderer final: public AbstractRenderer {
public:
    NullRenderer() : AbstractRenderer() {};

    virtual void AddHUDPart(CBSPPart *part) override {};
    virtual void AddPart(CBSPPart *part) override {};
    virtual void ApplyLights() override {};
    virtual void ApplyProjection() override {};
    virtual void ApplySky() override {};
    virtual std::unique_ptr<VertexData> NewVertexDataInstance() override {
        return nullptr;
    };
    virtual void RefreshWindow() override {};
    virtual void RemoveHUDPart(CBSPPart *part) override {};
    virtual void RemovePart(CBSPPart *part) override {};
    virtual void RenderFrame() override {};
    virtual void UpdateViewRect(int width, int height, float pixelRatio) override {};
};
