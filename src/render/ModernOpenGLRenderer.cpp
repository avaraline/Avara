#include "ModernOpenGLRenderer.h"

#include "AssetManager.h"
#include "ColorManager.h"
#include "FastMat.h"
#include "OpenGLVertices.h"
#include "RenderManager.h"

#include <glm/gtc/matrix_transform.hpp>

#include <SDL2/SDL.h>

#define SKY_VERT "sky_vert.glsl"
#define SKY_FRAG "sky_frag.glsl"

#define OBJ_VERT "avara_vert.glsl"
#define OBJ_FRAG "avara_frag.glsl"

#define HUD_VERT "hud_vert.glsl"
#define HUD_FRAG "hud_frag.glsl"

#define HUD_POST_VERT "hudPost_vert.glsl"
#define HUD_POST_FRAG "hudPost_frag.glsl"

const float skyboxVertices[] = {
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

const float screenQuadVertices[] = {
    // positions   // texCoords
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

const char *glGetErrorString(GLenum error)
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

void _glCheckErrors(const char *filename, int line)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
        SDL_Log("OpenGL Error: %s (%d) [%u] %s\n", filename, line, err, glGetErrorString(err));
}
#define glCheckErrors() _glCheckErrors(__FILE__, __LINE__)

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

ModernOpenGLRenderer::ModernOpenGLRenderer(RenderManager *manager)
{
    this->manager = manager;

    GLsizei w, h;
    SDL_GL_GetDrawableSize(this->manager->window, &w, &h);

    // Initialize shaders.
    skyShader = LoadShader(SKY_VERT, SKY_FRAG);
    worldShader = LoadShader(OBJ_VERT, OBJ_FRAG);
    hudShader = LoadShader(HUD_VERT, HUD_FRAG);
    hudPostShader = LoadShader(HUD_POST_VERT, HUD_POST_FRAG);
    ApplyLights();
    ApplyProjection();

    // Create a separate VBO and VAO for the skybox, and upload its geometry to the GPU.
    glGenVertexArrays(1, &skyVertArray);
    glGenBuffers(1, &skyBuffer);
    glBindVertexArray(skyVertArray);
    glBindBuffer(GL_ARRAY_BUFFER, skyBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    // Rebind to default VBO/VAO.
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create a framebuffer, texture, and renderbuffer for the HUD.
    glGenFramebuffers(1, &hudFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hudFBO);
    glGenTextures(1, &hudTexture);
    glBindTexture(GL_TEXTURE_2D, hudTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hudTexture, 0);
    glGenRenderbuffers(1, &hudRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, hudRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);

    // Rebind to the default renderbuffer.
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Attach renderbuffer to framebuffer and check if the framebuffer was "completed" successfully.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, hudRBO);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        SDL_Log("Failed to create HUD framebuffer");
        exit(1);
    }

    // Rebind to the default framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create a separate VBO/VAO for a fullscreen quad, and upload its geometry to the GPU.
    glGenVertexArrays(1, &screenQuadVertArray);
    glGenBuffers(1, &screenQuadBuffer);
    glBindVertexArray(screenQuadVertArray);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadVertices), screenQuadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(float)), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (4 * sizeof(float)), (void*)(2 * sizeof(float)));

    // Rebind to default VBO/VAO.
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Configure alpha blending.
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE);
}

ModernOpenGLRenderer::~ModernOpenGLRenderer() {}

void ModernOpenGLRenderer::ApplyLights()
{
    float ambientIntensity = ToFloat(manager->viewParams->ambientLight);
    float ambientRGB[3];
    manager->viewParams->ambientLightColor.ExportGLFloats(ambientRGB, 3);

    hudShader->Use();
    AdjustAmbient(*hudShader, 0.7f); // Default HUD ambient.

    worldShader->Use();
    AdjustAmbient(*worldShader, ambientIntensity);
    worldShader->SetFloat3("ambientColor", ambientRGB);

    for (int i = 0; i < 4; i++) {
        float intensity = ToFloat(manager->viewParams->dirLightSettings[i].intensity);
        float elevation = ToFloat(manager->viewParams->dirLightSettings[i].angle1);
        float azimuth = ToFloat(manager->viewParams->dirLightSettings[i].angle2);
        float rgb[3];
        manager->viewParams->dirLightSettings[i].color.ExportGLFloats(rgb, 3);

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

void ModernOpenGLRenderer::ApplyProjection()
{
    int w, h;
    manager->GetWindowSize(w, h);

    glm::mat4 proj = glm::scale(
        glm::perspective(
            glm::radians(manager->fov),
            (float)w / (float)h,
            0.099f,
            1000.0f
        ),
        glm::vec3(-1, 1, -1)
    );

    skyShader->Use();
    skyShader->SetMat4("proj", proj);
    glCheckErrors();

    worldShader->Use();
    worldShader->SetMat4("proj", proj);
    glCheckErrors();

    hudShader->Use();
    hudShader->SetMat4("proj", proj);
    glCheckErrors();
}

void ModernOpenGLRenderer::ApplyView()
{
    glm::mat4 glMatrix = ToFloatMat(manager->viewParams->viewMatrix);

    worldShader->Use();
    worldShader->SetMat4("view", glMatrix);
    glCheckErrors();

    hudShader->Use();
    hudShader->SetMat4("view", glMatrix);
    glCheckErrors();
}

void ModernOpenGLRenderer::Clear()
{
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

std::unique_ptr<VertexData> ModernOpenGLRenderer::NewVertexDataInstance()
{
    return std::make_unique<OpenGLVertices>();
}

void ModernOpenGLRenderer::RefreshWindow()
{
    SDL_GL_SwapWindow(manager->window);
}

void ModernOpenGLRenderer::RenderSky()
{
    Matrix *trans = &(manager->viewParams->viewMatrix);

    // Get rid of the view translation.
    glm::mat4 glMatrix = ToFloatMat(*trans);
    glMatrix[3][0] = glMatrix[3][1] = glMatrix[3][2] = 0;

    float groundColorRGB[3];
    float lowSkyColorRGB[3];
    float highSkyColorRGB[3];
    manager->skyParams->groundColor.ExportGLFloats(groundColorRGB, 3);
    manager->skyParams->lowSkyColor.ExportGLFloats(lowSkyColorRGB, 3);
    manager->skyParams->highSkyColor.ExportGLFloats(highSkyColorRGB, 3);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glBindVertexArray(skyVertArray);
    glBindBuffer(GL_ARRAY_BUFFER, skyBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);
    glEnableVertexAttribArray(0);

    skyShader->Use();
    skyShader->SetMat4("view", glMatrix);
    skyShader->SetFloat3("groundColor", groundColorRGB);
    skyShader->SetFloat3("horizonColor", lowSkyColorRGB);
    skyShader->SetFloat3("skyColor", highSkyColorRGB);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(skyboxVertices));
    glDisableVertexAttribArray(0);

    glBindVertexArray(0);
    glCheckErrors();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glCheckErrors();
}

void ModernOpenGLRenderer::RenderStaticWorld()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);

    worldShader->Use();

    manager->staticWorld->PrepareForRender();
    auto partList = manager->staticWorld->GetVisiblePartListPointer();
    auto partCount = manager->staticWorld->GetVisiblePartCount();
    for (uint16_t i = 0; i < partCount; i++) {
        Draw(*worldShader, **partList);
        partList++;
    }
}

