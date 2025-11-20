#pragma once

#include <fstream>
#include <SDL2/SDL.h>

class Logging {
private:
    static std::ofstream logFile;
public:
    static void OpenLog();
    static void Log(void *userdata, int category, SDL_LogPriority priority, const char *message);
    static void CloseLog();
};
