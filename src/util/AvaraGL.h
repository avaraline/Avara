#pragma once

#include "nanovg.h"

#include <SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "FastMat.h"
#include "Memory.h"

// Forward declaration of Avara fundamental drawing classes
class CBSPPart;
#include "CBSPPart.h"
class CWorldShader;
#include "CWorldShader.h"

#define DEFAULT_LIGHT_COLOR 0xffffffff

#define GLAD_DEBUG


GLuint LoadShaders(const char *vertex_file_path, const char *fragment_file_path);
void AvaraGLSetLight(int light, float intensity, float elevation, float azimuth, long color);
void AvaraGLSetDepthTest(bool doTest);
void AvaraGLSetAmbient(float ambient, long color);
void AvaraGLSetView(glm::mat4 view);
void AvaraGLSetFOV(float fov);
void AvaraGLUpdateProjectionMatrix();
void AvaraGLLightDefaults();
void AvaraGLInitContext();
void AvaraGLViewport(short width, short height);
void AvaraGLDrawPolygons(CBSPPart* part);
void AvaraGLShadeWorld(CWorldShader *theShader, CViewParameters *theView);
void AvaraGLToggleRendering(int active);
bool AvaraGLIsRendering();

static inline glm::mat4 ToFloatMat(Matrix *m) {
    glm::mat4 mat(1.0);
    for (int i = 0; i < 3; i ++) {
        mat[0][i] = ToFloat((*m)[0][i]);
        mat[1][i] = ToFloat((*m)[1][i]);
        mat[2][i] = ToFloat((*m)[2][i]);
        mat[3][i] = ToFloat((*m)[3][i]);
    }
    return mat;
}

extern GLuint gProgram;
