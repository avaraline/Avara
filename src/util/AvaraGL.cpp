#include "AvaraGL.h"
#include "FastMat.h"
#include "Resource.h"
#include "CViewParameters.h"
#include "RGBAColor.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define OBJ_VERT "rsrc/shaders/avara_vert.glsl"
#define OBJ_FRAG "rsrc/shaders/avara_frag.glsl"

#define SKY_VERT "rsrc/shaders/sky_vert.glsl"
#define SKY_FRAG "rsrc/shaders/sky_frag.glsl"

bool actuallyRender = true;
bool ready = false;

glm::mat4 proj;
const float near_dist = .1f;
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
GLuint mvLoc, ntLoc, ambLoc, lights_activeLoc, projLoc, viewLoc;
GLuint light0Loc, light1Loc, light2Loc, light3Loc;

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
}

void AvaraGLSetLight(int light_index, float intensity, float elevation, float azimuth) {
    if (!actuallyRender) return;

    float x = cos(Deg2Rad(elevation)) * intensity;
    float y = sin(Deg2Rad(-elevation)) * intensity;
    float z = cos(Deg2Rad(azimuth)) * intensity;
    x = sin(Deg2Rad(-azimuth)) * intensity;

    glUseProgram(gProgram);
    switch (light_index) {
        case 0:
            glUniform3f(light0Loc, x, y, z);
            break;
        case 1:
            glUniform3f(light1Loc, x, y, z);
            break;
        case 2:
            glUniform3f(light2Loc, x, y, z);
            break;
        case 3:
            glUniform3f(light3Loc, x, y, z);
            break;
    }
}

void AvaraGLSetAmbient(float ambient) {
    if (!actuallyRender) return;
    glUseProgram(gProgram);
    glUniform1f(ambLoc, ambient);
}

void ActivateLights(float active) {
    glUseProgram(gProgram);
    glUniform1f(lights_activeLoc, active);
}

void AvaraGLLightDefaults() {
    if (!actuallyRender) return;
    // called before loading a level
    AvaraGLSetLight(0, 0.4f, 45.0f, 20.0f);
    AvaraGLSetLight(1, 0.3f, 20.0f, 200.0f);
    AvaraGLSetLight(2, 0, 0, 0);
    AvaraGLSetLight(3, 0, 0, 0);
    AvaraGLSetAmbient(0.4f);
}

void SetTransforms(Matrix *modelview, Matrix *normal_transform) {
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(ToFloatMat(modelview)));
    glm::mat3 normal_mat = glm::mat3(1.0f);

    for (int i = 0; i < 3; i ++) {
        normal_mat[0][i] = ToFloat((*normal_transform)[0][i]);
        normal_mat[1][i] = ToFloat((*normal_transform)[1][i]);
        normal_mat[2][i] = ToFloat((*normal_transform)[2][i]);
    }

    glUniformMatrix3fv(ntLoc, 1, GL_TRUE, glm::value_ptr(normal_mat));
}

void AvaraGLSetDepthTest(bool doTest) {
    if (doTest) glDepthFunc(GL_LEQUAL);
    else glDepthFunc(GL_ALWAYS);
}

void AvaraGLInitContext() {
    //glEnable(GL_DEBUG_OUTPUT);
    if (!actuallyRender) return;
    gProgram = LoadShaders(BundlePath(OBJ_VERT), BundlePath(OBJ_FRAG));
    glUseProgram(gProgram);

    projLoc = glGetUniformLocation(gProgram, "proj");
    viewLoc = glGetUniformLocation(gProgram, "view");
    AvaraGLUpdateProjectionMatrix();
    mvLoc = glGetUniformLocation(gProgram, "modelview");
    ntLoc = glGetUniformLocation(gProgram, "normal_transform");
    ambLoc = glGetUniformLocation(gProgram, "ambient");
    lights_activeLoc = glGetUniformLocation(gProgram, "lights_active");
    glCheckErrors();


    light0Loc = glGetUniformLocation(gProgram, "light0");
    light1Loc = glGetUniformLocation(gProgram, "light1");
    light2Loc = glGetUniformLocation(gProgram, "light2");
    light3Loc = glGetUniformLocation(gProgram, "light3");
    glCheckErrors();

    AvaraGLLightDefaults();
    glCheckErrors();

    skyProgram = LoadShaders(BundlePath(SKY_VERT), BundlePath(SKY_FRAG));
    glGenVertexArrays(1, &skyVertArray);
    glGenBuffers(1, &skyBuffer);
    skyViewLoc = glGetUniformLocation(skyProgram, "view");
    skyProjLoc = glGetUniformLocation(skyProgram, "proj");
    groundColorLoc = glGetUniformLocation(skyProgram, "groundColor");
    horizonColorLoc = glGetUniformLocation(skyProgram, "horizonColor");
    skyColorLoc = glGetUniformLocation(skyProgram, "skyColor");
    ready = true;
}

