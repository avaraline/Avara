#define APPLICATIONMAIN
#include "CApplication.h"

#include "Preferences.h"
#include "Types.h"

#include <SDL2/SDL.h>
#include <fstream>
#include <string>
#include <vector>

#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"

#include "AvaraGL.h"



static float get_pixel_ratio(SDL_Window *window) {
#if defined(_WIN32)
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(window, &info);
    HWND hWnd = info.info.win.window;
    HMONITOR monitor = nullptr;
    #if defined(MONITOR_DEFAULTTONEAREST)
        monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    #else
        static HMONITOR (WINAPI *MonitorFromWindow_)(HWND, DWORD) = nullptr;
        static bool MonitorFromWindow_tried = false;
        if (!MonitorFromWindow_tried) {
            auto user32 = LoadLibrary(TEXT("user32"));
            if (user32)
                MonitorFromWindow_ = (decltype(MonitorFromWindow_)) GetProcAddress(user32, "MonitorFromWindow");
            MonitorFromWindow_tried = true;
        }
        if (MonitorFromWindow_)
            monitor = MonitorFromWindow_(hWnd, 2);
    #endif  // defined(MONITOR_DEFAULTTONEAREST)
    /* The following function only exists on Windows 8.1+, but we don't want to make that a dependency */
    static HRESULT (WINAPI *GetDpiForMonitor_)(HMONITOR, UINT, UINT*, UINT*) = nullptr;
    static bool GetDpiForMonitor_tried = false;

    if (!GetDpiForMonitor_tried) {
        auto shcore = LoadLibrary(TEXT("shcore"));
        if (shcore)
            GetDpiForMonitor_ = (decltype(GetDpiForMonitor_)) GetProcAddress(shcore, "GetDpiForMonitor");
        GetDpiForMonitor_tried = true;
    }

    if (GetDpiForMonitor_ && monitor) {
        uint32_t dpiX, dpiY;
        if (GetDpiForMonitor_(monitor, 0 /* effective DPI */, &dpiX, &dpiY) == S_OK)
            return dpiX / 96.0;
    }
    return 1.f;
#elif defined(__linux__)
    (void) window;

    float ratio = 1.0f;
    FILE *fp;
    /* Try to read the pixel ratio from KDEs config */
    auto currentDesktop = std::getenv("XDG_CURRENT_DESKTOP");
    if (currentDesktop && currentDesktop == std::string("KDE")) {
        fp = popen("kreadconfig5 --group KScreen --key ScaleFactor", "r");
        if (!fp)
            return 1;

        if (fscanf(fp, "%f", &ratio) != 1)
            return 1;
    } else {
        /* Try to read the pixel ratio from GTK */
        fp = popen("gsettings get org.gnome.desktop.interface scaling-factor", "r");
        if (!fp)
            return 1;

        int ratioInt = 1;
        if (fscanf(fp, "uint32 %i", &ratioInt) != 1)
            return 1;
        ratio = ratioInt;
    }
    if (pclose(fp) != 0)
        return 1;
    return ratio >= 1 ? ratio : 1;

#else
    Vector2i fbSize, size;
    SDL_GL_GetDrawableSize(window, &fbSize[0], &fbSize[1]);
    SDL_GetWindowSize(window, &size[0], &size[1]);
    return (float)fbSize[0] / (float)size[0];
#endif
}


json prefs = ReadPrefs();


CApplication::CApplication(std::string the_title) {
    window_title = the_title;
    gApplication = this;

    auto glMajor = 3;
    auto glMinor = 3;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glMajor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glMinor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    if (prefs[kMultiSamplesTag] > 0) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, prefs[kMultiSamplesTag]);
    }
    auto flags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
    if (prefs[kFullScreenTag]) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    window = SDL_CreateWindow(title().c_str(), 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        prefs[kWindowWidth], 
        prefs[kWindowHeight], 
        flags);
    window_id = SDL_GetWindowID(window);
    gl_context = SDL_GL_CreateContext(window);

    if (!window || !gl_context) {
        SDL_Log("Could not create an OpenGL %s.%s context", 
            std::to_string(glMajor).c_str(),
            std::to_string(glMinor).c_str());
        return;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    //glGetError();

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
        SDL_Log("Could not initialize GLAD!");
        return;
    }
    //glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM
    
    

    window_id = SDL_GetWindowID(window);
    SDL_GetWindowSize(window, &win_size_x, &win_size_y);
    SDL_GL_GetDrawableSize(window, &fb_size_x, &fb_size_y);
    pixel_ratio = get_pixel_ratio(window);
    
    SDL_Log("Window Size: %d by %d", win_size_x, win_size_y);
    SDL_Log("FB Size: %d by %d", fb_size_x, fb_size_y);
    glViewport(0, 0, fb_size_x, fb_size_y);
    glClearColor(.2, .2, .2, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    SDL_GL_SetSwapInterval(0);
    SDL_GL_SwapWindow(window);

    GLint nStencilBits = 0, nSamples = 0;
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER,
        GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &nStencilBits);
    glGetIntegerv(GL_SAMPLES, &nSamples);
    int nvgflags = 0;
    if (nStencilBits >= 8)
       nvgflags |= NVG_STENCIL_STROKES;
    if (nSamples <= 1)
       nvgflags |= NVG_ANTIALIAS;

    nvg_context = nvgCreateGL3(nvgflags);

    if (nvg_context == nullptr) {
        SDL_Log("Could not initialize NanoVG!");
        return;
    }

    nvgBeginFrame(nvg_context, win_size_x, win_size_y, pixel_ratio);
    nvgEndFrame(nvg_context);
