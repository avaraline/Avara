#pragma once

#include "Types.h"
#include "PlayerConfig.h"

#include <sstream>
#include <string>

void UseResFile(std::string filename);
std::string OSTypeString(OSType t);
OSType StringOSType(std::string s);
Handle GetResource(OSType theType, short theID);
Handle GetNamedResource(OSType theType, std::string name);

void WriteResource(Handle theResource);
void ChangedResource(Handle theResource);
void ReleaseResource(Handle theResource);
void DetachResource(Handle theResource);
