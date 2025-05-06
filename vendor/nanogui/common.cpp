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
#  include <commdlg.h>
#  include <prsht.h>
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

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        throw std::runtime_error("Could not initialize SDL!");

    setTime(0);
}

static bool mainloop_active = false;
int throttle = 0;

void mainloop(int refresh) {
    throttle = refresh;
    if (mainloop_active)
        throw std::runtime_error("Main loop is already running!");

    mainloop_active = true;

    //try {
        SDL_Event theEvent;

        while (mainloop_active) {
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

            /* Wait for mouse/keyboard or empty refresh events */
            int result = SDL_WaitEventTimeout(&theEvent, throttle);  // uses SDL_PollEvent(&theEvent) when throttle == 0
            if(result) {
                if (theEvent.type == SDL_QUIT) {
                    mainloop_active = false;
                }
                for(auto screen : __nanogui_screens) {
                    screen->handleSDLEvent(theEvent);
                }
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
