#include "Logging.h"
#include "Preferences.h"

std::ofstream Logging::logFile;


static const char * const _priority_names[] = {
    NULL,
    "TRACE",
    "VERBOSE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "CRITICAL"
};

void Logging::OpenLog() {
    try {
        std::string logPath = PrefPath("avara.log");
        logFile.open(logPath, std::ios::out | std::ios::trunc);
        if (logFile.fail()) {
            SDL_Log("Failed to open log file at %s.", logPath.c_str());
        }
        else {
            SDL_LogSetOutputFunction(Logging::Log, nullptr);
        }
    }
    catch (std::exception e) {
        SDL_Log("Error opening log file.");
    }
}

void Logging::Log(void *userdata, int category, SDL_LogPriority priority, const char *message) {
    if (logFile.is_open()) {
        std::time_t t = std::time(nullptr);
        std::tm tm = *std::localtime(&t);
        logFile << std::put_time(&tm, "%Y%m%d %H%M%S ");
        logFile << SDL_GetTicks64() << " " << _priority_names[priority] << ": " << message << "\n";
        logFile.flush();
    }
    printf("%s: %s\n", _priority_names[priority], message);
}

void Logging::CloseLog() {
    logFile.flush();
    logFile.close();
}
