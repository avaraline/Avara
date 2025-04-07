#include "ModernOpenGLRenderer.h"

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

#define OBJ_POST_VERT "worldPost_vert.glsl"
#define OBJ_POST_FRAG "worldPost_frag.glsl"

#define HUD_VERT "hud_vert.glsl"
#define HUD_FRAG "hud_frag.glsl"
#define HUD_AMBIENT 0.7f

#define HUD_POST_VERT "hudPost_vert.glsl"
#define HUD_POST_FRAG "hudPost_frag.glsl"

#define FINAL_VERT "final_vert.glsl"
#define FINAL_FRAG "final_frag.glsl"

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

ModernOpenGLRenderer::ModernOpenGLRenderer(SDL_Window *window) : AbstractRenderer()
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
    worldPostShader = LoadShader(OBJ_POST_VERT, OBJ_POST_FRAG);
    hudShader = LoadShader(HUD_VERT, HUD_FRAG);
    hudPostShader = LoadShader(HUD_POST_VERT, HUD_POST_FRAG);
    finalShader = LoadShader(FINAL_VERT, FINAL_FRAG);
    ApplyLights();
    ApplyProjection();
    
    alphaParts = {};

    // Create a separate VBO and VAO for the skybox, and upload its geometry to the GPU.
    glGenVertexArrays(1, &skyVertArray);
    glGenBuffers(1, &skyBuffer);
    glBindVertexArray(skyVertArray);
    glBindBuffer(GL_ARRAY_BUFFER, skyBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    // Rebind to default VBO/VAO.
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

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
}

ModernOpenGLRenderer::~ModernOpenGLRenderer() {
    delete dynamicWorld;
    delete hudWorld;
    AbstractRenderer::~AbstractRenderer();
}

void ModernOpenGLRenderer::AddHUDPart(CBSPPart *part)
{
    if (part->vData == nullptr) {
        part->vData = NewVertexDataInstance();
        part->vData->Replace(*part);
    }
    hudWorld->AddPart(part);
}

void ModernOpenGLRenderer::AddPart(CBSPPart *part)
{
    /*
    if (!part->usesPrivateYon && part->userFlags & CBSPUserFlags::kIsStatic) {
        if (staticGeometry == nullptr) {
            staticGeometry = (CBSPPart *)malloc(sizeof(CBSPPart));
            *staticGeometry = *part;
            staticGeometry->vData = NewVertexDataInstance();
        }
        staticGeometry->vData->Append(*part);
    } else {
        if (part->vData == nullptr) {
            part->vData = NewVertexDataInstance();
            part->vData->Replace(*part);
        }
        dynamicWorld->AddPart(part);
    }
    */
    if (part->vData == nullptr) {
        part->vData = NewVertexDataInstance();
        part->vData->Replace(*part);
    }
    dynamicWorld->AddPart(part);
}

void ModernOpenGLRenderer::ApplyLights()
{
    float ambientIntensity = ToFloat(viewParams->ambientLight);
    float ambientRGB[3];
    viewParams->ambientLightColor.ExportGLFloats(ambientRGB, 3);

    hudShader->Use();
    AdjustAmbient(*hudShader, HUD_AMBIENT);

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

void ModernOpenGLRenderer::ApplyProjection()
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
    glCheckErrors();

    worldShader->Use();
    worldShader->SetMat4("proj", proj);
    glCheckErrors();

    hudShader->Use();
    hudShader->SetMat4("proj", proj);
    glCheckErrors();
}

void ModernOpenGLRenderer::ApplySky()
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
    skyShader->SetFloat("lowAlt", ToFloat(skyParams->lowSkyAltitude) / 20000.0f);
    skyShader->SetFloat("highAlt", ToFloat(skyParams->highSkyAltitude) / 20000.0f);
    
    worldShader->Use();
    worldShader->SetFloat3("horizonColor", lowSkyColorRGB);
}

