#include "OggFile.h"

#include <stb_vorbis.h>

OggFile::OggFile(std::string path, HSNDRecord hsnd)
{
    this->hsnd = hsnd;
    samples = {};
    
    int error;
    stb_vorbis *v = stb_vorbis_open_filename(path.c_str(), &error, NULL);
    const size_t numSamples = stb_vorbis_stream_length_in_samples(v);
    samples.reserve(numSamples);
    for(;;) {
        const size_t buffa_length = 512;
        int16_t buffa[buffa_length];
        uint8_t sample;
        int n = stb_vorbis_get_samples_short_interleaved(v, 1, buffa, buffa_length);
        if (n == 0) break;
        for (size_t i = 0; i < buffa_length; ++i) {
            // it is a mystery
            sample = (buffa[i] + INT16_MAX + 1) >> 9;
            samples.push_back(sample);
        }
    }
    stb_vorbis_close(v);
}
