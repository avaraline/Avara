#pragma once

#include "CBSPPart.h"
#include "glm.hpp"

class CBSPPart;

class VertexData {
public:
    virtual ~VertexData() {};

    virtual void Append(const CBSPPart &part) = 0;
    virtual void Replace(const CBSPPart &part) = 0;
};

static inline glm::mat4 ToFloatMat(const Matrix &m)
{
    glm::mat4 mat(1.0);
    for (int i = 0; i < 3; i ++) {
        mat[0][i] = ToFloat(m[0][i]);
        mat[1][i] = ToFloat(m[1][i]);
        mat[2][i] = ToFloat(m[2][i]);
        mat[3][i] = ToFloat(m[3][i]);
    }
    return mat;
}
