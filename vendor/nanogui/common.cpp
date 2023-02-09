/*
    nanogui/nanogui.cpp -- Basic initialization and utility routines

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/screen.h>

#if defined(_WIN32)
#  include <windows.h>
#endif

#include <nanogui/opengl.h>
#include <map>
#include <thread>
#include <chrono>
#include <iostream>
#include <algorithm>

#if !defined(_WIN32)
#  include <locale.h>
#  include <signal.h>
#  include <sys/dir.h>
#endif

NAMESPACE_BEGIN(nanogui)

extern std::vector<Screen *> __nanogui_screens;

#if defined(__APPLE__)
  extern void disable_saved_application_state_osx();
#endif

double gStartTime = 0.0;

void setTime(double t) {
    gStartTime = t;
}

double getTime() {
    return ((double)SDL_GetTicks() / (double)1000.0) - gStartTime;
}

void init() {
    #if !defined(_WIN32)
        /* Avoid locale-related number parsing issues */
        setlocale(LC_NUMERIC, "C");
    #endif

    #if defined(__APPLE__)
        disable_saved_application_state_osx();
    #endif

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        throw std::runtime_error("Could not initialize SDL!");

    setTime(0);
}

static bool mainloop_active = false;
uint64_t last_frame = 0;

void mainloop(int refresh) {
    if (mainloop_active)
        throw std::runtime_error("Main loop is already running!");

    mainloop_active = true;

    //try {
        SDL_Event theEvent;

        while (mainloop_active) {
            last_frame = SDL_GetTicks64();
            int numScreens = 0;
            for(auto screen : __nanogui_screens) {
                if (!screen->visible()) {
                    continue;
                }
                screen->idle();
                screen->drawAll();
                numScreens++;
            }

            if (numScreens == 0) {
                /* Give up if there was nothing to draw */
                mainloop_active = false;
                break;
            }

            int result = SDL_PollEvent(&theEvent);
            if(result) {
                if (theEvent.type == SDL_QUIT) {
                    mainloop_active = false;
                }
                for(auto screen : __nanogui_screens) {
                    screen->handleSDLEvent(theEvent);
                }
            }
            /* Wait for mouse/keyboard or empty refresh events */
            auto now = SDL_GetTicks64();
            while(last_frame + refresh > now) { 
                SDL_Delay(1);
                now = SDL_GetTicks64(); 
            }
        }

        /* Process events once more */
        SDL_PollEvent(&theEvent);
    //} catch (const std::exception &e) {
    //    std::cerr << "Caught exception in main loop: " << e.what() << std::endl;
    //    leave();
    //}
}

void leave() {
    mainloop_active = false;
}

bool active() {
    return mainloop_active;
}

void shutdown() {
    SDL_Quit();
}

uint32_t utf8_decode(char *p, size_t len) {
    uint32_t codepoint = 0;
    size_t i = 0;

    if (!len)
        return 0;

    for (; i < len; ++i) {
        if (i == 0) {
            codepoint = (0xff >> len) & *p;
        }
        else {
            codepoint <<= 6;
            codepoint |= 0x3f & *p;
        }
        if (!*p)
            return 0;
        p++;
    }

    return codepoint;
}

std::array<char, 8> utf8(int c) {
    std::array<char, 8> seq;
    int n = 0;
    if (c < 0x80) n = 1;
    else if (c < 0x800) n = 2;
    else if (c < 0x10000) n = 3;
    else if (c < 0x200000) n = 4;
    else if (c < 0x4000000) n = 5;
    else if (c <= 0x7fffffff) n = 6;
    seq[n] = '\0';
    switch (n) {
        case 6: seq[5] = 0x80 | (c & 0x3f); c = c >> 6; c |= 0x4000000;
        case 5: seq[4] = 0x80 | (c & 0x3f); c = c >> 6; c |= 0x200000;
        case 4: seq[3] = 0x80 | (c & 0x3f); c = c >> 6; c |= 0x10000;
        case 3: seq[2] = 0x80 | (c & 0x3f); c = c >> 6; c |= 0x800;
        case 2: seq[1] = 0x80 | (c & 0x3f); c = c >> 6; c |= 0xc0;
        case 1: seq[0] = c;
    }
    return seq;
}

int __nanogui_get_image(NVGcontext *ctx, const std::string &name, uint8_t *data, uint32_t size) {
    static std::map<std::string, int> iconCache;
    auto it = iconCache.find(name);
    if (it != iconCache.end())
        return it->second;
    int iconID = nvgCreateImageMem(ctx, 0, data, size);
    if (iconID == 0)
        throw std::runtime_error("Unable to load resource data.");
    iconCache[name] = iconID;
    return iconID;
}