void ModernOpenGLRenderer::UpdateViewRect(int width, int height, float pixelRatio)
{
    AbstractRenderer::UpdateViewRect(width, height, pixelRatio);

    GLsizei w, h;
    SDL_GL_GetDrawableSize(window, &w, &h);

    AdjustFramebuffer(0, w, h);
    AdjustFramebuffer(1, w, h);
}

void ModernOpenGLRenderer::LevelReset()
{
    dynamicWorld->DisposeParts();
    hudWorld->DisposeParts();
    AbstractRenderer::LevelReset();
}

std::unique_ptr<VertexData> ModernOpenGLRenderer::NewVertexDataInstance()
{
    return std::make_unique<OpenGLVertices>();
}

void ModernOpenGLRenderer::OverheadPoint(Fixed *pt, Fixed *extent)
{
    dynamicWorld->OverheadPoint(pt, extent);
}

void ModernOpenGLRenderer::RefreshWindow()
{
    SDL_GL_SwapWindow(window);
}

void ModernOpenGLRenderer::RemoveHUDPart(CBSPPart *part)
{
    hudWorld->RemovePart(part);
}

void ModernOpenGLRenderer::RemovePart(CBSPPart *part)
{
    dynamicWorld->RemovePart(part);
}

void ModernOpenGLRenderer::RenderFrame()
{
    Clear();
    ApplyView();
    
    // RENDER SKYBOX ///////////////////////////////////////////////////////////////////////////////

    Matrix *trans = &(viewParams->viewMatrix);

    // Get rid of the view translation.
    glm::mat4 glMatrix = ToFloatMat(*trans);
    glMatrix[3][0] = glMatrix[3][1] = glMatrix[3][2] = 0;

    // Switch to first offscreen FBO.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
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
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(skyboxVertices));
    glDisableVertexAttribArray(0);

    // RENDER WORLD ////////////////////////////////////////////////////////////////////////////////

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);

    worldShader->Use();

    dynamicWorld->PrepareForRender();

    float defaultAmbient = ToFloat(viewParams->ambientLight);

    // Draw opaque geometry.
    // Draw(*worldShader, *staticGeometry, defaultAmbient, false);
    auto partList = dynamicWorld->GetVisiblePartListPointer();
    auto partCount = dynamicWorld->GetVisiblePartCount();
    alphaParts.clear();
    BlendingOn();
    for (uint16_t i = 0; i < partCount; i++) {
        Draw(*worldShader, **partList, defaultAmbient, false);
        if ((*partList)->HasAlpha()) {
            alphaParts.push_back(*partList);
        }
        partList++;
    }

    // Draw translucent geometry.
    // Draw(*worldShader, *staticGeometry, defaultAmbient, true);
    for (auto it = alphaParts.begin(); it != alphaParts.end(); ++it) {
        Draw(*worldShader, **it, defaultAmbient, true);
    }
    BlendingOff();

    // First pass of sky and world rendering complete, post-process into second offscreen FBO.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    worldPostShader->Use();
    glBindVertexArray(screenQuadVertArray);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // RENDER HUD //////////////////////////////////////////////////////////////////////////////////

    // Switch to first offscreen FBO.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    hudShader->Use();
    hudWorld->PrepareForRender();
    partList = hudWorld->GetVisiblePartListPointer();
    partCount = hudWorld->GetVisiblePartCount();
    for (uint16_t i = 0; i < partCount; i++) {
        Draw(*hudShader, **partList, HUD_AMBIENT);
        partList++;
    }

    // First pass of HUD rendering complete, post-process into second offscreen FBO.
    // Don't clear the buffer this time, as the sky/world texture is already waiting there waiting
    // for the HUD texture to be overlaid on it.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
    hudPostShader->Use();
    hudPostShader->SetFloat("hudAlpha", ColorManager::getHUDAlpha());
    glBindVertexArray(screenQuadVertArray);

    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    BlendingOn();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    BlendingOff();
    glEnable(GL_DEPTH_TEST);

    // FINAL POST-PROCESSING, SEND TO DEFAULT FRAMEBUFFER //////////////////////////////////////////

    float res[2] = {1.0f / (float)resolution[0], 1.0f / (float)resolution[1]};
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    finalShader->Use();
    finalShader->SetBool("fxaa", gApplication ? gApplication->Get<bool>(kFXAA) : true);
    finalShader->SetFloat2("texelStep", res);
    finalShader->SetFloat("lumaThreshold", 0.0625f);
    finalShader->SetFloat("mulReduce", 1.0f / 8.0f);
    finalShader->SetFloat("minReduce", 1.0f / 32.0f);
    finalShader->SetFloat("maxSpan", 8);
    glBindVertexArray(screenQuadVertArray);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, texture[1]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ModernOpenGLRenderer::AdjustAmbient(OpenGLShader &shader, float intensity)
{
    shader.SetFloat("ambient", intensity);
}