void AvaraGLViewport(short width, short height) {
    window_width = width;
    window_height = height;
    AvaraGLUpdateProjectionMatrix();
}

void AvaraGLDrawPolygons(CBSPPart* part) {
    glCheckErrors();
    if(!actuallyRender || !ready) return;
    // Create a buffer big enough to hold vertex/color/normal for every point we draw.
    glUseProgram(gProgram);
    glBindVertexArray(part->vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, part->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, part->glDataSize, part->glData, GL_STREAM_DRAW);
    glCheckErrors();

    for (int i = 0; i < 3; i++) {
        glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, sizeof(GLData), (void *)(i * 3 * sizeof(float)));
        glEnableVertexAttribArray(i);
    }

    // custom per-object lighting
    float extra_amb = ToFloat(part->extraAmbient);
    float current_amb = ToFloat(part->currentView->ambientLight);

    if (part->privateAmbient != -1) {
        AvaraGLSetAmbient(ToFloat(part->privateAmbient));
    }
    if (extra_amb > 0) {
        AvaraGLSetAmbient(current_amb + extra_amb);
    }
    if (part->ignoreDirectionalLights) {
        ActivateLights(0);
        glCheckErrors();
    }

    // hack to find the viewPortPart and
    // scout, we want to render only the
    // front faces of these so we can see thru
    // the back of the faces with the camera
    if (part->usesPrivateHither == true) {
        // magic value set for scout and head
        if (part->hither == FIX3(101)) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);
        }
    }

    // if we're drawing something thin
    // give it a little z-buffer push towards
    // the camera by scaling the z-value
    bool decal = part->isDecal;
    if (decal) {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.0, 1.0);
    }

    SetTransforms(&part->fullTransform, &part->itsTransform);
    glCheckErrors();

    glBindVertexArray(part->vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, part->totalPoints);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    // reset z-buffer scale
    if (decal) {
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    // restore previous lighting state
    if (part->privateAmbient != -1 || extra_amb > 0) {
        AvaraGLSetAmbient(current_amb);
        glCheckErrors();
    }
    if (part->ignoreDirectionalLights) {
        ActivateLights(1);
        glCheckErrors();
    }

    // turn backface culling back off for
    // all other geometry
    if (part->usesPrivateHither == true) {
        if (part->hither == FIX3(101)) {
            glDisable(GL_CULL_FACE);
        }
    }

    glBindVertexArray(NULL);
    glCheckErrors();

    glBindBuffer(GL_ARRAY_BUFFER, NULL);
    glCheckErrors();

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
    LongToRGBA(theShader->groundColor, groundColorRGB, 3);
    LongToRGBA(theShader->lowSkyColor, lowSkyColorRGB, 3);
    LongToRGBA(theShader->highSkyColor, highSkyColorRGB, 3);

    glDisable(GL_DEPTH_TEST);
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

GLuint LoadShaders(const char *vertex_file_path, const char *fragment_file_path) {
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    } else {
        printf("Impossible to open %s. Are you in the right directory? \n", vertex_file_path);
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
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
    printf("Compiling shader : %s\n", fragment_file_path);
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

    // Link the program
    printf("Linking program\n");
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
