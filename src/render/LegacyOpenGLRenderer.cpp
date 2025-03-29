#include "LegacyOpenGLRenderer.h"

#include "AssetManager.h"
#include "ColorManager.h"
#include "FastMat.h"
#include "OpenGLVertices.h"

#include <glm/gtc/matrix_transform.hpp>

#include <SDL2/SDL.h>

#define SKY_VERT "sky_vert.glsl"
#define SKY_FRAG "sky_frag.glsl"

#define OBJ_VERT "world_vert.glsl"
#define OBJ_FRAG "world_frag.glsl"

const float legacySkyboxVertices[] = {
    -5.0f,  5.0f, -5.0f,
    -5.0f, -5.0f, -5.0f,
     5.0f, -5.0f, -5.0f,
     5.0f, -5.0f, -5.0f,
     5.0f,  5.0f, -5.0f,
    -5.0f,  5.0f, -5.0f,

    -5.0f, -5.0f,  5.0f,
    -5.0f, -5.0f, -5.0f,
    -5.0f,  5.0f, -5.0f,
    -5.0f,  5.0f, -5.0f,
    -5.0f,  5.0f,  5.0f,
    -5.0f, -5.0f,  5.0f,

     5.0f, -5.0f, -5.0f,
     5.0f, -5.0f,  5.0f,
     5.0f,  5.0f,  5.0f,
     5.0f,  5.0f,  5.0f,
     5.0f,  5.0f, -5.0f,
     5.0f, -5.0f, -5.0f,

    -5.0f, -5.0f,  5.0f,
    -5.0f,  5.0f,  5.0f,
     5.0f,  5.0f,  5.0f,
     5.0f,  5.0f,  5.0f,
     5.0f, -5.0f,  5.0f,
    -5.0f, -5.0f,  5.0f,

    -5.0f,  5.0f, -5.0f,
     5.0f,  5.0f, -5.0f,
     5.0f,  5.0f,  5.0f,
     5.0f,  5.0f,  5.0f,
    -5.0f,  5.0f,  5.0f,
    -5.0f,  5.0f, -5.0f,

    -5.0f, -5.0f, -5.0f,
    -5.0f, -5.0f,  5.0f,
     5.0f, -5.0f, -5.0f,
     5.0f, -5.0f, -5.0f,
    -5.0f, -5.0f,  5.0f,
     5.0f, -5.0f,  5.0f
};

const char *__glGetErrorString(GLenum error)
{
    switch (error) {
        case GL_NO_ERROR:
            return "No Error";
        case GL_INVALID_ENUM:
            return "Invalid Enum";
        case GL_INVALID_VALUE:
            return "Invalid Value";
        case GL_INVALID_OPERATION:
            return "Invalid Operation";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "Invalid Framebuffer Operation";
        case GL_OUT_OF_MEMORY:
            return "Out of Memory";
        case GL_STACK_UNDERFLOW:
            return "Stack Underflow";
        case GL_STACK_OVERFLOW:
            return "Stack Overflow";
        // case GL_CONTEXT_LOST:
        //     return "Context Lost";
        default:
            return "Unknown Error";
    }
}

void ___glCheckErrors(const char *filename, int line)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
        SDL_Log("OpenGL Error: %s (%d) [%u] %s\n", filename, line, err, __glGetErrorString(err));
}
#define __glCheckErrors() ___glCheckErrors(__FILE__, __LINE__)

inline glm::mat4 ToFloatMat(const Matrix &m)
{
    glm::mat4 mat(1.0);
    for (int i = 0; i < 3; i ++) {
        mat[0][i] = ToFloat(m[0][i]);
        mat[1][i] = ToFloat(m[1][i]);
        mat[2][i] = ToFloat(m[2][i]);
        mat[3][i] = ToFloat(m[3][i]);
    }
    return mat;
}

