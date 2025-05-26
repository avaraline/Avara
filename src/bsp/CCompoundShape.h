#pragma once

#include "CBSPPart.h"
#include "CBSPWorld.h"

class CBSPWorldImpl;

class CCompoundShape : public CBSPPart {
public:
    CCompoundShape();
    virtual ~CCompoundShape() override;
    void Append(CBSPPart &part);
    void Reserve(CBSPWorldImpl &world);
};
