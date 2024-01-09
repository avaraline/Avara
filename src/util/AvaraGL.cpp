#include "AvaraGL.h"
#include "AssetManager.h"
#include "FastMat.h"
#include "CViewParameters.h"
#include "ARGBColor.h"
#include "CBSPPart.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <math.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define OBJ_VERT "avara_vert.glsl"
#define OBJ_FRAG "avara_frag.glsl"

#define HUD_VERT "hud_vert.glsl"
#define HUD_FRAG "hud_frag.glsl"

#define SKY_VERT "sky_vert.glsl"
#define SKY_FRAG "sky_frag.glsl"

bool actuallyRender = true;
bool ready = false;

glm::mat4 proj;
const float near_dist = .099f;
const float far_dist = 1000.0f;

float current_fov = 60.0f;
short window_height = 1;
short window_width = 1;

float skyboxVertices[] = {
        // positions
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

GLuint gProgram;
GLuint mvLoc, ntLoc, ambLoc, ambColorLoc, lights_activeLoc, projLoc, viewLoc;
GLuint light0Loc, light0ColorLoc;
GLuint light1Loc, light1ColorLoc;
GLuint light2Loc, light2ColorLoc;
GLuint light3Loc, light3ColorLoc;

GLuint hudProgram;
GLuint hudViewLoc, hudProjLoc, hudMvLoc, hudLightsActiveLoc;

GLuint skyProgram;
GLuint skyVertArray, skyBuffer;
GLuint skyViewLoc, skyProjLoc, groundColorLoc, horizonColorLoc, skyColorLoc;

const char* glGetErrorString(GLenum error)
{
    switch (error)
    {
    case GL_NO_ERROR:          return "No Error";
    case GL_INVALID_ENUM:      return "Invalid Enum";
    case GL_INVALID_VALUE:     return "Invalid Value";
    case GL_INVALID_OPERATION: return "Invalid Operation";
    case GL_INVALID_FRAMEBUFFER_OPERATION: return "Invalid Framebuffer Operation";
    case GL_OUT_OF_MEMORY:     return "Out of Memory";
    case GL_STACK_UNDERFLOW:   return "Stack Underflow";
    case GL_STACK_OVERFLOW:    return "Stack Overflow";
    //case GL_CONTEXT_LOST:      return "Context Lost";
    default:                   return "Unknown Error";
    }
}

void _glCheckErrors(const char *filename, int line)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
        printf("OpenGL Error: %s (%d) [%u] %s\n", filename, line, err, glGetErrorString(err));
}
#define glCheckErrors() _glCheckErrors(__FILE__, __LINE__)

void AvaraGLToggleRendering(int active) {
    if (active < 1)
    actuallyRender = false;
}

bool AvaraGLIsRendering() {
    return actuallyRender;
}

