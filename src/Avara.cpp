/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: Avara.c
    Created: Sunday, November 13, 1994, 21:18
    Modified: Monday, September 2, 1996, 17:39
*/

#include "AvaraGL.h"
#include "AvaraTCP.h"
#include "CAvaraApp.h"
#include "CAvaraGame.h"
#include "CBSPPart.h"
#include "FastMat.h"
#include "Preferences.h"

#include <SDL2/SDL.h>
#include <nanogui/nanogui.h>
#include <string.h>

using namespace nanogui;

#ifdef _WIN32

typedef enum PROCESS_DPI_AWARENESS {
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

HRESULT(WINAPI *SetProcessDpiAwareness)(PROCESS_DPI_AWARENESS dpiAwareness);
#endif

void SetHiDPI() {
#ifdef _WIN32
    void *shcoreDLL = SDL_LoadObject("SHCORE.DLL");
    SetProcessDpiAwareness =
        (HRESULT(WINAPI *)(PROCESS_DPI_AWARENESS))SDL_LoadFunction(shcoreDLL, "SetProcessDpiAwareness");
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif
}

void NullLogger(void *userdata, int category, SDL_LogPriority priority, const char *message) {}

int main(int argc, char *argv[]) {
    // Allow Windows to run in HiDPI mode.
    SetHiDPI();

    // Init SDL and nanogui.
    init();

    // SDL_LogSetOutputFunction(&NullLogger, NULL);

    // Init Avara stuff.
    InitMatrix();
    OpenAvaraTCP();

    // The Avara application itself.
    CAvaraApp *app = new CAvaraAppImpl();

    // process command-line arguments
    std::string connectAddress;
    bool host = false;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-p" || arg == "--port") {
            int port = atoi(argv[++i]);  // pre-inc to next arg
            app->Set(kDefaultClientUDPPort, port);
        } else if (arg == "-n" || arg == "--name") {
            app->Set(kPlayerNameTag, std::string(argv[++i]));
        } else if (arg == "-c" || arg == "--connect") {
            connectAddress = std::string(argv[++i]);
        } else if (arg == "-h" || arg == "--host") {
            host = true;
        } else if (arg == "-f" || arg == "--frametime") {
            long frameTime = atol(argv[++i]);  // pre-inc to next arg
            app->GetGame()->SetFrameTime(frameTime);
        } else {
            SDL_Log("Unknown command-line argument '%s'\n", argv[i]);
            exit(1);
        }
    }

    if(host == true) {
        app->GetNet()->ChangeNet(kServerNet, "");
    } else if(connectAddress.size() > 0) {
        app->GetNet()->ChangeNet(kClientNet, connectAddress);
    }

    // the mainloop should be sufficiently fast so that frames occur near their scheduled time
    mainloop(app->GetGame()->frameTime / 4);

    app->Done();

    // Shut it down!!
    shutdown();

    return 0;
}
