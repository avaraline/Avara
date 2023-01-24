#include "Memory.h"
#include <limits.h>

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

static size_t my_strnlen_s(const char* s, size_t len) {
    size_t i = 0;
    for (; i < len && s[i] != '\0'; ++i);
    return i;
}

// TODO: NO! BAD!
static unsigned char *CStringtoPascalString(const char *s) {
    size_t len = my_strnlen_s(s, UCHAR_MAX);
    if (len == 0) return 0;
    unsigned char *p = new unsigned char[len + 1];
    p[0] = len;
    memmove(p + 1, s, len);
    return p;
}