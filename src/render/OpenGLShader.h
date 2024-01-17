#pragma once

#include <glad/glad.h>

#include <string>

class OpenGLShader {
public:
    OpenGLShader(const std::string vertPath, const std::string fragPath);

    void Use();
    void SetBool(const std::string &uniform, bool value) const;
    void SetInt(const std::string &uniform, int value) const;
    void SetFloat(const std::string &uniform, float value) const;
private:
    GLuint id;
};
