#include "AvaraGL.h"
#include "FastMat.h"
#include "Resource.h"
#include "CViewParameters.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


bool actuallyRender = true;

glm::mat4 proj;
const float near_dist = .47f;
const float far_dist = 1000.0f;
const float default_fov = 50.0f;
const float default_aspect = 4.0/3.0f;

float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

GLuint gProgram;
GLuint mvLoc, ntLoc, ambLoc, lights_activeLoc, projLoc, viewLoc, decalLoc;
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

void AvaraGLSetView(glm::mat4 view) {
    if (!actuallyRender) return;
    glUseProgram(gProgram);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glCheckErrors();
}


void AvaraGLUpdateProjectionMatrix(float fov, float aspect) {
    glUseProgram(gProgram);
    proj = glm::scale(glm::perspective(glm::radians(fov), aspect, near_dist, far_dist), glm::vec3(-1, 1, -1));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glCheckErrors();
}

void AvaraGLSetLight(int light_index, float intensity, float elevation, float azimuth) {
    if (!actuallyRender) return;
    glUseProgram(gProgram);
    switch (light_index) {
        case 0:
            glUniform3f(light0Loc, intensity, elevation, azimuth);
            break;
        case 1:
            glUniform3f(light1Loc, intensity, elevation, azimuth);
            break;
        case 2:
            glUniform3f(light2Loc, intensity, elevation, azimuth);
            break;
        case 3:
            glUniform3f(light3Loc, intensity, elevation, azimuth);
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

void AvaraGLSetDecal(float active) {
    glUseProgram(gProgram);
    glUniform1f(decalLoc, active);
}

void SetTransforms(Matrix *modelview, glm::mat3 normal_transform) {
    glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(FromFixedMat(modelview)));
    glUniformMatrix3fv(ntLoc, 1, GL_TRUE, glm::value_ptr(normal_transform));
}

void AvaraGLInitContext() {
    //glEnable(GL_DEBUG_OUTPUT);
    if (!actuallyRender) return;
    gProgram = LoadShaders(BundlePath("shaders/avara_vert.glsl"), BundlePath("shaders/avara_frag.glsl"));
    glUseProgram(gProgram);

    projLoc = glGetUniformLocation(gProgram, "proj");
    viewLoc = glGetUniformLocation(gProgram, "view");
    AvaraGLUpdateProjectionMatrix(default_fov, default_aspect);
    mvLoc = glGetUniformLocation(gProgram, "modelview");
    ntLoc = glGetUniformLocation(gProgram, "normal_transform");
    ambLoc = glGetUniformLocation(gProgram, "ambient");
    lights_activeLoc = glGetUniformLocation(gProgram, "lights_active");
    decalLoc = glGetUniformLocation(gProgram, "decal");
    glCheckErrors();


    light0Loc = glGetUniformLocation(gProgram, "light0");
    light1Loc = glGetUniformLocation(gProgram, "light1");
    light2Loc = glGetUniformLocation(gProgram, "light2");
    light3Loc = glGetUniformLocation(gProgram, "light3");
    glCheckErrors();

    AvaraGLLightDefaults();
    glCheckErrors();

    skyProgram = LoadShaders(BundlePath("shaders/sky_vert.glsl"), BundlePath("shaders/sky_frag.glsl"));
    glGenVertexArrays(1, &skyVertArray);
    glGenBuffers(1, &skyBuffer);
    skyViewLoc = glGetUniformLocation(skyProgram, "view");
    skyProjLoc = glGetUniformLocation(skyProgram, "proj");
    groundColorLoc = glGetUniformLocation(skyProgram, "groundColor");
    horizonColorLoc = glGetUniformLocation(skyProgram, "horizonColor");
    skyColorLoc = glGetUniformLocation(skyProgram, "skyColor");

}

void AvaraGLDrawPolygons(CBSPPart* part) {
    glCheckErrors();
    if(!actuallyRender) return;
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

    // if we're drawing something thin
    // give it a little z-buffer push towards
    // the camera by scaling down the z-value
    if (part->isDecal) {
        AvaraGLSetDecal(.9995f);
    }

    part->UpdateNormalMatrix();
    SetTransforms(&part->fullTransform, part->normalTransform);
    glCheckErrors();
    
    glBindVertexArray(part->vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, part->totalPoints);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    // reset z-buffer scale
    if (part->isDecal) {
        AvaraGLSetDecal(1.0);
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

    glBindVertexArray(NULL);
    glCheckErrors();

    glBindBuffer(GL_ARRAY_BUFFER, NULL);
    glCheckErrors();
}


void AvaraGLShadeWorld(CWorldShader *theShader, CViewParameters *theView) {
    glCheckErrors();
    if (!actuallyRender) return;
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

    long groundColor = theShader->groundColor;
    long lowSkyColor = theShader->lowSkyColor;
    long highSkyColor = theShader->highSkyColor;

    glDisable(GL_DEPTH_TEST);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);
    glEnableVertexAttribArray(0);
    glUseProgram(skyProgram);
    glUniformMatrix4fv(skyViewLoc, 1, GL_FALSE, matrix);
    glUniformMatrix4fv(skyProjLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniform3f(groundColorLoc,
        ((groundColor >> 16) & 0xFF) / 255.0,
        ((groundColor >> 8) & 0xFF) / 255.0,
        (groundColor & 0xFF) / 255.0);
    glUniform3f(horizonColorLoc,
        ((lowSkyColor >> 16) & 0xFF) / 255.0,
        ((lowSkyColor >> 8) & 0xFF) / 255.0,
        (lowSkyColor & 0xFF) / 255.0);
    glUniform3f(skyColorLoc,
        ((highSkyColor >> 16) & 0xFF) / 255.0,
        ((highSkyColor >> 8) & 0xFF) / 255.0,
        (highSkyColor & 0xFF) / 255.0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindVertexArray(skyVertArray);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(skyboxVertices));
    glDisableVertexAttribArray(0);
    glEnable(GL_DEPTH_TEST);
}

glm::mat4 FromFixedMat(Matrix *m) {
    glm::mat4 mat(1.0);
    for (int i = 0; i < 3; i ++) {
        mat[0][i] = ToFloat((*m)[0][i]);
        mat[1][i] = ToFloat((*m)[1][i]);
        mat[2][i] = ToFloat((*m)[2][i]);
        mat[3][i] = ToFloat((*m)[3][i]);
    }
    return mat;
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
