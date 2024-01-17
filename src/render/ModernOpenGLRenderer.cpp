#include "ModernOpenGLRenderer.h"

#include "AssetManager.h"
#include "RenderManager.h"

#include <SDL2/SDL.h>

#define SKY_VERT "sky_vert.glsl"
#define SKY_FRAG "sky_frag.glsl"

#define OBJ_VERT "avara_vert.glsl"
#define OBJ_FRAG "avara_frag.glsl"

#define HUD_VERT "hud_vert.glsl"
#define HUD_FRAG "hud_frag.glsl"

ModernOpenGLRenderer::ModernOpenGLRenderer(RenderManager *manager)
{
    this->manager = manager;

    skyShader = LoadShader(SKY_VERT, SKY_FRAG);
    worldShader = LoadShader(OBJ_VERT, OBJ_FRAG);
    hudShader = LoadShader(HUD_VERT, HUD_FRAG);
}

ModernOpenGLRenderer::~ModernOpenGLRenderer()
{
    
}

void ModernOpenGLRenderer::RefreshWindow()
{
    SDL_GL_SwapWindow(manager->window);
}

std::unique_ptr<OpenGLShader> ModernOpenGLRenderer::LoadShader(const std::string &vertFile,
                                                               const std::string &fragFile)
{
    std::optional<std::string> vertPath, fragPath;

    vertPath = AssetManager::GetShaderPath(vertFile);
    fragPath = AssetManager::GetShaderPath(fragFile);

    if (!vertPath || !fragPath) {
        SDL_Log("Failed to find shader (%s, %s)", vertFile.c_str(), fragFile.c_str());
        exit(1);
    }

    return std::make_unique<OpenGLShader>(*vertPath, *fragPath);
}
