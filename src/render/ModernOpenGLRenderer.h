#pragma once

#include "AbstractRenderer.h"
#include "CBSPPart.h"
#include "CBSPWorld.h"
#include "CCompoundShape.h"
#include "OpenGLShader.h"
#include "VertexData.h"

#include <SDL2/SDL.h>

#include <memory>
#include <vector>

class ModernOpenGLRenderer final: public AbstractRenderer {
public:
    ModernOpenGLRenderer(SDL_Window *window);
    virtual ~ModernOpenGLRenderer() override;

    virtual void AddHUDPart(CBSPPart *part) override;
    virtual void AddPart(CBSPPart *part) override;
    virtual void ApplyLights() override;
    virtual void ApplyPrefs(std::optional<std::string> name = std::optional<std::string>{}) override;
    virtual void ApplyProjection() override;
    virtual void ApplySky() override;
    virtual void LevelReset() override;
    virtual std::unique_ptr<VertexData> NewVertexDataInstance() override;
    virtual void PostLevelLoad() override;
    virtual void RefreshWindow() override;
    virtual void RemoveHUDPart(CBSPPart *part) override;
    virtual void RemovePart(CBSPPart *part) override;
    virtual void RenderFrame() override;
    virtual void UpdateViewRect(int width, int height, float pixelRatio) override;
private:
    SDL_Window *window;
    
    std::unique_ptr<CCompoundShape> staticGeometry = nullptr;
    CBSPWorldImpl *staticWorld;
    CBSPWorldImpl *dynamicWorld;
    CBSPWorldImpl *hudWorld;
    
    std::unique_ptr<OpenGLShader> skyShader;
    std::unique_ptr<OpenGLShader> worldShader;
    std::unique_ptr<OpenGLShader> worldPostShader;
    std::unique_ptr<OpenGLShader> hudShader;
    std::unique_ptr<OpenGLShader> hudPostShader;
    std::unique_ptr<OpenGLShader> finalShader;
    
    std::vector<CBSPPart*> alphaParts;

    GLsizei resolution[2];
    GLuint skyBuffer;
    GLuint skyVertArray;
    GLuint screenQuadBuffer;
    GLuint screenQuadVertArray;
    GLuint fbo[2];
    GLuint rbo[2];
    GLuint texture[2];

    bool showSpecular, dither, fxaa;

    void AdjustAmbient(OpenGLShader &shader, float intensity);
    void ApplyView();
    void BlendingOff();
    void BlendingOn();
    void Clear();
    void Draw(OpenGLShader &shader, const CBSPPart &part, float defaultAmbient, bool useAlphaBuffer = false);
    void IgnoreDirectionalLights(OpenGLShader &shader, bool yn);
    std::unique_ptr<OpenGLShader> LoadShader(const std::string &vertFile, const std::string &fragFile);
    void AdjustFramebuffer(short index, GLsizei width, GLsizei height);
    void SetPositions(OpenGLShader &shader);
    void SetTransforms(const CBSPPart &part);
};
