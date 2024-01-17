#pragma once

#include "CHUD.h"
#include "Renderer.h"
#include "RenderManager.h"

class RenderManager;

class ModernOpenGLRenderer final: public Renderer {
public:
    ModernOpenGLRenderer(RenderManager *manager);
    ~ModernOpenGLRenderer();

    virtual void RefreshWindow();
    virtual bool UsesStaticWorld() { return true; };
private:
    RenderManager *manager;
};
