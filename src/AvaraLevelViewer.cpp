/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: Avara.c
    Created: Sunday, November 13, 1994, 21:18
    Modified: Monday, September 2, 1996, 17:39
*/

#include "AvaraGL.h"
#include "AvaraTCP.h"
#include "CLevelViewerApp.h"
#include "CAvaraGame.h"
#include "FastMat.h"

#include <nanogui/nanogui.h>
#include <string.h>

using namespace nanogui;

#ifdef _WIN32
#include <Windows.h>

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

    // Init SDL_net
    if (SDLNet_Init() != 0) {
        SDL_Log("Unable to initialize SDL_net: %s", SDLNet_GetError());
        return 1;
    }

    // SDL_LogSetOutputFunction(&NullLogger, NULL);

    // Init Avara stuff.
    InitMatrix();
    OpenAvaraTCP();

    // The Avara application itself.
    CLevelViewerApp *app = new CLevelViewerApp();

    mainloop(app->GetGame()->frameTime / 4);

    app->Done();

    // Shut it down!!
    shutdown();

    return 0;
}
