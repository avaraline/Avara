#pragma once

#include "nanovg.h"

#include <SDL2/SDL.h>
#include <glad/glad.h>

GLuint LoadShaders(const char *vertex_file_path, const char *fragment_file_path);
void InitContext();

extern GLuint gProgram;
