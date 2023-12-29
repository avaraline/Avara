#pragma once

#include "Types.h"

#include <vector>

typedef struct {
    uint32_t versNum;
    uint32_t loopStart;
    uint32_t loopEnd;
    uint32_t loopCount;
    UnsignedFixed baseRate;
} HSNDRecord;

class OggFile {
public:
    HSNDRecord hsnd;
    std::vector<uint8_t> samples;

    OggFile(std::string path, HSNDRecord hsnd);
};
