#include "ModernOpenGLRenderer.h"

#include "RenderManager.h"

#include <SDL2/SDL.h>

ModernOpenGLRenderer::ModernOpenGLRenderer(RenderManager *manager)
{
    this->manager = manager;
}

ModernOpenGLRenderer::~ModernOpenGLRenderer()
{
    
}

void ModernOpenGLRenderer::RefreshWindow()
{
    SDL_GL_SwapWindow(manager->window);
}
