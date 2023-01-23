#include "Memory.h"

static char *PascalStringtoCString(const unsigned char *p, int len) {
    if (len == 0) return const_cast<char *>("");
    char *s = new char[len + 1];
    memcpy(s, p + 1, len);
    s[len] = 0;
    return s;
}

static char *PascalStringtoCString(const unsigned char *p) {
    return PascalStringtoCString(p, p[0]);
}

// TODO: NO! BAD!
static unsigned char *CStringtoPascalString(const char *s) {
    size_t len = std::strlen(s);
    if (len == 0) return 0;
    unsigned char *p = new unsigned char[len + 1];
    p[0] = len;
    memmove(p + 1, s, len);
    return p;
}