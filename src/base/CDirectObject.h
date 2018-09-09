/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: CDirectObject.h
    Created: Friday, March 11, 1994, 17:14
    Modified: Tuesday, February 20, 1996, 07:20
*/

#pragma once

class CDirectObject {
public:
    CDirectObject() {}
    virtual ~CDirectObject() {}

    virtual void Dispose();
};
