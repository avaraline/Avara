#include "MetalVertices.h"


MetalVertices::MetalVertices() {
}

MetalVertices::~MetalVertices() {
}

void MetalVertices::Append(const CBSPPart &part) {
}

void MetalVertices::Replace(const CBSPPart &part) {
    Append(part);
}