void ModernOpenGLRenderer::RenderDynamicWorld()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);

    worldShader->Use();

    manager->dynamicWorld->PrepareForRender();
    auto partList = manager->dynamicWorld->GetVisiblePartListPointer();
    auto partCount = manager->dynamicWorld->GetVisiblePartCount();
    for (uint16_t i = 0; i < partCount; i++) {
        Draw(*worldShader, **partList);
        partList++;
    }
}

void ModernOpenGLRenderer::RenderHUD()
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, hudFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    hudShader->Use();
    manager->hudWorld->PrepareForRender();
    auto partList = manager->hudWorld->GetVisiblePartListPointer();
    auto partCount = manager->hudWorld->GetVisiblePartCount();
    for (uint16_t i = 0; i < partCount; i++) {
        Draw(*hudShader, **partList);
        partList++;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    hudPostShader->Use();
    hudPostShader->SetFloat("hudAlpha", ColorManager::getHUDAlpha());
    glBindVertexArray(screenQuadVertArray);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, hudTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_DEPTH_TEST);

    if (manager->ui) {
        if (gApplication ? gApplication->Get<bool>(kShowNewHUD) : true) {
            manager->ui->RenderNewHUD(manager->nvg);
        } else {
            manager->ui->Render(manager->nvg);
        }
    }
}

void ModernOpenGLRenderer::AdjustAmbient(OpenGLShader &shader, float intensity)
{
    shader.SetFloat("ambient", intensity);
}

void ModernOpenGLRenderer::Draw(OpenGLShader &shader, const CBSPPart &part)
{
    OpenGLVertices *glData = dynamic_cast<OpenGLVertices*>(part.vData.get());

    if (glData == nullptr) return;

    glBindVertexArray(glData->vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, glData->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, glData->glDataSize, glData->glData.data(), GL_STREAM_DRAW);
    glCheckErrors();

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
    float extra_amb = ToFloat(part.extraAmbient);
    float current_amb = ToFloat(manager->viewParams->ambientLight);
    if (part.privateAmbient != -1) {
        AdjustAmbient(shader, ToFloat(part.privateAmbient));
    }
    if (extra_amb > 0) {
        AdjustAmbient(shader, current_amb + extra_amb);
    }
    if (part.ignoreDepthTesting) {
        glDisable(GL_DEPTH_TEST);
    }
    if (part.ignoreDirectionalLights) {
        IgnoreDirectionalLights(shader, true);
        glCheckErrors();
    }

    SetTransforms(part);
    shader.Use();
    glCheckErrors();

    glDrawArrays(GL_TRIANGLES, 0, glData->pointCount);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    // Restore previous lighting and depth testing state.
    if (part.privateAmbient != -1 || extra_amb > 0) {
        AdjustAmbient(shader, current_amb);
        glCheckErrors();
    }
    if (part.ignoreDepthTesting) {
        glEnable(GL_DEPTH_TEST);
    }
    if (part.ignoreDirectionalLights) {
        IgnoreDirectionalLights(shader, false);
        glCheckErrors();
    }

    glBindVertexArray(0);
    glCheckErrors();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glCheckErrors();
}

void ModernOpenGLRenderer::IgnoreDirectionalLights(OpenGLShader &shader, bool yn)
{
    shader.SetFloat("lightsActive", (yn) ? 0.0f : 1.0f);
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

void ModernOpenGLRenderer::SetTransforms(const CBSPPart &part) {
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

    hudShader->Use();
    hudShader->SetMat4("modelview", mv);
}