void AvaraGLSetView(glm::mat4 view) {
    if (!actuallyRender) return;
    glUseProgram(gProgram);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glCheckErrors();
    glUseProgram(hudProgram);
    glUniformMatrix4fv(hudViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glCheckErrors();
}


void AvaraGLSetFOV(float fov) {
    current_fov = fov;
    AvaraGLUpdateProjectionMatrix();
}

void AvaraGLUpdateProjectionMatrix() {
    proj = glm::scale(glm::perspective(
                        glm::radians(current_fov),
                        (float)window_width / (float)window_height,
                        near_dist,
                        far_dist)
                     , glm::vec3(-1, 1, -1));
    glUseProgram(gProgram);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glCheckErrors();
    glUseProgram(hudProgram);
    glUniformMatrix4fv(hudProjLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glCheckErrors();
}

void AvaraGLSetLight(int light_index, float intensity, float elevation, float azimuth, ARGBColor color) {
    if (!actuallyRender) return;

    float x = cos(Deg2Rad(elevation)) * intensity;
    float y = sin(Deg2Rad(-elevation)) * intensity;
    float z = cos(Deg2Rad(azimuth)) * intensity;
    float rgb[3];

    x = sin(Deg2Rad(-azimuth)) * intensity;
    color.ExportGLFloats(rgb, 3);

    glUseProgram(gProgram);
    switch (light_index) {
        case 0:
            glUniform3f(light0Loc, x, y, z);
            glUniform3fv(light0ColorLoc, 1, rgb);
            break;
        case 1:
            glUniform3f(light1Loc, x, y, z);
            glUniform3fv(light1ColorLoc, 1, rgb);
            break;
        case 2:
            glUniform3f(light2Loc, x, y, z);
            glUniform3fv(light2ColorLoc, 1, rgb);
            break;
        case 3:
            glUniform3f(light3Loc, x, y, z);
            glUniform3fv(light3ColorLoc, 1, rgb);
            break;
    }
}

void AvaraGLSetAmbient(float ambient, ARGBColor color) {
    if (!actuallyRender) return;

    float rgb[3];
    color.ExportGLFloats(rgb, 3);

    glUseProgram(gProgram);
    glUniform1f(ambLoc, ambient);
    glUniform3fv(ambColorLoc, 1, rgb);
}

void ActivateLights(float active) {
    glUseProgram(gProgram);
    glUniform1f(lights_activeLoc, active);
    glUseProgram(hudProgram);
    glUniform1f(hudLightsActiveLoc, active);
}

void AvaraGLLightDefaults() {
    if (!actuallyRender) return;
    // called before loading a level
    AvaraGLSetLight(0, 0.4f, 45.0f, 20.0f, DEFAULT_LIGHT_COLOR);
    AvaraGLSetLight(1, 0.3f, 20.0f, 200.0f, DEFAULT_LIGHT_COLOR);
    AvaraGLSetLight(2, 0, 0, 0, DEFAULT_LIGHT_COLOR);
    AvaraGLSetLight(3, 0, 0, 0, DEFAULT_LIGHT_COLOR);
    AvaraGLSetAmbient(0.4f, DEFAULT_LIGHT_COLOR);
}

void SetTransforms(CBSPPart *part) {
    glm::mat4 mv = ToFloatMat(&part->fullTransform);
    if (part->hasScale) {
        glm::vec3 sc = glm::vec3(
            ToFloat(part->scale[0]), 
            ToFloat(part->scale[1]), 
            ToFloat(part->scale[2])
        );
        mv = glm::scale(mv, sc);
    }

    glm::mat3 normal_mat = glm::mat3(1.0f);
    for (int i = 0; i < 3; i ++) {
        normal_mat[0][i] = ToFloat((part->itsTransform)[0][i]);
        normal_mat[1][i] = ToFloat((part->itsTransform)[1][i]);
        normal_mat[2][i] = ToFloat((part->itsTransform)[2][i]);
    }

    glUseProgram(gProgram);
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mv));
    glUniformMatrix3fv(ntLoc, 1, GL_TRUE, glm::value_ptr(normal_mat));
    glUseProgram(hudProgram);
    glUniformMatrix4fv(hudMvLoc, 1, GL_FALSE, glm::value_ptr(mv));
}

void AvaraGLSetDepthTest(bool doTest) {
    if (doTest) glDepthFunc(GL_LEQUAL);
    else glDepthFunc(GL_ALWAYS);
}

void AvaraGLInitContext() {
    //glEnable(GL_DEBUG_OUTPUT);
    if (!actuallyRender) return;
    std::optional<std::string> vertPath = AssetManager::GetShaderPath(OBJ_VERT);
    std::optional<std::string> fragPath = AssetManager::GetShaderPath(OBJ_FRAG);
    gProgram = LoadShaders(vertPath, fragPath);
    glUseProgram(gProgram);

    projLoc = glGetUniformLocation(gProgram, "proj");
    viewLoc = glGetUniformLocation(gProgram, "view");
    AvaraGLUpdateProjectionMatrix();
    mvLoc = glGetUniformLocation(gProgram, "modelview");
    ntLoc = glGetUniformLocation(gProgram, "normal_transform");
    ambLoc = glGetUniformLocation(gProgram, "ambient");
    ambColorLoc = glGetUniformLocation(gProgram, "ambientColor");
    lights_activeLoc = glGetUniformLocation(gProgram, "lights_active");
    glCheckErrors();

    light0Loc = glGetUniformLocation(gProgram, "light0");
    light0ColorLoc = glGetUniformLocation(gProgram, "light0Color");
    light1Loc = glGetUniformLocation(gProgram, "light1");
    light1ColorLoc = glGetUniformLocation(gProgram, "light1Color");
    light2Loc = glGetUniformLocation(gProgram, "light2");
    light2ColorLoc = glGetUniformLocation(gProgram, "light2Color");
    light3Loc = glGetUniformLocation(gProgram, "light3");
    light3ColorLoc = glGetUniformLocation(gProgram, "light3Color");
    glCheckErrors();

    AvaraGLLightDefaults();
    glCheckErrors();

    std::optional<std::string> hudVertPath = AssetManager::GetShaderPath(HUD_VERT);
    std::optional<std::string> hudFragPath = AssetManager::GetShaderPath(HUD_FRAG);
    hudProgram = LoadShaders(hudVertPath, hudFragPath);
    glUseProgram(hudProgram);

    hudViewLoc = glGetUniformLocation(hudProgram, "view");
    hudProjLoc = glGetUniformLocation(hudProgram, "proj");
    hudMvLoc = glGetUniformLocation(hudProgram, "modelview");
    hudLightsActiveLoc = glGetUniformLocation(hudProgram, "lights_active");
    glCheckErrors();

    std::optional<std::string> skyVertPath = AssetManager::GetShaderPath(SKY_VERT);
    std::optional<std::string> skyFragPath = AssetManager::GetShaderPath(SKY_FRAG);
    skyProgram = LoadShaders(skyVertPath, skyFragPath);
    glGenVertexArrays(1, &skyVertArray);
    glGenBuffers(1, &skyBuffer);
    skyViewLoc = glGetUniformLocation(skyProgram, "view");
    skyProjLoc = glGetUniformLocation(skyProgram, "proj");
    groundColorLoc = glGetUniformLocation(skyProgram, "groundColor");
    horizonColorLoc = glGetUniformLocation(skyProgram, "horizonColor");
    skyColorLoc = glGetUniformLocation(skyProgram, "skyColor");

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE);

    ready = true;
}

