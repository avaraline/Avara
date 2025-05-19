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

    staticWorld = new CBSPWorldImpl(100);
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
}

LegacyOpenGLRenderer::~LegacyOpenGLRenderer()
{
    delete staticWorld;
    delete dynamicWorld;
    delete hudWorld;
}

void LegacyOpenGLRenderer::AddHUDPart(CBSPPart *part)
{
    part->ignoreDirectionalLights = true;
    part->ignoreDepthTesting = true;
    if (part->vData == nullptr) {
        part->vData = NewVertexDataInstance();
        part->vData->Replace(*part);
    }
    hudWorld->AddPart(part);
}

void LegacyOpenGLRenderer::AddPart(CBSPPart *part)
{
    if (part->usesPrivateYon && part->yon < FIX3(100)) {
        // Don't add the part to anything! It's (effectively) invisible and
        // will never be rendered.
    } else if (!part->usesPrivateYon && part->userFlags & CBSPUserFlags::kIsStatic) {
        staticWorld->AddPart(part);
    } else {
        if (part->vData == nullptr) {
            part->vData = NewVertexDataInstance();
            part->vData->Replace(*part);
        }
        dynamicWorld->AddPart(part);
    }
}

void LegacyOpenGLRenderer::ApplyLights()
{
    float ambientIntensity = ToFloat(viewParams->ambientLight);
    float ambientRGB[3];
    viewParams->ambientLightColor.ExportGLFloats(ambientRGB, 3);

    worldShader->Use();
    AdjustAmbient(*worldShader, ambientIntensity);
    worldShader->SetFloat3("ambientColor", ambientRGB);
    worldShader->SetFloat("maxShininess", MAX_SHININESS_EXP);
    
    skyShader->Use();
    skyShader->SetFloat("celestialDistance", DIR_LIGHT_DISTANCE);

    for (int i = 0; i < MAXLIGHTS; i++) {
        float rgb[3];
        viewParams->dirLightSettings[i].color.ExportGLFloats(rgb, 3);
        bool applySpecular = viewParams->dirLightSettings[i].applySpecular;
        
        const std::string dirUniform = "lightDir[" + std::to_string(i) + "]";
        const std::string posUniform = "lightPos[" + std::to_string(i) + "]";
        const std::string colorUniform = "lightColor[" + std::to_string(i) + "]";
        const std::string radUniform = "lightCelestialRadius[" + std::to_string(i) + "]";
        const std::string specUniform = "lightApplySpecular[" + std::to_string(i) + "]";

        worldShader->Use();
        worldShader->SetFloat3(dirUniform, viewParams->dirLightSettings[i].direction);
        worldShader->SetFloat3(posUniform, viewParams->dirLightSettings[i].position);
        worldShader->SetFloat3(colorUniform, rgb);
        worldShader->SetFloat(radUniform, ToFloat(viewParams->dirLightSettings[i].celestialRadius));
        worldShader->SetBool(specUniform, applySpecular);
        
        skyShader->Use();
        skyShader->SetFloat3(dirUniform, viewParams->dirLightSettings[i].direction);
        skyShader->SetFloat3(posUniform, viewParams->dirLightSettings[i].position);
        skyShader->SetFloat3(colorUniform, rgb);
        skyShader->SetFloat(radUniform, ToFloat(viewParams->dirLightSettings[i].celestialRadius));
        skyShader->SetBool(specUniform, applySpecular);
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
    float highSkyColorRGB[3];
    float lowSkyColorRGB[3];
    float groundColorRGB[3];
    float groundSpecRGB[3];
    float groundShininess;
    skyParams->highSkyColor.ExportGLFloats(highSkyColorRGB, 3);
    skyParams->lowSkyColor.ExportGLFloats(lowSkyColorRGB, 3);
    skyParams->groundMaterial.GetColor().ExportGLFloats(groundColorRGB, 3);
    skyParams->groundMaterial.GetSpecular().ExportGLFloats(groundSpecRGB, 3);
    groundShininess = skyParams->groundMaterial.GetShininess() / 255.0f * MAX_SHININESS_EXP;
    
    float lowAlt = ToFloat(skyParams->lowSkyAltitude) / 20000.0f;
    float highAlt = ToFloat(skyParams->highSkyAltitude) / 20000.0f;
    float hazeDensity = skyParams->hazeDensity;
    
    skyShader->Use();
    skyShader->SetFloat3("skyColor", highSkyColorRGB);
    skyShader->SetFloat3("horizonColor", lowSkyColorRGB);
    skyShader->SetFloat3("groundColor", groundColorRGB);
    skyShader->SetFloat3("groundSpec", groundSpecRGB);
    skyShader->SetFloat("groundShininess", groundShininess);
    skyShader->SetFloat("lowAlt", lowAlt);
    skyShader->SetFloat("highAlt", highAlt);
    skyShader->SetFloat("hazeDensity", hazeDensity);
    
    worldShader->Use();
    worldShader->SetFloat3("skyColor", highSkyColorRGB);
    worldShader->SetFloat3("horizonColor", lowSkyColorRGB);
    worldShader->SetFloat("highAlt", highAlt);
    worldShader->SetFloat("hazeDensity", hazeDensity);
}

void LegacyOpenGLRenderer::UpdateViewRect(int width, int height, float pixelRatio)
{
    AbstractRenderer::UpdateViewRect(width, height, pixelRatio);
    ApplyProjection();
}

void LegacyOpenGLRenderer::LevelReset()
{
    if (staticGeometry) {
        staticGeometry.reset();
    }
    staticWorld->DisposeParts();
    dynamicWorld->DisposeParts();
    hudWorld->DisposeParts();
    AbstractRenderer::LevelReset();
}

std::unique_ptr<VertexData> LegacyOpenGLRenderer::NewVertexDataInstance()
{
    return std::make_unique<OpenGLVertices>();
}

void LegacyOpenGLRenderer::PostLevelLoad()
{
    staticGeometry = staticWorld->Squash();
    staticGeometry->vData = NewVertexDataInstance();
    staticGeometry->vData->Replace(*staticGeometry);
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
    staticWorld->RemovePart(part);
    dynamicWorld->RemovePart(part);
}

void LegacyOpenGLRenderer::RenderFrame()
{
    Clear();
    ApplyView();

    // RENDER SKYBOX ///////////////////////////////////////////////////////////////////////////////

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glBindVertexArray(skyVertArray);
    glBindBuffer(GL_ARRAY_BUFFER, skyBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);
    glEnableVertexAttribArray(0);

    skyShader->Use();
    skyShader->SetBool("dither", gApplication ? gApplication->Get<bool>(kDither) : true);
    skyShader->SetBool("showSpecular", gApplication ? gApplication->Get<bool>(kSpecular) : true);

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
    worldShader->SetBool("dither", gApplication ? gApplication->Get<bool>(kDither) : true);
    worldShader->SetBool("showSpecular", gApplication ? gApplication->Get<bool>(kSpecular) : true);

    dynamicWorld->PrepareForRender();

    __glCheckErrors();
    float defaultAmbient = ToFloat(viewParams->ambientLight);

    // Draw opaque geometry.
    BlendingOn();
    if (staticGeometry) {
        staticGeometry->PrepareForRender();
        Draw(*worldShader, *staticGeometry, defaultAmbient, false);
    }
    auto partList = dynamicWorld->GetVisiblePartListPointer();
    auto partCount = dynamicWorld->GetVisiblePartCount();
    alphaParts.clear();
    for (uint32_t i = 0; i < partCount; i++) {
        Draw(*worldShader, **partList, defaultAmbient, false);
        if ((*partList)->HasAlpha()) {
            alphaParts.push_back(*partList);
        }
        partList++;
    }
    
    // Draw translucent geometry.
    if (staticGeometry) {
        Draw(*worldShader, *staticGeometry, defaultAmbient, true);
    }
    for (auto it = alphaParts.begin(); it != alphaParts.end(); ++it) {
        Draw(*worldShader, **it, defaultAmbient, true);
    }
    
    // RENDER HUD //////////////////////////////////////////////////////////////////////////////////
    hudWorld->PrepareForRender();
    partList = hudWorld->GetVisiblePartListPointer();
    partCount = hudWorld->GetVisiblePartCount();
    for (uint32_t i = 0; i < partCount; i++) {
        Draw(*worldShader, **partList, defaultAmbient, false);
        partList++;
    }
    BlendingOff();
}

void LegacyOpenGLRenderer::AdjustAmbient(OpenGLShader &shader, float intensity)
{
    shader.SetFloat("ambient", intensity);
}

void LegacyOpenGLRenderer::ApplyView()
{
    glm::mat4 glMatrix = ToFloatMat(viewParams->viewMatrix);
    
    // Get rid of the view translation for the sky.
    glm::mat4 glSkyMatrix = ToFloatMat(viewParams->viewMatrix);
    glSkyMatrix[3][0] = glSkyMatrix[3][1] = glSkyMatrix[3][2] = 0;
    
    skyShader->Use();
    skyShader->SetMat4("view", glSkyMatrix);
    skyShader->SetFloat("maxHazeDist", ToFloat(viewParams->yonBound));
    SetPositions(*skyShader);

    worldShader->Use();
    worldShader->SetTransposedMat4("view", glMatrix);
    worldShader->SetFloat("worldYon", ToFloat(viewParams->yonBound));
    worldShader->SetFloat("objectYon", ToFloat(viewParams->yonBound));
    SetPositions(*worldShader);
    __glCheckErrors();
}

void LegacyOpenGLRenderer::BlendingOff()
{
    glDisable(GL_BLEND);
}

void LegacyOpenGLRenderer::BlendingOn()
{
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
        if (glData->opaque.glDataSize == 0) return;
        glBindVertexArray(glData->opaque.vertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, glData->opaque.vertexBuffer);
    } else {
        if (glData->alpha.glDataSize == 0) return;
        glData->alpha.SortFromCamera(
            ToFloat(part.invModelViewTransform[3][0]),
            ToFloat(part.invModelViewTransform[3][1]),
            ToFloat(part.invModelViewTransform[3][2])
        );
        glBindVertexArray(glData->alpha.vertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, glData->alpha.vertexBuffer);
        
        // Reupload sorted tris to GPU.
        glBufferData(GL_ARRAY_BUFFER, glData->alpha.glDataSize, glData->alpha.glData.data(), GL_STREAM_DRAW);
    }

    // Position!
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLData), 0);
    glEnableVertexAttribArray(0);

    // RGBAColor!
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GLData), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Specular!
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(GLData), (void *)((3 * sizeof(float)) + (4 * sizeof(uint8_t))));
    glEnableVertexAttribArray(2);

    // Normal!
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(GLData), (void *)((3 * sizeof(float)) + (8 * sizeof(uint8_t))));
    glEnableVertexAttribArray(3);

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
        __glCheckErrors();
    }
    if (part.usesPrivateYon) {
        shader.SetFloat("objectYon", ToFloat(part.yon));
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
    if (part.usesPrivateYon) {
        shader.SetFloat("objectYon", currentYon);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    __glCheckErrors();
}

void LegacyOpenGLRenderer::IgnoreDirectionalLights(OpenGLShader &shader, bool yn)
{
    shader.SetBool("lightsActive", !yn);
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

void LegacyOpenGLRenderer::SetPositions(OpenGLShader &shader)
{
    glm::mat4 glInvMatrix = ToFloatMat(*viewParams->GetInverseMatrix());
    float camPos[3] = {glInvMatrix[3][0], glInvMatrix[3][1], glInvMatrix[3][2]};
    
    shader.Use();
    shader.SetFloat3("camPos", camPos);
    for (int i = 0; i < MAXLIGHTS; i++) {
        glm::vec3 adjLightPos = {
            viewParams->dirLightSettings[i].position[0] + camPos[0],
            viewParams->dirLightSettings[i].position[1] + camPos[1],
            viewParams->dirLightSettings[i].position[2] + camPos[2]
        };
        
        const std::string adjPosUniform = "adjustedLightPos[" + std::to_string(i) + "]";
        shader.SetVec3(adjPosUniform, adjLightPos);
    }
}

void LegacyOpenGLRenderer::SetTransforms(const CBSPPart &part)
{
    glm::mat4 m = ToFloatMat(part.modelTransform);
    if (part.hasScale) {
        glm::vec3 sc = glm::vec3(
            ToFloat(part.scale[0]),
            ToFloat(part.scale[1]),
            ToFloat(part.scale[2])
        );
        m = glm::scale(m, sc);
    }
    
    glm::mat3 normalMat = glm::mat3(1.0f);
    for (int i = 0; i < 3; i ++) {
        normalMat[0][i] = ToFloat((part.modelTransform)[0][i]);
        normalMat[1][i] = ToFloat((part.modelTransform)[1][i]);
        normalMat[2][i] = ToFloat((part.modelTransform)[2][i]);
    }

    worldShader->Use();
    worldShader->SetTransposedMat4("model", m);
    worldShader->SetTransposedMat3("normalTransform", normalMat);
}
