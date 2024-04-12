#pragma once

#include "AbstractRenderer.h"
#include "CBSPPart.h"
#include "CBSPWorld.h"
#include "OpenGLShader.h"
#include "VertexData.h"

#include <SDL2/SDL.h>

#include <memory>

class ModernOpenGLRenderer final: public AbstractRenderer {
public:
    ModernOpenGLRenderer(SDL_Window *window);
    virtual ~ModernOpenGLRenderer() override;

    virtual void AddHUDPart(CBSPPart *part) override;
    virtual void AddPart(CBSPPart *part) override;
    virtual void ApplyLights() override;
    virtual void ApplyProjection() override;
    virtual void LevelReset() override;
    virtual std::unique_ptr<VertexData> NewVertexDataInstance() override;
    virtual void OverheadPoint(Fixed *pt, Fixed *extent) override;
    virtual void RefreshWindow() override;
    virtual void RemoveHUDPart(CBSPPart *part) override;
    virtual void RemovePart(CBSPPart *part) override;
    virtual void RenderFrame() override;
private:
    SDL_Window *window;
    
    CBSPWorldImpl *dynamicWorld;
    CBSPWorldImpl *hudWorld;
    
    std::unique_ptr<OpenGLShader> skyShader;
    std::unique_ptr<OpenGLShader> worldShader;
    std::unique_ptr<OpenGLShader> worldPostShader;
    std::unique_ptr<OpenGLShader> hudShader;
    std::unique_ptr<OpenGLShader> hudPostShader;
    std::unique_ptr<OpenGLShader> finalShader;

    GLsizei resolution[2];
    GLuint skyBuffer;
    GLuint skyVertArray;
    GLuint screenQuadBuffer;
    GLuint screenQuadVertArray;
    GLuint fbo[2];
    GLuint rbo[2];
    GLuint texture[2];

    void AdjustAmbient(OpenGLShader &shader, float intensity);
    void ApplyView();
    void Clear();
    void Draw(OpenGLShader &shader, const CBSPPart &part, float defaultAmbient, bool useAlphaBuffer = false);
    void IgnoreDirectionalLights(OpenGLShader &shader, bool yn);
    std::unique_ptr<OpenGLShader> LoadShader(const std::string &vertFile, const std::string &fragFile);
    void MakeFramebuffer(short index, GLsizei width, GLsizei height);
    void SetTransforms(const CBSPPart &part);
};
