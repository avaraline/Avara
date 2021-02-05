#pragma once

#include "Types.h"

#include <string>
#include <vector>

void UseResFile(std::string filename);

std::vector<std::string> LevelDirListing();

Handle GetResource(OSType theType, short theID);
Handle GetNamedResource(OSType theType, std::string name);

void WriteResource(Handle theResource);
void ChangedResource(Handle theResource);
void ReleaseResource(Handle theResource);
void DetachResource(Handle theResource);

void GetIndString(Str255 theString, short strListID, short index);

char *BundlePath(const char *rel);
