#pragma once

#include "Types.h"
#include "PlayerConfig.h"
#include "SoundSystemDefines.h"

#include <string>
#include <vector>
#include <json.hpp>

void UseResFile(std::string filename);
void UseLevelFolder(std::string folder);
std::string OSTypeString(OSType t);
OSType StringOSType(std::string s);
Handle GetResource(OSType theType, short theID);
Handle GetNamedResource(OSType theType, std::string name);

void WriteResource(Handle theResource);
void ChangedResource(Handle theResource);
void ReleaseResource(Handle theResource);
void DetachResource(Handle theResource);

void GetIndString(Str255 theString, short strListID, short index);

char *BundlePath(const char *rel);

void LevelDirListing();
std::vector<std::string> LevelDirNameListing();
int8_t GetVersionForLevelSet(std::string levelset);

nlohmann::json LoadLevelListFromJSON(std::string set);
nlohmann::json GetManifestJSON(std::string set);
nlohmann::json GetKeyFromSetJSON(std::string rsrc, std::string key, std::string default_id);

char* GetBSPPath(int resId);
std::string GetALFPath(std::string alfname);
std::string GetDefaultScript();
std::string GetBaseScript();
void LoadHullFromSetJSON(HullConfigRecord *hull, short resId);
void LoadDefaultOggFiles();
void LoadLevelOggFiles(std::string set);
void LoadOggFile(short resId, const char* filename);
SampleHeaderHandle LoadSampleHeaderFromSetJSON(short resId, SampleHeaderHandle sampleList);
