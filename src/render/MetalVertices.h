#pragma once

#include "CBSPPart.h"
#include "MetalRenderer.h"
#include "VertexData.h"

#include <vector>

class MetalVertices: public VertexData {
public:
    friend class MetalRenderer;

    MetalVertices();
    virtual ~MetalVertices();

    virtual void Append(const CBSPPart &part);
    virtual void Replace(const CBSPPart &part);
};
