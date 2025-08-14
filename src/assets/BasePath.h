#ifndef BASE_PATH_H
#define BASE_PATH_H
#include <string>

// Path separator
#if defined(_WIN32)
#define PATHSEP "\\"
#else
#define PATHSEP "/"
#endif

void SetBasePath(char* bp);
std::string GetBasePath();

#endif
