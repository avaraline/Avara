#pragma once

#include "CHUD.h"
#include "OpenGLShader.h"
#include "Renderer.h"
#include "RenderManager.h"

#include <memory>

class RenderManager;

class ModernOpenGLRenderer final: public Renderer {
public:
    ModernOpenGLRenderer(RenderManager *manager);
    ~ModernOpenGLRenderer();

    virtual void RefreshWindow();
    virtual bool UsesStaticWorld() { return true; };
private:
    RenderManager *manager;
    std::unique_ptr<OpenGLShader> skyShader;
    std::unique_ptr<OpenGLShader> worldShader;
    std::unique_ptr<OpenGLShader> hudShader;

    std::unique_ptr<OpenGLShader> LoadShader(const std::string &vertFile, const std::string &fragFile);
};
