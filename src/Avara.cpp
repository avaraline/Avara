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
#include "BasePath.h"

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
#include <sstream>
#include <string>

using namespace nanogui;

void NullLogger(void *userdata, int category, SDL_LogPriority priority, const char *message) {}

// combine 'defaultArgs' and command-line arguments
std::vector<std::string> combinedArgs(std::string defaultArgs, int argc, char* argv[]) {
    std::vector<std::string> args;
    // first parse/insert the defaultArgs
    std::stringstream ss(defaultArgs);
    std::string arg, text;
    while (std::getline(ss, text, ' ')) {
        // look for quoted text
        if (text[0] == '\'') {
            arg = text.substr(1);
            // wait for the entire quoted string
            continue;
        } else if (arg.size() > 0) {
            // append to existing until we find the final quote
            arg += ' ';
            size_t lastChar = text.size() - 1;
            if (text[lastChar] == '\'') {
                arg += text.substr(0, lastChar);
            } else {
                arg += text;
                continue;
            }
        } else {
            arg = text;
        }
        args.push_back(arg);
        arg = "";
    }

    // now add actual command-line arguments (inserted AFTER defaultArgs so they override)
    for (int i = 1; i < argc; i++) {
        args.push_back(std::string(argv[i]));
    }
    return args;
}

int main(int argc, char *argv[]) {
    // Check basepath override.
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--basepath") == 0) {
            SetBasePath(argv[++i]);
        }
    }
    // Allow Windows to run in HiDPI mode.
    SetHiDPI();

    // Init SDL and nanogui.
    init();

    // SDL_LogSetOutputFunction(&NullLogger, NULL);

    // Init Avara stuff.
    InitMatrix();
    OpenAvaraTCP();

    // The Avara application itself.
    CAvaraAppImpl *app = new CAvaraAppImpl();

    // process command-line arguments
    std::string connectAddress;
    std::vector<std::string> textCommands;
    bool host = false;
    std::vector<std::string> args = combinedArgs(app->Get<std::string>(kDefaultArgs), argc, argv);
    for (int i = 0; i < args.size(); i++) {
        std::string &arg = args[i];
        if (arg == "-p" || arg == "--port") {
            int port = atoi(args[++i].c_str());  // pre-inc to next arg
            app->Set(kDefaultClientUDPPort, port);
        } else if (arg == "-n" || arg == "--name") {
            app->Set(kPlayerNameTag, args[++i]);
        } else if (arg == "-c" || arg == "--connect") {
            connectAddress = args[++i];
            app->Set(kLastAddress, connectAddress);
        } else if (arg == "-s" || arg == "--serve" ||
                   arg == "-S" || arg == "--Serve") {
            host = true;
            app->Set(kTrackerRegister, arg[1] == 'S' || arg[2] == 'S');
        } else if (arg == "-f" || arg == "--frametime") {
            uint16_t frameTime = atol(args[++i].c_str());  // pre-inc to next arg
            app->GetGame()->SetFrameTime(frameTime);
        } else if (arg == "-i" || arg == "--keys-from-stdin") {
            app->GetGame()->SetKeysFromStdin();
        } else if (arg == "-if" || arg == "--keys-from-file") {
            // redirect a playback file to stdin
            freopen(args[++i].c_str(), "r", stdin);
            app->GetGame()->SetKeysFromStdin();
        } else if (arg == "-o" || arg == "--keys-to-stdout") {
            app->GetGame()->SetKeysToStdout();
        } else if (arg == "-/" || arg == "--command") {
            std::string textCommand = args[++i];
            if (textCommand[0] != '/') {
                textCommand.insert(0, "/");
            }
            textCommands.push_back(textCommand);
        } else if (arg == "--basepath") {
            // skip, it was handled earlier in main()
            i = i + 2;
        } else {
            SDL_Log("Unknown command-line argument '%s'\n", args[i].c_str());
            exit(1);
        }
    }

    if (textCommands.size() > 0) {
        auto p = CPlayerManagerImpl::LocalPlayer();
        auto *tui = ((CAvaraAppImpl *)app)->GetTui();
        for (auto cmd: textCommands) {
            tui->ExecuteMatchingCommand(cmd, p);
        }
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

#if defined(__IPHONEOS__) || defined(__TVOS__)

#ifndef SDL_MAIN_HANDLED
#ifdef main
#undef main
#endif

int main(int argc, char *argv[])
{
    return SDL_UIKitRunApp(argc, argv, SDL_main);
}
#endif /* !SDL_MAIN_HANDLED */

#endif /* __IPHONEOS__ || __TVOS__ */