void AvaraGLViewport(short width, short height) {
    window_width = width;
    window_height = height;
    AvaraGLUpdateProjectionMatrix();
}

void AvaraGLDrawPolygons(CBSPPart* part, Shader shader) {
    glCheckErrors();
    if(!actuallyRender || !ready) return;
    // Bind the vertex array and buffer that we set up earlier
    if (shader == Shader::HUD) {
        glUseProgram(hudProgram);
    } else {
        glUseProgram(gProgram);
    }
    glBindVertexArray(part->vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, part->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, part->glDataSize, part->glData.get(), GL_STREAM_DRAW);
    glCheckErrors();

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLData), 0);
    glEnableVertexAttribArray(0);
    // RGBAColor
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(GLData), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GLData), (void *)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // custom per-object lighting
    float extra_amb = ToFloat(part->extraAmbient);
    float current_amb = ToFloat(part->currentView->ambientLight);

    if (part->privateAmbient != -1) {
        AvaraGLSetAmbient(ToFloat(part->privateAmbient), part->currentView->ambientLightColor);
    }
    if (extra_amb > 0) {
        AvaraGLSetAmbient(current_amb + extra_amb, part->currentView->ambientLightColor);
    }
    if (part->ignoreDirectionalLights) {
        ActivateLights(0);
        glCheckErrors();
    }

    if (shader == Shader::HUD) {
        glUseProgram(hudProgram);
    } else {
        glUseProgram(gProgram);
    }

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    SetTransforms(part);
    glCheckErrors();

    if (shader == Shader::HUD) {
        glUseProgram(hudProgram);
    } else {
        glUseProgram(gProgram);
    }

    glBindVertexArray(part->vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, part->openGLPoints);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    // restore previous lighting state
    if (part->privateAmbient != -1 || extra_amb > 0) {
        AvaraGLSetAmbient(current_amb, part->currentView->ambientLightColor);
        glCheckErrors();
    }
    if (part->ignoreDirectionalLights) {
        ActivateLights(1);
        glCheckErrors();
    }

    if (shader == Shader::HUD) {
        glUseProgram(hudProgram);
    } else {
        glUseProgram(gProgram);
    }

    glBindVertexArray(0);
    glCheckErrors();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glCheckErrors();

}

