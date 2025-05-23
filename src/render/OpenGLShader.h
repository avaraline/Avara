#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif
#include <glad/glad.h>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_vec.hpp>

#include <string>

class OpenGLShader {
public:
    OpenGLShader(const std::string vertPath, const std::string fragPath);

    void Use();
    void SetBool(const std::string &uniform, bool value) const;
    void SetInt(const std::string &uniform, int value) const;
    void SetFloat(const std::string &uniform, float value) const;
    void SetFloat2(const std::string &uniform, const float value[2]) const;
    void SetFloat3(const std::string &uniform, const float value[3]) const;
    void SetMat3(const std::string &uniform, const glm::mat3 &value) const;
    void SetTransposedMat3(const std::string &uniform, const glm::mat3 &value) const;
    void SetMat4(const std::string &uniform, const glm::mat4 &value) const;
    void SetTransposedMat4(const std::string &uniform, const glm::mat4 &value) const;
    void SetVec3(const std::string &uniform, const glm::vec3 &value) const;
private:
    GLuint id;
};
