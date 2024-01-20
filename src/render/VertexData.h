#pragma once

#include "CBSPPart.h"

class CBSPPart;

class VertexData {
public:
    virtual ~VertexData() {};

    virtual void Append(const CBSPPart &part) = 0;
    virtual void Replace(const CBSPPart &part) = 0;
};
