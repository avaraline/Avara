#pragma once

#include "CBSPPart.h"

class CCompoundShape : public CBSPPart {
public:
    CCompoundShape();
    virtual ~CCompoundShape() override;
    void Append();
};
