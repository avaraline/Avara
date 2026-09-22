#include "OpenGLShader.h"

#include <SDL2/SDL.h>

#include <fstream>
#include <sstream>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

OpenGLShader::OpenGLShader(const std::string vertPath, const std::string fragPath)
{
    std::string VertexShaderCode;
    std::string FragmentShaderCode;
    std::ifstream VertexShaderStream;
    std::ifstream FragmentShaderStream;
    VertexShaderStream.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    FragmentShaderStream.exceptions (std::ifstream::failbit | std::ifstream::badbit);

    try {
        VertexShaderStream.open(vertPath);
        FragmentShaderStream.open(fragPath);

        std::stringstream sstr;
#if defined(AVARA_GLES)
        sstr << "#version 300 es\n";
        sstr << "precision highp float;\n\n";
#else
        sstr << "#version 330 core\n";
#endif
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        sstr.str("");
#if defined(AVARA_GLES)
        sstr << "#version 300 es\n";
        sstr << "precision highp float;\n\n";
#else
        sstr << "#version 330 core\n";
#endif
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();

        VertexShaderStream.close();
        FragmentShaderStream.close();
    } catch (std::ifstream::failure e) {
        SDL_Log("Failed to read shader (%s, %s)", vertPath.c_str(), fragPath.c_str());
        exit(1);
    }

    GLint vertex, fragment, result;
    int infoLogLength;

    vertex = glCreateShader(GL_VERTEX_SHADER);
    const char *VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(vertex, 1, &VertexSourcePointer, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> errorMessage(infoLogLength + 1);
        glGetShaderInfoLog(vertex, infoLogLength, NULL, &errorMessage[0]);
        SDL_Log("Failed to compile vertex shader %s\n%s", vertPath.c_str(), &errorMessage[0]);
        exit(1);
    };

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    const char *FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(fragment, 1, &FragmentSourcePointer, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &result);
    if (!result) {
        glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> errorMessage(infoLogLength + 1);
        glGetShaderInfoLog(fragment, infoLogLength, NULL, &errorMessage[0]);
        SDL_Log("Failed to compile fragment shader %s\n%s", fragPath.c_str(), &errorMessage[0]);
        exit(1);
    };

    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    glGetProgramiv(id, GL_LINK_STATUS, &result);
    if (!result) {
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> errorMessage(infoLogLength + 1);
        glGetProgramInfoLog(id, infoLogLength, NULL, &errorMessage[0]);
        SDL_Log("Failed to link shader\n%s", &errorMessage[0]);
        exit(1);
    }

    glDetachShader(id, vertex);
    glDetachShader(id, fragment);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void OpenGLShader::Use()
{
    glUseProgram(id);
}

void OpenGLShader::SetBool(const std::string &uniform, bool value) const
{
    glUniform1i(glGetUniformLocation(id, uniform.c_str()), (int)value);
}

void OpenGLShader::SetInt(const std::string &uniform, int value) const
{
    glUniform1i(glGetUniformLocation(id, uniform.c_str()), value);
}

void OpenGLShader::SetFloat(const std::string &uniform, float value) const
{
    glUniform1f(glGetUniformLocation(id, uniform.c_str()), value);
}

void OpenGLShader::SetFloat2(const std::string &uniform, const float value[2]) const
{
    glUniform2fv(glGetUniformLocation(id, uniform.c_str()), 1, value);
}

void OpenGLShader::SetFloat3(const std::string &uniform, const float value[3]) const
{
    glUniform3fv(glGetUniformLocation(id, uniform.c_str()), 1, value);
}

void OpenGLShader::SetMat3(const std::string &uniform, const glm::mat3 &value) const
{
    glUniformMatrix3fv(
        glGetUniformLocation(id, uniform.c_str()),
        1,
        GL_FALSE,
        glm::value_ptr(value)
    );
}

void OpenGLShader::SetTransposedMat3(const std::string &uniform, const glm::mat3 &value) const
{
    glUniformMatrix3fv(
        glGetUniformLocation(id, uniform.c_str()),
        1,
        GL_TRUE,
        glm::value_ptr(value)
    );
}

void OpenGLShader::SetMat4(const std::string &uniform, const glm::mat4 &value) const
{
    glUniformMatrix4fv(
        glGetUniformLocation(id, uniform.c_str()),
        1,
        GL_FALSE,
        glm::value_ptr(value)
    );
}

void OpenGLShader::SetTransposedMat4(const std::string &uniform, const glm::mat4 &value) const
{
    glUniformMatrix4fv(
        glGetUniformLocation(id, uniform.c_str()),
        1,
        GL_TRUE,
        glm::value_ptr(value)
    );
}

void OpenGLShader::SetVec3(const std::string &uniform, const glm::vec3 &value) const
{
    glUniform3fv(glGetUniformLocation(id, uniform.c_str()), 1, glm::value_ptr(value));
}
