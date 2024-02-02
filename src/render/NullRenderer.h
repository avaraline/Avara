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
    virtual void GetWindowSize(int &w, int &h) override {
        w = 0;
        h = 0;

    };
    virtual std::unique_ptr<VertexData> NewVertexDataInstance() override {
        return nullptr;
    };
    virtual void OverheadPoint(Fixed *pt, Fixed *extent) override {
        pt[0] = 0;
        pt[1] = 0;
        pt[2] = 0;
        extent[0] = 0;
        extent[1] = 0;
        extent[2] = 0;
        extent[3] = 0;
        extent[4] = 0;
        extent[5] = 0;
        
    };
    virtual void RefreshWindow() override {};
    virtual void RemoveHUDPart(CBSPPart *part) override {};
    virtual void RemovePart(CBSPPart *part) override {};
    virtual void RenderFrame() override {};
};
