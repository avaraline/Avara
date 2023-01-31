#include "Memory.h"
#include <limits.h>
#include <string.h>


/* TODO: NO! BAD!
// For a variety of reasons, the original Avara used Pascal strings in
// several places. The internal parser and dictionary (which serves as
// storage for the simple scripting language) are places you will see 
// these awful, terrible functions used, until they can be ported to 
// use... something else. If your stack trace ends here, I'm sorry in 
// advance.
*/

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
    #ifdef strnlen_s
    return strnlen_s(s, len);
    #else
    size_t i = 0;
    for (; i < len && s[i] != '\0'; ++i);
    return i;
    #endif
}

static unsigned char *CStringtoPascalString(const char *s) {
    size_t len = my_strnlen_s(s, UCHAR_MAX);
    if (len == 0) return 0;
    unsigned char *p = new unsigned char[len + 1];
    p[0] = len;
    memmove(p + 1, s, len);
    return p;
}
