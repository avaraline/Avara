#pragma once

#include "CBSPPart.h"
#include "ModernOpenGLRenderer.h"
#include "VertexData.h"

#include <vector>

struct GLData {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;
    float nx = 0.0f;
    float ny = 0.0f;
    float nz = 0.0f;

    GLData() {};
    GLData(GLData &&other) noexcept :
        x(std::exchange(other.x, 0)),
        y(std::exchange(other.y, 0)),
        z(std::exchange(other.z, 0)),
        r(std::exchange(other.r, 0)),
        g(std::exchange(other.g, 0)),
        b(std::exchange(other.b, 0)),
        a(std::exchange(other.a, 0)),
        nx(std::exchange(other.nx, 0)),
        ny(std::exchange(other.ny, 0)),
        nz(std::exchange(other.nz, 0))
    {};
};

class OpenGLVertices: public VertexData {
public:
    friend class ModernOpenGLRenderer;

    OpenGLVertices();
    virtual ~OpenGLVertices();

    virtual void Append(const CBSPPart &part);
    virtual void Replace(const CBSPPart &part);
private:
    GLuint vertexBuffer = 0;
    GLuint vertexArray = 0;
    uint16_t pointCount = 0;
    std::vector<GLData> glData = {};
    GLsizeiptr glDataSize = 0;
};
