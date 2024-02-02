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
    virtual void GetWindowSize(int &w, int &h) override;
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
    std::unique_ptr<OpenGLShader> hudShader;

    GLuint skyBuffer;
    GLuint skyVertArray;

    void AdjustAmbient(OpenGLShader &shader, float intensity);
    void ApplyView();
    void Clear();
    void Draw(OpenGLShader &shader, const CBSPPart &part);
    void IgnoreDirectionalLights(OpenGLShader &shader, bool yn);
    std::unique_ptr<OpenGLShader> LoadShader(const std::string &vertFile, const std::string &fragFile);
    void SetTransforms(const CBSPPart &part);
};