LegacyOpenGLRenderer::LegacyOpenGLRenderer(SDL_Window *window) : AbstractRenderer()
{
    this->window = window;

    GLsizei w, h;
    SDL_GL_GetDrawableSize(window, &w, &h);
    viewParams->viewPixelDimensions.h = w;
    viewParams->viewPixelDimensions.v = h;

    dynamicWorld = new CBSPWorldImpl(100);
    hudWorld = new CBSPWorldImpl(30);

    // Initialize shaders.
    skyShader = LoadShader(SKY_VERT, SKY_FRAG);
    worldShader = LoadShader(OBJ_VERT, OBJ_FRAG);
    ApplyLights();
    ApplyProjection();
    
    alphaParts = {};

    // Create a separate VBO and VAO for the skybox, and upload its geometry to the GPU.
    glGenVertexArrays(1, &skyVertArray);
    glGenBuffers(1, &skyBuffer);
    __glCheckErrors();
    glBindVertexArray(skyVertArray);
    __glCheckErrors();
    glBindBuffer(GL_ARRAY_BUFFER, skyBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(legacySkyboxVertices), legacySkyboxVertices, GL_STATIC_DRAW);

    __glCheckErrors();
    // Rebind to default VBO/VAO.
    glBindVertexArray(0);
    __glCheckErrors();
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE);
    __glCheckErrors();

}

LegacyOpenGLRenderer::~LegacyOpenGLRenderer() {
    delete dynamicWorld;
    delete hudWorld;
    AbstractRenderer::~AbstractRenderer();
}

void LegacyOpenGLRenderer::AddHUDPart(CBSPPart *part)
{
    part->ignoreDirectionalLights = true;
    part->ignoreDepthTesting = true;
    hudWorld->AddPart(part);
}

void LegacyOpenGLRenderer::AddPart(CBSPPart *part)
{
    dynamicWorld->AddPart(part);
}

void LegacyOpenGLRenderer::ApplyLights()
{
    float ambientIntensity = ToFloat(viewParams->ambientLight);
    float ambientRGB[3];
    viewParams->ambientLightColor.ExportGLFloats(ambientRGB, 3);

    worldShader->Use();
    AdjustAmbient(*worldShader, ambientIntensity);
    worldShader->SetFloat3("ambientColor", ambientRGB);

    for (int i = 0; i < 4; i++) {
        float intensity = ToFloat(viewParams->dirLightSettings[i].intensity);
        float elevation = ToFloat(viewParams->dirLightSettings[i].angle1);
        float azimuth = ToFloat(viewParams->dirLightSettings[i].angle2);
        float rgb[3];
        viewParams->dirLightSettings[i].color.ExportGLFloats(rgb, 3);

        float xyz[3] = {
            sin(Deg2Rad(-azimuth)) * intensity,
            sin(Deg2Rad(-elevation)) * intensity,
            cos(Deg2Rad(azimuth)) * intensity
        };

        switch (i) {
            case 0:
                worldShader->SetFloat3("light0", xyz);
                worldShader->SetFloat3("light0Color", rgb);
                break;
            case 1:
                worldShader->SetFloat3("light1", xyz);
                worldShader->SetFloat3("light1Color", rgb);
                break;
            case 2:
                worldShader->SetFloat3("light2", xyz);
                worldShader->SetFloat3("light2Color", rgb);
                break;
            case 3:
                worldShader->SetFloat3("light3", xyz);
                worldShader->SetFloat3("light3Color", rgb);
                break;
        }
    }
}

void LegacyOpenGLRenderer::ApplyProjection()
{
    SDL_GL_GetDrawableSize(this->window, &resolution[0], &resolution[1]);

    glm::mat4 proj = glm::scale(
        glm::perspective(
            glm::radians(fov),
            (float)viewParams->viewPixelDimensions.h / (float)viewParams->viewPixelDimensions.v,
            0.099f,
            1000.0f
        ),
        glm::vec3(-1, 1, -1)
    );

    skyShader->Use();
    skyShader->SetMat4("proj", proj);
    __glCheckErrors();

    worldShader->Use();
    worldShader->SetMat4("proj", proj);
    __glCheckErrors();
}

