#pragma once

#include "SoundSystemDefines.h"

#include <vector>

class OggFile {
public:
    HSNDRecord hsnd;
    std::vector<Sample> samples;

    OggFile(std::string path, HSNDRecord hsnd);
};
