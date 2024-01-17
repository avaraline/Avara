#pragma once

#include "CHUD.h"
#include "Renderer.h"
#include "RenderManager.h"

class RenderManager;

class NullRenderer final: public Renderer {
public:
    NullRenderer(RenderManager *manager): manager(manager) {};
    ~NullRenderer() {};

    virtual void RefreshWindow() {};
    virtual bool UsesStaticWorld() { return false; };
private:
    RenderManager *manager;
};
