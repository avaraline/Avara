#pragma once

#include "nanovg.h"

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "FastMat.h"
#include "Memory.h"

// Forward declaration of Avara fundamental drawing classes
class CBSPPart;
#include "CBSPPart.h"
class CWorldShader;
#include "CWorldShader.h"

#define GLAD_DEBUG

GLuint LoadShaders(const char *vertex_file_path, const char *fragment_file_path);
void AvaraGLSetLight(int light, float intensity, float elevation, float azimuth);
void AvaraGLSetAmbient(float ambient);
void AvaraGLSetView(glm::mat4 view);
void AvaraGLLightDefaults();
void AvaraGLInitContext();
void AvaraGLDrawPolygons(CBSPPart* part);
void AvaraGLShadeWorld(CWorldShader *theShader, CViewParameters *theView);
void AvaraGLToggleRendering(int active);

glm::mat4 FromFixedMat(Matrix *m);
extern GLuint gProgram;
