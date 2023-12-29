#pragma once

#include "Types.h"
#include "PlayerConfig.h"

#include <sstream>
#include <string>

// path separator
#if defined(_WIN32)
#define PATHSEP "\\"
#else
#define PATHSEP "/"
#endif

void UseResFile(std::string filename);
std::string OSTypeString(OSType t);
OSType StringOSType(std::string s);
Handle GetResource(OSType theType, short theID);
Handle GetNamedResource(OSType theType, std::string name);

void WriteResource(Handle theResource);
void ChangedResource(Handle theResource);
void ReleaseResource(Handle theResource);
void DetachResource(Handle theResource);

void BundlePath(const char *rel, char *dest);
void BundlePath(std::stringstream &buffa, char *dest);
