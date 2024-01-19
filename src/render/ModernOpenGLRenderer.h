#pragma once

#include "CHUD.h"
#include "OpenGLShader.h"
#include "Renderer.h"
#include "RenderManager.h"
#include "VertexData.h"

#include <memory>

class RenderManager;

class ModernOpenGLRenderer final: public Renderer {
public:
    ModernOpenGLRenderer(RenderManager *manager);
    ~ModernOpenGLRenderer();

    virtual void ApplyLights();
    virtual void ApplyProjection();
    virtual void ApplyView();
    virtual void Clear();
    virtual std::unique_ptr<VertexData> NewVertexDataInstance();
    virtual void RefreshWindow();
    virtual void RenderSky();
    virtual void RenderStaticWorld();
    virtual void RenderDynamicWorld();
    virtual void RenderHUD();
    virtual bool UsesStaticWorld() { return false; };
private:
    RenderManager *manager;

    std::unique_ptr<OpenGLShader> skyShader;
    std::unique_ptr<OpenGLShader> worldShader;
    std::unique_ptr<OpenGLShader> hudShader;

    GLuint skyBuffer;
    GLuint skyVertArray;

    void AdjustAmbient(OpenGLShader &shader, float intensity);
    void Draw(OpenGLShader &shader, const CBSPPart &part);
    void IgnoreDirectionalLights(OpenGLShader &shader, bool yn);
    std::unique_ptr<OpenGLShader> LoadShader(const std::string &vertFile, const std::string &fragFile);
    void SetTransforms(const CBSPPart &part);
};