void ModernOpenGLRenderer::ApplyView()
{
    glm::mat4 glMatrix = ToFloatMat(viewParams->viewMatrix);

    worldShader->Use();
    worldShader->SetMat4("view", glMatrix);
    worldShader->SetFloat("worldYon", ToFloat(viewParams->yonBound));
    worldShader->SetFloat("objectYon", ToFloat(viewParams->yonBound));
    glCheckErrors();

    hudShader->Use();
    hudShader->SetMat4("view", glMatrix);
    glCheckErrors();
}

void ModernOpenGLRenderer::BlendingOff()
{
    glDisable(GL_BLEND);
}

void ModernOpenGLRenderer::BlendingOn()
{
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ModernOpenGLRenderer::Clear()
{
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void ModernOpenGLRenderer::Draw(OpenGLShader &shader, const CBSPPart &part, float defaultAmbient, bool useAlphaBuffer)
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
    float extraAmbient = ToFloat(part.extraAmbient);
    float currentYon = ToFloat(gRenderer->viewParams->yonBound);
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
        glCheckErrors();
    }
    if (part.usesPrivateYon) {
        shader.SetFloat("objectYon", ToFloat(part.yon));
    }

    SetTransforms(part);
    shader.Use();
    glCheckErrors();

    if (!useAlphaBuffer) {
        glDrawArrays(GL_TRIANGLES, 0, glData->opaque.pointCount);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, glData->alpha.pointCount);
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    // Restore previous lighting and depth testing state.
    if (part.privateAmbient != -1 || extraAmbient > 0) {
        AdjustAmbient(shader, defaultAmbient);
        glCheckErrors();
    }
    if (part.ignoreDepthTesting) {
        glEnable(GL_DEPTH_TEST);
    }
    if (part.ignoreDirectionalLights) {
        IgnoreDirectionalLights(shader, false);
        glCheckErrors();
    }
    if (part.usesPrivateYon) {
        shader.SetFloat("objectYon", currentYon);
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

void ModernOpenGLRenderer::AdjustFramebuffer(short index, GLsizei width, GLsizei height)
{
    // Remove previous bound objects
    glDeleteTextures(1, &texture[index]);
    glDeleteFramebuffers(1, &fbo[index]);
    glDeleteRenderbuffers(1, &rbo[index]);

    // Create a framebuffer, texture, and renderbuffer for the HUD.
    glGenFramebuffers(1, &fbo[index]);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[index]);
    glGenTextures(1, &texture[index]);
    glBindTexture(GL_TEXTURE_2D, texture[index]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture[index], 0);
    glGenRenderbuffers(1, &rbo[index]);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo[index]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    // Rebind to the default renderbuffer.
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Attach renderbuffer to framebuffer and check if the framebuffer was "completed" successfully.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo[index]);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        SDL_Log("Failed to create framebuffer %d", index);
        exit(1);
    }

    // Rebind to the default framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
