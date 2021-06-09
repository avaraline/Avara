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
#include "CNetManager.h"
#include "CBSPPart.h"
#include "FastMat.h"
#include "Preferences.h"

#include <string.h>

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
#if !defined(_WIN32)
    /* Avoid locale-related number parsing issues */
    setlocale(LC_NUMERIC, "C");
#endif

#if defined(__APPLE__)
    disable_saved_application_state_osx();
#endif

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        throw std::runtime_error("Could not initialize SDL!");

    // SDL_LogSetOutputFunction(&NullLogger, NULL);

    // Init Avara stuff.
    InitMatrix();
    OpenAvaraTCP();

    // The Avara application itself.
    CAvaraAppImpl *app = new CAvaraAppImpl();

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

    bool main_loop_active = true;
    int refresh = app->GetGame()->frameTime / 4;
    SDL_Event event;

    while (main_loop_active) {
        //if (!app->visible()) {
        //    continue;
        //}
        app->idle();
        if (SDL_WaitEventTimeout(&event, refresh)) {
            if (event.type == SDL_QUIT) {
                main_loop_active = false;
            }
            app->handleSDLEvent(event);
        }
        app->drawContents();

        SDL_GL_SwapWindow(app->window);
    }

    app->Done();

    // Shut it down!!
    //SDL_Quit();

    return 0;
}
