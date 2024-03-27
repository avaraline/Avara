/*
    Copyright ©1994-1996, Juri Munkki
    All rights reserved.

    File: Avara.c
    Created: Sunday, November 13, 1994, 21:18
    Modified: Monday, September 2, 1996, 17:39
*/

#include "AvaraTCP.h"
#include "CAvaraApp.h"
#include "CAvaraGame.h"
#include "CPlayerManager.h"
#include "CBSPPart.h"
#include "FastMat.h"
#include "Preferences.h"

#ifdef _WIN32
#include <Windows.h>
#include <ShellAPI.h>
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

#include <SDL2/SDL.h>
#include <nanogui/nanogui.h>
#include <string.h>

using namespace nanogui;

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
    std::vector<std::string> textCommands;
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
            app->Set(kLastAddress, connectAddress);
        } else if (arg == "-s" || arg == "--serve" ||
                   arg == "-S" || arg == "--Serve") {
            host = true;
            app->Set(kTrackerRegister, arg[1] == 'S' || arg[2] == 'S');
        } else if (arg == "-f" || arg == "--frametime") {
            uint16_t frameTime = atol(argv[++i]);  // pre-inc to next arg
            app->GetGame()->SetFrameTime(frameTime);
        } else if (arg == "-i" || arg == "--keys-from-stdin") {
            app->GetGame()->SetKeysFromStdin();
        } else if (arg == "-if" || arg == "--keys-from-file") {
            // redirect a playback file to stdin
            freopen(argv[++i], "r", stdin);
            app->GetGame()->SetKeysFromStdin();
        } else if (arg == "-o" || arg == "--keys-to-stdout") {
            app->GetGame()->SetKeysToStdout();
        } else if (arg == "-/" || arg == "--command") {
            std::string textCommand = argv[++i];
            if (textCommand[0] != '/') {
                textCommand.insert(0, "/");
            }
            textCommands.push_back(textCommand);
        } else {
            SDL_Log("Unknown command-line argument '%s'\n", argv[i]);
            exit(1);
        }
    }

    auto p = CPlayerManagerImpl::LocalPlayer();
    auto *tui = ((CAvaraAppImpl *)app)->GetTui();
    auto defaultCmd = "/rand avara aa emo ex #fav -#bad -#koth";
    if (textCommands.size() > 0) {
        for (auto cmd: textCommands) {
            tui->ExecuteMatchingCommand(cmd, p);
        }
    } else {
        tui->ExecuteMatchingCommand(defaultCmd, p);
    }

    if(host == true) {
        app->GetNet()->ChangeNet(kServerNet, "");
    } else if(connectAddress.size() > 0) {
        app->GetNet()->ChangeNet(kClientNet, connectAddress);
    }

    // outside of the game, use INACTIVE_LOOP_REFRESH (no need to poll when not playing)
    mainloop(INACTIVE_LOOP_REFRESH);

    app->Done();

    // Shut it down!!
    shutdown();

    return 0;
}
