#include "AvaraGL.h"

#include "Resource.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

GLuint gProgram;

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


void AvaraGLSetLight(int light_index, float intensity, float elevation, float azimuth) {
    // THERE... ARE... FOUR LIGHTS!!!!
    if (light_index < 0 || light_index > 3) return;
    std::ostringstream buffa;
    buffa << "light" << light_index;
    glUniform3f(glGetUniformLocation(gProgram, buffa.str().c_str()), intensity, elevation, azimuth);
}


void AvaraGLActivateLights(float active) {
    glUniform1f(glGetUniformLocation(gProgram, "lights_active"), active);
}

void AvaraGLSetAmbient(float ambient) {
    glUniform1f(glGetUniformLocation(gProgram, "ambient"), ambient);
}

void AvaraGLSetView(glm::mat4 view) {
    //SDL_Log("view-set");
    //std::cout << glm::to_string(view) << std::endl;
    glUniformMatrix4fv(glGetUniformLocation(gProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
}

void AvaraGLSetTransforms(Matrix *modelview, glm::mat4 normal_transform) {
    glUniformMatrix4fv(glGetUniformLocation(gProgram, "modelview"), 1, GL_FALSE, glm::value_ptr(FromFixedMat(modelview)));
    glUniformMatrix3fv(glGetUniformLocation(gProgram, "normal_transform"), 1, GL_FALSE, glm::value_ptr(normal_transform));
}


void AvaraGLLightDefaults() {
    // called before loading a level
    AvaraGLSetLight(0, 0.4, 45, 20);
    AvaraGLSetLight(1, 0.3, 20, 200);
    AvaraGLSetLight(2, 0, 0, 0);
    AvaraGLSetLight(3, 0, 0, 0);
    AvaraGLSetAmbient(0.4);
}

void AvaraGLInitContext() {
    gProgram = LoadShaders(BundlePath("shaders/avara_vert.glsl"), BundlePath("shaders/avara_frag.glsl"));
    glUseProgram(gProgram);
    AvaraGLLightDefaults();
    glm::mat4 proj = glm::scale(glm::perspective(glm::radians(30.0f), 4.0f/3.0f, 0.5f, 500.0f), glm::vec3(-1, 1, -1));
    glUniformMatrix4fv(glGetUniformLocation(gProgram, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
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