void LegacyOpenGLRenderer::ApplySky()
{
    float groundColorRGB[3];
    float lowSkyColorRGB[3];
    float highSkyColorRGB[3];
    skyParams->groundColor.ExportGLFloats(groundColorRGB, 3);
    skyParams->lowSkyColor.ExportGLFloats(lowSkyColorRGB, 3);
    skyParams->highSkyColor.ExportGLFloats(highSkyColorRGB, 3);
    
    skyShader->Use();
    skyShader->SetFloat3("groundColor", groundColorRGB);
    skyShader->SetFloat3("horizonColor", lowSkyColorRGB);
    skyShader->SetFloat3("skyColor", highSkyColorRGB);
    skyShader->SetFloat("highAlt", ToFloat(skyParams->highSkyAltitude) / 20000.0f);
}

void LegacyOpenGLRenderer::UpdateViewRect(int width, int height, float pixelRatio)
{
    AbstractRenderer::UpdateViewRect(width, height, pixelRatio);
    ApplyProjection();
}

void LegacyOpenGLRenderer::LevelReset()
{
    dynamicWorld->DisposeParts();
    hudWorld->DisposeParts();
    AbstractRenderer::LevelReset();
}

std::unique_ptr<VertexData> LegacyOpenGLRenderer::NewVertexDataInstance()
{
    return std::make_unique<OpenGLVertices>();
}

void LegacyOpenGLRenderer::OverheadPoint(Fixed *pt, Fixed *extent)
{
    dynamicWorld->OverheadPoint(pt, extent);
}

void LegacyOpenGLRenderer::RefreshWindow()
{
    SDL_GL_SwapWindow(window);
}

void LegacyOpenGLRenderer::RemoveHUDPart(CBSPPart *part)
{
    hudWorld->RemovePart(part);
}

void LegacyOpenGLRenderer::RemovePart(CBSPPart *part)
{
    dynamicWorld->RemovePart(part);
}

void LegacyOpenGLRenderer::RenderFrame()
{
    Clear();
    ApplyView();

    // RENDER SKYBOX ///////////////////////////////////////////////////////////////////////////////

    Matrix *trans = &(viewParams->viewMatrix);

    // Get rid of the view translation.
    glm::mat4 glMatrix = ToFloatMat(*trans);
    glMatrix[3][0] = glMatrix[3][1] = glMatrix[3][2] = 0;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glBindVertexArray(skyVertArray);
    glBindBuffer(GL_ARRAY_BUFFER, skyBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);
    glEnableVertexAttribArray(0);

    skyShader->Use();
    skyShader->SetMat4("view", glMatrix);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(legacySkyboxVertices));
    glDisableVertexAttribArray(0);

    __glCheckErrors();
    
    // RENDER WORLD ////////////////////////////////////////////////////////////////////////

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);

    worldShader->Use();

    dynamicWorld->PrepareForRender();

    __glCheckErrors();
    float defaultAmbient = ToFloat(viewParams->ambientLight);

    // Draw opaque geometry. Parts are sorted far-to-near, so traverse in
    // reverse so the depth buffer can help out.
    auto partList = dynamicWorld->GetVisiblePartListPointer();
    auto partCount = dynamicWorld->GetVisiblePartCount();
    CBSPPart **partPtr = partList + (partCount - 1);
    alphaParts.clear();
    for (uint16_t i = 0; i < partCount; i++) {
        Draw(*worldShader, **partPtr, defaultAmbient, false);
        if ((*partPtr)->HasAlpha()) {
            alphaParts.push_back(*partPtr);
        }
        partPtr--;
    }
    
    // Draw translucent geometry in a separate pass. Far-to-near is good here.
    for (auto it = alphaParts.rbegin(); it != alphaParts.rend(); ++it) {
        Draw(*worldShader, **it, defaultAmbient, true);
    }
    
    // RENDER HUD //////////////////////////////////////////////////////////////////////////////////
    hudWorld->PrepareForRender();
    partList = hudWorld->GetVisiblePartListPointer();
    partCount = hudWorld->GetVisiblePartCount();
    for (uint16_t i = 0; i < partCount; i++) {
        Draw(*worldShader, **partList, defaultAmbient, false);
        partList++;
    }

}