void AvaraGLUpdateData(CBSPPart *part) {
    if (!AvaraGLIsRendering()) return;

    // If we haven't generated OpenGL data for this
    // BSP yet, glDataSize will be 0
    if(part->glDataSize > 0) {
        // Free previously used memory on the GPU
        glDeleteVertexArrays(1, &part->vertexArray);
        glDeleteBuffers(1, &part->vertexBuffer);
    }
    // Vertex buffer and array for this shape
    // We use these pointers later to refer to the 
    // vertex data in the draw step
    glGenVertexArrays(1, &part->vertexArray);
    glGenBuffers(1, &part->vertexBuffer);

    PolyRecord *poly;
    ARGBColor *color;
    part->openGLPoints = 0;
    uint8_t vis;
    int tris, points;

    // up front, calculate how many points we *actually* need
    // for faces that should be visible on both sides, double
    // the faces, triangles, and points to be reversed for the
    // back side
    for (size_t i = 0; i < part->polyCount; i++)
    {
        poly = &part->polyTable[i];
        color = &part->currColorTable[poly->colorIdx];
        vis = (part->HasAlpha() && poly->vis != 0) ? 3 : poly->vis;
        if (!vis) vis = 0;
        switch (vis) {
            case 0:
                tris = 0;
                break;
            case 3:
                tris = poly->triCount * 2;
                break;
            default:
                tris = poly->triCount;
                break;
        }
        points = tris * 3;
        part->openGLPoints += points;
    }
    part->glDataSize = part->openGLPoints * sizeof(GLData);
    part->glData = std::make_unique<GLData[]>(part->glDataSize);

    // Count all the points we output so that we can make sure
    // it matches what we just calculated above
    int p = 0;
    for (int i = 0; i < part->polyCount; i++) {
        poly = &part->polyTable[i];
        color = &part->currColorTable[poly->colorIdx];
        uint8_t vis = (part->HasAlpha() && poly->vis != 0) ? 3 : poly->vis;
        if (!vis) vis = 0; // default to 0 (none) if vis is empty
        
        // vis can ONLY be 0 for None, 1 for Front, 2 for Back, or 3 for Both
        assert (vis == 0 || vis == 1 || vis == 2 || vis == 3);

        // wrap forwards - front side
        if (vis == 1 || vis == 3) {
            for (int v = 0; v < poly->triCount * 3; v++) {
                Vector *pt = &part->pointTable[poly->triPoints[v]];
                part->glData[p].x = ToFloat((*pt)[0]);
                part->glData[p].y = ToFloat((*pt)[1]);
                part->glData[p].z = ToFloat((*pt)[2]);
                color->ExportGLFloats(&part->glData[p].r, 4);
                part->glData[p].nx = poly->normal[0];
                part->glData[p].ny = poly->normal[1];
                part->glData[p].nz = poly->normal[2];
                p++;
            }
        }
        // wrap backwards - back side
        if (vis == 2 || vis == 3) {
            for (int v = (poly->triCount * 3) - 1; v >= 0; v--) {
                Vector *pt = &part->pointTable[poly->triPoints[v]];
                part->glData[p].x = ToFloat((*pt)[0]);
                part->glData[p].y = ToFloat((*pt)[1]);
                part->glData[p].z = ToFloat((*pt)[2]);
                color->ExportGLFloats(&part->glData[p].r, 4);
                part->glData[p].nx = -poly->normal[0];
                part->glData[p].ny = -poly->normal[1];
                part->glData[p].nz = -poly->normal[2];
                p++;
            }
        }
    }
    // make sure we filled in the array correctly
    assert(p == part->openGLPoints);
}


void AvaraGLShadeWorld(CWorldShader *theShader, CViewParameters *theView) {
    glCheckErrors();
    if (!actuallyRender || !ready) return;
    Matrix *trans = &theView->viewMatrix;
    float matrix[16];
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            matrix[(c * 4) + r] = ToFloat((*trans)[c][r]);
        }
    }
    // Get rid of the translation part
    matrix[12] = matrix[13] = matrix[14] = 0;

    glBindVertexArray(skyVertArray);
    glBindBuffer(GL_ARRAY_BUFFER, skyBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    float groundColorRGB[3];
    float lowSkyColorRGB[3];
    float highSkyColorRGB[3];
    theShader->groundColor.ExportGLFloats(groundColorRGB, 3);
    theShader->lowSkyColor.ExportGLFloats(lowSkyColorRGB, 3);
    theShader->highSkyColor.ExportGLFloats(highSkyColorRGB, 3);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);
    glEnableVertexAttribArray(0);
    glUseProgram(skyProgram);
    glUniformMatrix4fv(skyViewLoc, 1, GL_FALSE, matrix);
    glUniformMatrix4fv(skyProjLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3fv(groundColorLoc, 1, groundColorRGB);
    glUniform3fv(horizonColorLoc, 1, lowSkyColorRGB);
    glUniform3fv(skyColorLoc, 1, highSkyColorRGB);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindVertexArray(skyVertArray);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(skyboxVertices));
    glDisableVertexAttribArray(0);
    glEnable(GL_DEPTH_TEST);
}

GLuint LoadShaders(std::optional<std::string> vertex_file_path, std::optional<std::string> fragment_file_path) {
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(*vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    } else {
        printf("Impossible to open %s. Are you in the right directory? \n", (*vertex_file_path).c_str());
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(*fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();
        FragmentShaderStream.close();
    } else {
        printf("Impossible to open %s. Are you in the right directory? \n", (*fragment_file_path).c_str());
        getchar();
        return 0;
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    fprintf(stderr, "Compiling shader : %s\n", (*vertex_file_path).c_str());
    char const *VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    // Compile Fragment Shader
    fprintf(stderr, "Compiling shader : %s\n", (*fragment_file_path).c_str());
    char const *FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    if (Result == GL_FALSE) {
        // exit on shader compilation failure
        exit(1);
    }

    // Link the program
    fprintf(stderr, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}
