#include "CDataBuffer.h"

#include "Memory.h"

#include <SDL2/SDL.h>

/*
CDataBuffer::CDataBuffer(const char *filename) {
    stream.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
    pos = 0;
    total = stream.tellg();
    stream.seekg(pos);
}
*/

CDataBuffer::CDataBuffer(Handle data) {
    ptr = *data;
    pos = 0;
    total = GetHandleSize(data);
}

CDataBuffer::~CDataBuffer() {
    // stream.close();
}

Boolean CDataBuffer::More() {
    return pos < total;
}

void CDataBuffer::Skip(size_t num) {
    pos += num;
    // stream.seekg(pos);
}

void CDataBuffer::Read(void *into, size_t num) {
    BlockMoveData(ptr + pos, into, num);
    /*
    stream.read((char *)into, num);
    size_t numRead = stream.gcount();
    if(numRead != num) {
        SDL_Log("CDataBuffer::Read expected %lu, got %lu\n", num, numRead);
        abort();
    }
    */
    pos += num;
}

void CDataBuffer::ReadRect(Rect *r) {
    r->top = Short();
    r->left = Short();
    r->bottom = Short();
    r->right = Short();
}

void CDataBuffer::ReadPoint(Point *p) {
    p->v = Short();
    p->h = Short();
}

void CDataBuffer::ReadColor(RGBColor *c) {
    c->red = Short();
    c->green = Short();
    c->blue = Short();
}

int8_t CDataBuffer::Byte() {
    int8_t num = 0;
    Read(&num, 1);
    return num;
}

int16_t CDataBuffer::Short() {
    int16_t num = 0;
    Read(&num, 2);
    return ntohs(num);
}

int32_t CDataBuffer::Int() {
    int32_t num = 0;
    Read(&num, 4);
    return ntohl(num);
}

size_t CDataBuffer::GetPos() {
    return pos;
}

void CDataBuffer::Align() {
    if (pos % 2 != 0) {
        Skip(1);
    }
}