#if defined(__APPLE__)
    /* Poll for events once before starting a potentially
       lengthy loading process. This is needed to be
       classified as "interactive" by other software such
       as iTerm2 */
    SDL_Event dummyEvent;
    SDL_PollEvent(&dummyEvent);
#endif
    setResizeCallback([this](int new_x, int new_y) { this->WindowResized(new_x, new_y); return true; });

}

CApplication::~CApplication() {
    if (nvg_context)
        nvgDeleteGL3(nvg_context);
    if (window)
        SDL_DestroyWindow(window);
}

void CApplication::Unregister(CWindow *win) {
    std::vector<CWindow*>::iterator position = std::find(windowList.begin(), windowList.end(), win);
    if (position != windowList.end())
        windowList.erase(position);
}

void CApplication::Done() {
    for (auto win : windowList) {
        win->saveState();
    }

    prefs[kWindowWidth] = win_size_x;
    prefs[kWindowHeight] = win_size_y;
    prefs.erase(kDefaultClientUDPPort);  // don't save client port
    WritePrefs(prefs);
}

void CApplication::BroadcastCommand(int theCommand) {
    if (!DoCommand(theCommand)) {
        for (auto win : windowList) {
            if (win->DoCommand(theCommand))
                break;
        }
    }
}

void CApplication::PrefChanged(std::string name) {
    for (auto win : windowList) {
        win->PrefChanged(name);
    }
}

bool CApplication::resizeCallbackEvent(int, int) {
    int fb_tmp_size_x, fb_tmp_size_y, win_tmp_size_x, win_tmp_size_y;
    SDL_GL_GetDrawableSize(window, &fb_tmp_size_x, &fb_tmp_size_y);
    SDL_GetWindowSize(window, &win_tmp_size_x, &win_tmp_size_y);
#if defined(_WIN32) || defined(__linux__)
    win_tmp_size_x = win_tmp_size_x / pixel_ratio;
    win_tmp_size_y = win_tmp_size_y / pixel_ratio;
#endif

    if (win_tmp_size_x == win_tmp_size_y == 0 ||
        fb_size_x == fb_size_y == 0) 
        return false;

    fb_size_x = fb_tmp_size_x;
    fb_size_y = fb_tmp_size_y;
    win_size_x = win_tmp_size_x;
    win_size_y = win_tmp_size_y;
    return resize_callback(win_size_x, win_size_y);
}


bool CApplication::handleSDLEvent(SDL_Event &event) {
    // By default, give nanogui first crack at events.
    // bool handled = nanogui::Screen::handleSDLEvent(event);
    bool handled = false;
    switch(event.type) {
        case SDL_WINDOWEVENT:
            if (event.window.windowID != window_id)
                return false;
            switch(event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    return resizeCallbackEvent(event.window.data1, event.window.data2);
                    break;
            }
            break;
    }
    if (!handled) {
        handled = HandleEvent(event);
    }
    return handled;
}

std::string CApplication::String(const std::string name) {
    return prefs[name];
}

long CApplication::Number(const std::string name) {
    return prefs[name];
}

long CApplication::Number(const std::string name, const long defaultValue) {
    if (prefs[name].is_number()) {
        return prefs[name];
    }
    return defaultValue;
}

json CApplication::Get(const std::string name) {
    return prefs[name];
}

void CApplication::Set(const std::string name, const std::string value) {
    prefs[name] = value;
    PrefChanged(name);
}

void CApplication::Set(const std::string name, long value) {
    prefs[name] = value;
    PrefChanged(name);
}

void CApplication::Set(const std::string name, json value) {
    prefs[name] = value;
    PrefChanged(name);
}