void LegacyOpenGLRenderer::AdjustAmbient(OpenGLShader &shader, float intensity)
{
    shader.SetFloat("ambient", intensity);
}

void LegacyOpenGLRenderer::ApplyView()
{
    glm::mat4 glMatrix = ToFloatMat(viewParams->viewMatrix);

    worldShader->Use();
    worldShader->SetMat4("view", glMatrix);
    __glCheckErrors();
}

void LegacyOpenGLRenderer::Clear()
{
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void LegacyOpenGLRenderer::Draw(OpenGLShader &shader, const CBSPPart &part, float defaultAmbient, bool useAlphaBuffer)
{
    OpenGLVertices *glData = dynamic_cast<OpenGLVertices*>(part.vData.get());

    if (glData == nullptr) return;

    if (!useAlphaBuffer) {
        glBindVertexArray(glData->opaque.vertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, glData->opaque.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, glData->opaque.glDataSize, glData->opaque.glData.data(), GL_STREAM_DRAW);
    } else {
        glData->alpha.SortFromCamera(
            ToFloat(part.invFullTransform[3][0]),
            ToFloat(part.invFullTransform[3][1]),
            ToFloat(part.invFullTransform[3][2])
        );
        glBindVertexArray(glData->alpha.vertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, glData->alpha.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, glData->alpha.glDataSize, glData->alpha.glData.data(), GL_STREAM_DRAW);
    }

    // Position!
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLData), 0);
    glEnableVertexAttribArray(0);

    // RGBAColor!
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GLData), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Normal!
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLData), (void *)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Custom, per-object lighting and depth testing!
    float extraAmbient = ToFloat(part.extraAmbient);
    if (part.privateAmbient != -1) {
        AdjustAmbient(shader, ToFloat(part.privateAmbient));
    }
    if (extraAmbient > 0) {
        AdjustAmbient(shader, defaultAmbient + extraAmbient);
    }
    if (part.ignoreDepthTesting) {
        glDisable(GL_DEPTH_TEST);
    }
    if (part.ignoreDirectionalLights) {
        IgnoreDirectionalLights(shader, true);
        __glCheckErrors();
    }

    SetTransforms(part);
    shader.Use();
    __glCheckErrors();

    if (!useAlphaBuffer) {
        glDrawArrays(GL_TRIANGLES, 0, glData->opaque.pointCount);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, glData->alpha.pointCount);
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    __glCheckErrors();

    // Restore previous lighting and depth testing state.
    if (part.privateAmbient != -1 || extraAmbient > 0) {
        AdjustAmbient(shader, defaultAmbient);
        __glCheckErrors();
    }
    if (part.ignoreDepthTesting) {
        glEnable(GL_DEPTH_TEST);
    }
    if (part.ignoreDirectionalLights) {
        IgnoreDirectionalLights(shader, false);
        __glCheckErrors();
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    __glCheckErrors();
}

void LegacyOpenGLRenderer::IgnoreDirectionalLights(OpenGLShader &shader, bool yn)
{
    shader.SetFloat("lightsActive", (yn) ? 0.0f : 1.0f);
}

std::unique_ptr<OpenGLShader> LegacyOpenGLRenderer::LoadShader(const std::string &vertFile,
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

void LegacyOpenGLRenderer::SetTransforms(const CBSPPart &part) {
    glm::mat4 mv = ToFloatMat(part.fullTransform);
    if (part.hasScale) {
        glm::vec3 sc = glm::vec3(
            ToFloat(part.scale[0]),
            ToFloat(part.scale[1]),
            ToFloat(part.scale[2])
        );
        mv = glm::scale(mv, sc);
    }

    glm::mat3 normalMat = glm::mat3(1.0f);
    for (int i = 0; i < 3; i ++) {
        normalMat[0][i] = ToFloat((part.itsTransform)[0][i]);
        normalMat[1][i] = ToFloat((part.itsTransform)[1][i]);
        normalMat[2][i] = ToFloat((part.itsTransform)[2][i]);
    }

    worldShader->Use();
    worldShader->SetMat4("modelview", mv);
    worldShader->SetMat3("normalTransform", normalMat, true);
}
