#pragma once

#include "CBSPPart.h"
#include "Material.h"
#include "ModernOpenGLRenderer.h"
#include "VertexData.h"

#include <vector>

struct GLData {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
    uint8_t specR = 0;
    uint8_t specG = 0;
    uint8_t specB = 0;
    uint8_t specS = 0;
    float nx = 0.0f;
    float ny = 0.0f;
    float nz = 0.0f;

    GLData() {};
    GLData(float x0, float y0, float z0) { x = x0; y = y0; z = z0; };
};

struct GLVertexCollection {
public:
    GLuint vertexBuffer = 0;
    GLuint vertexArray = 0;
    uint32_t pointCount = 0;
    std::vector<GLData> glData = {};
    GLsizeiptr glDataSize = 0;

    void SortFromCamera(float x, float y, float z);
};

class OpenGLVertices: public VertexData {
public:
    friend class ModernOpenGLRenderer;
    friend class LegacyOpenGLRenderer;

    OpenGLVertices();
    virtual ~OpenGLVertices();

    virtual void Append(const CBSPPart &part);
    virtual void Replace(const CBSPPart &part);
private:
    GLVertexCollection opaque;
    GLVertexCollection alpha;
};
