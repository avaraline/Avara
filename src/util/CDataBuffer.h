#pragma once

#include "Types.h"

#include <fstream>
#include <iostream>

class CDataBuffer {
public:
    // CDataBuffer(const char *filename);
    CDataBuffer(Handle data);

    virtual ~CDataBuffer();

    Boolean More();

    void Skip(size_t num);

    void Read(void *into, size_t num);
    void ReadRect(Rect *r);
    void ReadPoint(Point *p);
    void ReadColor(RGBColor *c);

    int8_t Byte();
    int16_t Short();
    int32_t Int();

    uint8_t UByte() { return (uint8_t)Byte(); }
    uint16_t UShort() { return (uint16_t)Short(); }
    uint32_t UInt() { return (uint32_t)Int(); }

    size_t GetPos();
    void Align();

protected:
    // std::ifstream stream;
    Ptr ptr;
    size_t pos, total;
};
