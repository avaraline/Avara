#pragma once
#include "CTagBase.h"

class CCompactTagBase : public CTagBase {
public:
    CCompactTagBase() {}
    virtual ~CCompactTagBase() {}

    virtual Handle ConvertToHandle(); // Convert to something that can be stored as a resource.
    virtual void ConvertFromHandle(Handle theHandle); // Does the opposite of ConvertToHandle.
};
