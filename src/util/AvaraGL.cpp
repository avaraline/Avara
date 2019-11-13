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

GLuint gProgram;
GLuint mv_loc, nt_loc, amb_loc, lights_active_loc;

GLuint skyProgram;
GLuint skyVertArray, skyBuffer;
GLuint skyInvMatrixLoc, groundColorLoc, horizonColorLoc, skyColorLoc;

void AvaraGLToggleRendering(int active) {
    if (active < 1)
    actuallyRender = false;
}

void AvaraGLSetLight(int light_index, float intensity, float elevation, float azimuth) {
    if (!actuallyRender) return;
    // THERE... ARE... FOUR LIGHTS!!!!
    if (light_index < 0 || light_index > 3) return;
    std::ostringstream buffa;
    buffa << "light" << light_index;
    glUniform3f(glGetUniformLocation(gProgram, buffa.str().c_str()), intensity, elevation, azimuth);
}

void AvaraGLSetView(glm::mat4 view) {
    if (!actuallyRender) return;
    glUniformMatrix4fv(glGetUniformLocation(gProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
}

void AvaraGLLightDefaults() {
    if (!actuallyRender) return;
    // called before loading a level
    AvaraGLSetLight(0, 0.4, 45, 20);
    AvaraGLSetLight(1, 0.3, 20, 200);
    AvaraGLSetLight(2, 0, 0, 0);
    AvaraGLSetLight(3, 0, 0, 0);
    AvaraGLSetAmbient(0.4);
}

void AvaraGLInitContext() {
    //glEnable(GL_DEBUG_OUTPUT);
    if (!actuallyRender) return;
    gProgram = LoadShaders(BundlePath("shaders/avara_vert.glsl"), BundlePath("shaders/avara_frag.glsl"));
    glUseProgram(gProgram);
    AvaraGLLightDefaults();
    glm::mat4 proj = glm::scale(glm::perspective(glm::radians(30.0f), 4.0f/3.0f, 0.5f, 500.0f), glm::vec3(-1, 1, -1));
    glUniformMatrix4fv(glGetUniformLocation(gProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
    mv_loc = glGetUniformLocation(gProgram, "modelview");
    nt_loc = glGetUniformLocation(gProgram, "normal_transform");
    amb_loc = glGetUniformLocation(gProgram, "ambient");
    lights_active_loc = glGetUniformLocation(gProgram, "lights_active");

    skyProgram = LoadShaders(BundlePath("shaders/sky_vert.glsl"), BundlePath("shaders/sky_frag.glsl"));
    glGenVertexArrays(1, &skyVertArray);
    glGenBuffers(1, &skyBuffer);
    skyInvMatrixLoc = glGetUniformLocation(skyProgram, "invMatrix");
    groundColorLoc = glGetUniformLocation(skyProgram, "groundColor");
    horizonColorLoc = glGetUniformLocation(skyProgram, "horizonColor");
    skyColorLoc = glGetUniformLocation(skyProgram, "skyColor");
}

void AvaraGLSetAmbient(float ambient) {
    if (!actuallyRender) return;
    glUniform1f(amb_loc, ambient);
}

void ActivateLights(float active) {
    glUniform1f(lights_active_loc, active);
}

void SetTransforms(Matrix *modelview, glm::mat4 normal_transform) {
    glUniformMatrix4fv(mv_loc, 1, GL_FALSE, glm::value_ptr(FromFixedMat(modelview)));
    glUniformMatrix3fv(nt_loc, 1, GL_FALSE, glm::value_ptr(normal_transform));
}

void AvaraGLDrawPolygons(CBSPPart* part) {
    if(!actuallyRender) return;
    // Create a buffer big enough to hold vertex/color/normal for every point we draw.
    glUseProgram(gProgram);

    GLData *glData;
    GLuint vertexArray, vertexBuffer;
    GLsizeiptr glDataSize;

    glDataSize = part->totalPoints * sizeof(GLData);
    glData = (GLData *)NewPtr(glDataSize);

    glGenVertexArrays(1, &vertexArray);
    glGenBuffers(1, &vertexBuffer);


    PolyRecord *poly;
    float scale = 1.0; // ToFloat(currentView->screenScale);
    int p = 0;

    for (int i = 0; i < part->polyCount; i++) {
        poly = &part->polyTable[i];
        for (int v = 0; v < poly->triCount * 3; v++) {
            Vector *pt = &part->pointTable[poly->triPoints[v]];
            glData[p].x = ToFloat((*pt)[0]);
            glData[p].y = ToFloat((*pt)[1]);
            glData[p].z = ToFloat((*pt)[2]);
            glData[p].r = ((poly->color >> 16) & 0xFF) / 255.0;
            glData[p].g = ((poly->color >> 8) & 0xFF) / 255.0;
            glData[p].b = (poly->color & 0xFF) / 255.0;

            glData[p].nx = poly->normal[0];
            glData[p].ny = poly->normal[1];
            glData[p].nz = poly->normal[2];
            // SDL_Log("v(%f,%f,%f) c(%f,%f,%f) n(%f,%f,%f)\n", glData[p].x, glData[p].y, glData[p].z, glData[p].r,
            // glData[p].g, glData[p].b, glData[p].nx, glData[p].ny, glData[p].nz);
            p++;
        }
    }

    glBindVertexArray(vertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, glDataSize, glData, GL_STREAM_DRAW);

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
    }

    SetTransforms(&part->fullTransform, part->normalTransform);
    
    glBindVertexArray(vertexArray);
    glDrawArrays(GL_TRIANGLES, 0, part->totalPoints);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    // restore previous lighting state
    if (part->privateAmbient != -1 || extra_amb > 0) {
        AvaraGLSetAmbient(current_amb);
    }

    if (part->ignoreDirectionalLights) {
        ActivateLights(1);
    }

    glBindVertexArray(NULL);
    glBindBuffer(GL_ARRAY_BUFFER, NULL);
    DisposePtr((Ptr)glData);
    glDeleteVertexArrays(1, &vertexArray);
    glDeleteBuffers(1, &vertexBuffer);
}


void AvaraGLShadeWorld(CWorldShader *theShader, CViewParameters *theView) {
    if (!actuallyRender) return;
    Matrix *trans = theView->GetInverseMatrix();
    float matrix[16];
    for (int c = 0; c < 4; c++) {
        for (int r = 0; r < 4; r++) {
            matrix[(c * 4) + r] = ToFloat((*trans)[c][r]);
        }
    }
    // Get rid of the translation part
    matrix[12] = matrix[13] = matrix[14] = 0;
    matrix[15] = 1;

    // generate card for drawing on
    float Z = 1.0;
    float SCALE = 1.0;
    float points[12] = {
        -1 * SCALE, -1 * SCALE, Z, 1 * SCALE, -1 * SCALE, Z, -1 * SCALE, 1 * SCALE, Z, 1 * SCALE, 1 * SCALE, Z};
    glBindVertexArray(skyVertArray);
    glBindBuffer(GL_ARRAY_BUFFER, skyBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    long groundColor = theShader->groundColor;
    long lowSkyColor = theShader->lowSkyColor;
    long highSkyColor = theShader->highSkyColor;

    // glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);
    glEnableVertexAttribArray(0);
    glUseProgram(skyProgram);
    glUniformMatrix4fv(skyInvMatrixLoc, 1, GL_FALSE, matrix);
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
    glBindVertexArray(skyVertArray);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
