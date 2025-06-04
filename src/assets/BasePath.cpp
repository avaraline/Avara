#include "BasePath.h"
#include <SDL2/SDL.h>
#include <cstdlib>


#define RSRC_PATH_ENV_VAR "AVARA_RSRC_PATH"
bool basepathLookupDone = false;
std::string basepath = "";

void SetBasePath(char* bp) {
    // if set from a command line argument, override env var
    basepath.assign(bp);
    basepathLookupDone = true;
    SDL_Log("Asset base path set by --basepath to %s", basepath.c_str());
}

std::string GetBasePath() {
    if (!basepathLookupDone) {
        // look at environment variable
        if (const char* env_bp = std::getenv(RSRC_PATH_ENV_VAR)) {
            size_t length = strlen(env_bp);
            if (length > 0) {
                basepath.assign(env_bp);
            }
        }
        if (basepath.length() < 1)
            basepath.assign(SDL_GetBasePath());
        basepathLookupDone = true;
        SDL_Log("Asset base path set to %s", basepath.c_str());
    }
    return basepath;
}

