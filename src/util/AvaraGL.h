#pragma once

#include "nanovg.h"

#include <SDL2/SDL.h>
#include <glad/glad.h>

GLuint LoadShaders(const char *vertex_file_path, const char *fragment_file_path);
void AvaraGLSetLight(int light, float intensity, float elevation, float azimuth);
void AvaraGLSetAmbient(float ambient);
void AvaraGLActivateLights(float active);
void AvaraGLLightDefaults();
void AvaraGLInitContext();

extern GLuint gProgram;