std::string file_dialog(const std::vector<std::pair<std::string, std::string>> &filetypes, bool save) {
    auto result = file_dialog(filetypes, save, false);
    return result.empty() ? "" : result.front();
}

#if !defined(__APPLE__)
std::vector<std::string> file_dialog(const std::vector<std::pair<std::string, std::string>> &filetypes, bool save, bool multiple) {
    static const int FILE_DIALOG_MAX_BUFFER = 16384;
    if (save && multiple) {
        throw std::invalid_argument("save and multiple must not both be true.");
    }

#if defined(_WIN32)
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    wchar_t tmp[FILE_DIALOG_MAX_BUFFER];
    ofn.lpstrFile = tmp;
    ZeroMemory(tmp, sizeof(tmp));
    ofn.nMaxFile = FILE_DIALOG_MAX_BUFFER;
    ofn.nFilterIndex = 1;

    std::string filter;

    if (!save && filetypes.size() > 1) {
        filter.append("Supported file types (");
        for (size_t i = 0; i < filetypes.size(); ++i) {
            filter.append("*.");
            filter.append(filetypes[i].first);
            if (i + 1 < filetypes.size())
                filter.append(";");
        }
        filter.append(")");
        filter.push_back('\0');
        for (size_t i = 0; i < filetypes.size(); ++i) {
            filter.append("*.");
            filter.append(filetypes[i].first);
            if (i + 1 < filetypes.size())
                filter.append(";");
        }
        filter.push_back('\0');
    }
    for (auto pair : filetypes) {
        filter.append(pair.second);
        filter.append(" (*.");
        filter.append(pair.first);
        filter.append(")");
        filter.push_back('\0');
        filter.append("*.");
        filter.append(pair.first);
        filter.push_back('\0');
    }
    filter.push_back('\0');

    int size = MultiByteToWideChar(CP_UTF8, 0, &filter[0], (int)filter.size(), NULL, 0);
    std::wstring wfilter(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &filter[0], (int)filter.size(), &wfilter[0], size);

    ofn.lpstrFilter = wfilter.data();

    if (save) {
        ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
        if (GetSaveFileNameW(&ofn) == FALSE)
            return {};
    } else {
        ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if (multiple)
            ofn.Flags |= OFN_ALLOWMULTISELECT;
        if (GetOpenFileNameW(&ofn) == FALSE)
            return {};
    }

    size_t i = 0;
    std::vector<std::string> result;
    while (tmp[i] != '\0') {
        std::string filename;
        int tmpSize = (int)wcslen(&tmp[i]);
        if (tmpSize > 0) {
            int filenameSize = WideCharToMultiByte(CP_UTF8, 0, &tmp[i], tmpSize, NULL, 0, NULL, NULL);
            filename.resize(filenameSize, 0);
            WideCharToMultiByte(CP_UTF8, 0, &tmp[i], tmpSize, &filename[0], filenameSize, NULL, NULL);
        }

        result.emplace_back(filename);
        i += tmpSize + 1;
    }

    if (result.size() > 1) {
        for (i = 1; i < result.size(); ++i) {
            result[i] = result[0] + "\\" + result[i];
        }
        result.erase(begin(result));
    }

    return result;
#else
    char buffer[FILE_DIALOG_MAX_BUFFER];
    buffer[0] = '\0';

    std::string cmd = "zenity --file-selection ";
    // The safest separator for multiple selected paths is /, since / can never occur
    // in file names. Only where two paths are concatenated will there be two / following
    // each other.
    if (multiple)
        cmd += "--multiple --separator=\"/\" ";
    if (save)
        cmd += "--save ";
    cmd += "--file-filter=\"";
    for (auto pair : filetypes)
        cmd += "\"*." + pair.first + "\" ";
    cmd += "\"";
    FILE *output = popen(cmd.c_str(), "r");
    if (output == nullptr)
        throw std::runtime_error("popen() failed -- could not launch zenity!");
    while (fgets(buffer, FILE_DIALOG_MAX_BUFFER, output) != NULL)
        ;
    pclose(output);
    std::string paths(buffer);
    paths.erase(std::remove(paths.begin(), paths.end(), '\n'), paths.end());

    std::vector<std::string> result;
    while (!paths.empty()) {
        size_t end = paths.find("//");
        if (end == std::string::npos) {
            result.emplace_back(paths);
            paths = "";
        } else {
            result.emplace_back(paths.substr(0, end));
            paths = paths.substr(end + 1);
        }
    }

    return result;
#endif
}
#endif

void Object::decRef(bool dealloc) const noexcept {
    --m_refCount;
    if (m_refCount == 0 && dealloc) {
        delete this;
    } else if (m_refCount < 0) {
        fprintf(stderr, "Internal error: Object reference count < 0!\n");
        abort();
    }
}

Object::~Object() { }

NAMESPACE_END(nanogui)

