#pragma once

#include "nanovg.h"

#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "FastMat.h"

GLuint LoadShaders(const char *vertex_file_path, const char *fragment_file_path);
void AvaraGLSetLight(int light, float intensity, float elevation, float azimuth);
void AvaraGLSetAmbient(float ambient);
void AvaraGLActivateLights(float active);
void AvaraGLSetView(glm::mat4 view);
void AvaraGLSetTransforms(Matrix *modelview, glm::mat4 normal_transform);
void AvaraGLLightDefaults();
void AvaraGLInitContext();
glm::mat4 FromFixedMat(Matrix *m);
extern GLuint gProgram;
