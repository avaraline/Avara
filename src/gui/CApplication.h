#pragma once

#include "CWindow.h"

#include <SDL2/SDL.h>
#include <json.hpp>
#include <string>
#include <vector>
#include <glad/glad.h>

struct NVGcontext { /* Opaque handle type, never de-referenced */};


using json = nlohmann::json;

class CApplication {
public:
    CApplication(std::string title);
    virtual ~CApplication();
    std::string window_title;
    std::string title() { return window_title; }

    void Register(CWindow *win) { windowList.push_back(win); }
    void Unregister(CWindow *win);

    // Broadcast a command to myself, and then any windows, stopping at the fist one to handle it.
    virtual void BroadcastCommand(int theCommand);

    // Called when the application is done and the process is about to exit.
    virtual void Done();

    // Handles a command broadcasted by BroadcastCommand. Returns true if it was actually handled.
    virtual bool DoCommand(int theCommand) { return false; }

    // Events that were not handled by nanogui.
    virtual bool HandleEvent(SDL_Event &event) { return false; }

    // Called when preference values change.
    virtual void PrefChanged(std::string name);

    virtual void WindowResized(int width, int height) {}
    virtual bool resizeCallbackEvent(int, int);
    std::function<bool(int, int)> resizeCallback() const { return resize_callback; }
    void setResizeCallback(const std::function<bool(int, int)> &callback) { resize_callback = callback; }
    // Screen overrides.
    virtual bool handleSDLEvent(SDL_Event &event);

    SDL_Window *sdlWindow() { return window; }
    NVGcontext *NVGContext() { return nvg_context; }

    SDL_Window *window;
    SDL_GLContext gl_context;
    NVGcontext *nvg_context;
    float pixel_ratio = 1;
    int fb_size_x;
    int fb_size_y;
    int win_size_x;
    int win_size_y;
    int throttle;
    
    // templated version works for new types too (float, double, short, etc.)
    template <class T> T Get(const std::string name) {
        // throws json::out_of_range exception if 'name' not a known key
        return static_cast<T>(_prefs.at(name));
    }
    // these getters are here for backwards compatibility and/or readability
    std::string String(const std::string name)       { return Get<std::string>(name); };
    long Number(const std::string name)              { return Get<long>(name); };
    long Number(const std::string name, const long defaultValue);
    bool Boolean(const std::string name)             { return Get<bool>(name); }
    json Get(const std::string name)                 { return Get<json>(name); }

    template <class T> void Set(const std::string name, const T value) {
        _prefs[name] = value;
        PrefChanged(name);
    }

protected:
    static json _prefs;
    std::vector<CWindow *> windowList;
    std::function<bool(int, int)> resize_callback;
    uint32_t window_id;
};

#ifdef APPLICATIONMAIN
CApplication *gApplication;
#else
extern CApplication *gApplication;
#endif
