/*
    Copyright ©1996, Juri Munkki
    All rights reserved.

    File: CYonSphere.h
    Created: Thursday, August 22, 1996, 03:01
    Modified: Thursday, August 22, 1996, 03:01
*/

#pragma once
#include "CAbstractYon.h"

class CYonSphere : public CAbstractYon {
public:
    virtual void BeginScript();
    virtual CAbstractActor *EndScript();
};
