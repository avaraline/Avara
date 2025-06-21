#include "OggFile.h"
#include <SDL2/SDL.h>
#include "FastMat.h"


#include <stb_vorbis.h>

const static int STBVorbisErrorMax = STBVorbisError::VORBIS_ogg_skeleton_not_supported;
const static char * STBVorbisErrorString(int e) {
    if (20 <= e && e < 30) {
        return "Vorbis error - check file encoding";
    }
    if (30 <= e && e <= STBVorbisErrorMax) {
        return "OGG error - check file encoding";
    }
    switch (e) {
        case STBVorbisError::VORBIS_need_more_data: return "waiting for more data";
        case STBVorbisError::VORBIS_invalid_api_mixing: return "can't mix API modes";
        case STBVorbisError::VORBIS_outofmem: return "stb_vorbis out of memory";
        case STBVorbisError::VORBIS_feature_not_supported: return "ogg uses floor 0 (not supported)";
        case STBVorbisError::VORBIS_file_open_failure: return "fopen() failed";
        case STBVorbisError::VORBIS_seek_without_length: return "can't seek in unknown-length file";
        case STBVorbisError::VORBIS_unexpected_eof: return "EOF error, file is truncated?";
        case STBVorbisError::VORBIS_seek_invalid: return "seek past EOF";
        default: return "unknown error";
    }
}


OggFile::OggFile(std::string path, HSNDRecord hsnd)
{
    this->hsnd = hsnd;

    if (this->hsnd.versNum <= 1) {
        this->hsnd.baseRate = FIX1;
    }

    samples = {};
    
    int error = -1;
    stb_vorbis *v = stb_vorbis_open_filename(path.c_str(), &error, NULL);
    auto inf = stb_vorbis_get_info(v);
    if (inf.channels > 1) {
        SDL_Log("WARNING: ogg has too many channels. Avara cannot mix this sound to mono, it was not loaded. Path: %s", path.c_str());
        return;
    }
    if (error != -1 && error != STBVorbisError::VORBIS__no_error) {
        SDL_Log("Error loading ogg: (%i) %s. Path: %s", error, STBVorbisErrorString(error), path.c_str());
        return;
    }
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
