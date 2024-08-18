#pragma once

#include "Types.h"


/* TODO: NO! BAD!
// For a variety of reasons, the original Avara used Pascal strings in
// several places. The internal parser and dictionary (which serves as
// storage for the simple scripting language) are places you will see 
// these awful, terrible functions used, until they can be ported to 
// use... something else. If your stack trace ends here, I'm sorry in 
// advance.
*/

char *PascalStringtoCString(const unsigned char *p, int len);
char *PascalStringtoCString(const unsigned char *p);
size_t my_strnlen_s(const char* s, size_t len);
unsigned char *CStringtoPascalString(const char *s);
