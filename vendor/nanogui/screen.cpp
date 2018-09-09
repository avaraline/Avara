/*
    src/screen.cpp -- Top-level widget and interface between NanoGUI and SDL

    A significant redesign of this code was contributed by Christian Schueller.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/screen.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>
#include <nanogui/window.h>
#include <nanogui/popup.h>
#include <iostream>
#include <algorithm>

#include <SDL2/SDL_syswm.h>

#if defined(_WIN32)
    #include <windows.h>
#endif

/* Allow enforcing the GL2 implementation of NanoVG */
#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg_gl.h>

NAMESPACE_BEGIN(nanogui)

std::vector<Screen *> __nanogui_screens;

#if defined(NANOGUI_GLAD)
static bool gladInitialized = false;
#endif

/* Calculate pixel ratio for hi-dpi devices. */
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

Screen::Screen()
    : Widget(nullptr), mSDLWindow(nullptr), mNVGContext(nullptr),
      mCursor(Cursor::Arrow), mBackground(0.3f, 0.3f, 0.32f, 1.f),
      mShutdownSDLOnDestruct(false), mFullscreen(false) {
}

Screen::Screen(const Vector2i &size, const std::string &caption, bool resizable,
               bool fullscreen, int colorBits, int alphaBits, int depthBits,
               int stencilBits, int nSamples,
               unsigned int glMajor, unsigned int glMinor)
    : Widget(nullptr), mSDLWindow(nullptr), mNVGContext(nullptr),
      mCursor(Cursor::Arrow), mBackground(0.3f, 0.3f, 0.32f, 1.f), mCaption(caption),
      mShutdownSDLOnDestruct(false), mFullscreen(fullscreen) {

    /* Request a forward compatible OpenGL glMajor.glMinor core profile context.
       Default value is an OpenGL 3.3 core profile context. */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, glMajor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glMinor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, colorBits);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, colorBits);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, colorBits);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, alphaBits);

    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, stencilBits);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depthBits);

    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

    if (nSamples > 0) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, nSamples);
    }

    int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI;
    if (resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }

    mSDLWindow = SDL_CreateWindow(caption.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size.x, size.y, flags);
    mWindowID = SDL_GetWindowID(mSDLWindow);
    mGLContext = SDL_GL_CreateContext(mSDLWindow);

    if (!mSDLWindow || !mGLContext)
        throw std::runtime_error("Could not create an OpenGL " +
                                 std::to_string(glMajor) + "." +
                                 std::to_string(glMinor) + " context!");

    SDL_GL_MakeCurrent(mSDLWindow, mGLContext);

#if defined(NANOGUI_GLAD)
    if (!gladInitialized) {
        gladInitialized = true;
        if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress))
            throw std::runtime_error("Could not initialize GLAD!");
        glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM
    }
#endif

    SDL_GL_GetDrawableSize(mSDLWindow, &mFBSize[0], &mFBSize[1]);
    glViewport(0, 0, mFBSize[0], mFBSize[1]);
    glClearColor(mBackground[0], mBackground[1], mBackground[2], mBackground[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    SDL_GL_SetSwapInterval(0);
    SDL_GL_SwapWindow(mSDLWindow);

#if defined(__APPLE__)
    /* Poll for events once before starting a potentially
       lengthy loading process. This is needed to be
       classified as "interactive" by other software such
       as iTerm2 */
    SDL_Event dummyEvent;
    SDL_PollEvent(&dummyEvent);
#endif


    initialize(mSDLWindow, true);
}

void Screen::initialize(SDL_Window *window, bool shutdownSDLOnDestruct) {
    mSDLWindow = window;
    mWindowID = SDL_GetWindowID(mSDLWindow);
    mShutdownSDLOnDestruct = shutdownSDLOnDestruct;
    SDL_GetWindowSize(mSDLWindow, &mSize[0], &mSize[1]);
    SDL_GL_GetDrawableSize(mSDLWindow, &mFBSize[0], &mFBSize[1]);

    mPixelRatio = get_pixel_ratio(window);

#if defined(_WIN32) || defined(__linux__)
    if (mPixelRatio != 1 && !mFullscreen)
        SDL_SetWindowSize(window, mSize.x * mPixelRatio, mSize.y * mPixelRatio);
#endif

#if defined(NANOGUI_GLAD)
    if (!gladInitialized) {
        gladInitialized = true;
        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
            throw std::runtime_error("Could not initialize GLAD!");
        glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM
    }
#endif

    /* Detect framebuffer properties and set up compatible NanoVG context */
    GLint nStencilBits = 0, nSamples = 0;
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER,
        GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &nStencilBits);
    glGetIntegerv(GL_SAMPLES, &nSamples);

    int flags = 0;
    if (nStencilBits >= 8)
       flags |= NVG_STENCIL_STROKES;
    if (nSamples <= 1)
       flags |= NVG_ANTIALIAS;
#if !defined(NDEBUG)
    flags |= NVG_DEBUG;
#endif

    mNVGContext = nvgCreateGL3(flags);
    if (mNVGContext == nullptr)
        throw std::runtime_error("Could not initialize NanoVG!");

    mVisible = true; // glfwGetWindowAttrib(window, GLFW_VISIBLE) != 0;
    setTheme(new Theme(mNVGContext));
    mMousePos = Vector2i();
    mMouseState = mModifiers = 0;
    mDragActive = false;
    mLastInteraction = getTime();
    mProcessEvents = true;
    __nanogui_screens.push_back(this);

    mCursors.insert(std::pair<Cursor, SDL_Cursor *>(Cursor::Arrow, SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW)));
    mCursors.insert(std::pair<Cursor, SDL_Cursor *>(Cursor::IBeam, SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM)));
    mCursors.insert(std::pair<Cursor, SDL_Cursor *>(Cursor::Crosshair, SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR)));
    mCursors.insert(std::pair<Cursor, SDL_Cursor *>(Cursor::Hand, SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND)));
    mCursors.insert(std::pair<Cursor, SDL_Cursor *>(Cursor::HResize, SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE)));
    mCursors.insert(std::pair<Cursor, SDL_Cursor *>(Cursor::VResize, SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS)));

    /// Fixes retina display-related font rendering issue (#185)
    nvgBeginFrame(mNVGContext, mSize[0], mSize[1], mPixelRatio);
    nvgEndFrame(mNVGContext);
}

Screen::~Screen() {
    /*
    for(auto it = __nanogui_screens.begin(); it != __nanogui_screens.end(); it++) {
        if(*it == this) {
            __nanogui_screens.erase(it);
            break;
        }
    }*/
    __nanogui_screens.erase(std::find(__nanogui_screens.begin(), __nanogui_screens.end(), this));
    for (auto it = mCursors.begin(); it != mCursors.end(); it++) {
        SDL_FreeCursor(it->second);
    }
    if (mNVGContext)
        nvgDeleteGL3(mNVGContext);
    if (mSDLWindow && mShutdownSDLOnDestruct)
        SDL_DestroyWindow(mSDLWindow);
}

void Screen::setVisible(bool visible) {
    if (mVisible != visible) {
        mVisible = visible;

        if (visible)
            SDL_ShowWindow(mSDLWindow);
        else
            SDL_HideWindow(mSDLWindow);
    }
}

void Screen::setCaption(const std::string &caption) {
    if (caption != mCaption) {
        SDL_SetWindowTitle(mSDLWindow, caption.c_str());
        mCaption = caption;
    }
}

void Screen::setSize(const Vector2i &size) {
    Widget::setSize(size);

#if defined(_WIN32) || defined(__linux__)
    SDL_SetWindowSize(mSDLWindow, size.x * mPixelRatio, size.y * mPixelRatio);
#else
    SDL_SetWindowSize(mSDLWindow, size.x, size.y);
#endif
}

void Screen::drawAll() {
    glClearColor(mBackground[0], mBackground[1], mBackground[2], mBackground[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawContents();
    drawWidgets();

    SDL_GL_SwapWindow(mSDLWindow);
}

void Screen::drawWidgets() {
    if (!mVisible)
        return;

    SDL_GL_MakeCurrent(mSDLWindow, mGLContext);
    SDL_GL_GetDrawableSize(mSDLWindow, &mFBSize[0], &mFBSize[1]);
    SDL_GetWindowSize(mSDLWindow, &mSize[0], &mSize[1]);

#if defined(_WIN32) || defined(__linux__)
    mSize = ((Vector2f)mSize / mPixelRatio);
    mFBSize = ((Vector2f)mSize * mPixelRatio);
#else
    /* Recompute pixel ratio on OSX */
    if (mSize[0])
        mPixelRatio = (float) mFBSize[0] / (float) mSize[0];
#endif

    glViewport(0, 0, mFBSize[0], mFBSize[1]);
    glBindSampler(0, 0);
    nvgBeginFrame(mNVGContext, mSize[0], mSize[1], mPixelRatio);

    draw(mNVGContext);

    double elapsed = getTime() - mLastInteraction;

    if (elapsed > 0.5f) {
        /* Draw tooltips */
        const Widget *widget = findWidget(mMousePos);
        if (widget && !widget->tooltip().empty()) {
            int tooltipWidth = 150;

            float bounds[4];
            nvgFontFace(mNVGContext, "sans");
            nvgFontSize(mNVGContext, 15.0f);
            nvgTextAlign(mNVGContext, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
            nvgTextLineHeight(mNVGContext, 1.1f);
            Vector2i pos = widget->absolutePosition() +
                           Vector2i(widget->width() / 2, widget->height() + 10);

            nvgTextBounds(mNVGContext, pos.x, pos.y,
                            widget->tooltip().c_str(), nullptr, bounds);
            int h = (bounds[2] - bounds[0]) / 2;
            if (h > tooltipWidth / 2) {
                nvgTextAlign(mNVGContext, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
                nvgTextBoxBounds(mNVGContext, pos.x, pos.y, tooltipWidth,
                                widget->tooltip().c_str(), nullptr, bounds);

                h = (bounds[2] - bounds[0]) / 2;
            }
            nvgGlobalAlpha(mNVGContext,
                           std::min(1.0, 2 * (elapsed - 0.5f)) * 0.8);

            nvgBeginPath(mNVGContext);
            nvgFillColor(mNVGContext, Color(0, 255));
            nvgRoundedRect(mNVGContext, bounds[0] - 4 - h, bounds[1] - 4,
                           (int) (bounds[2] - bounds[0]) + 8,
                           (int) (bounds[3] - bounds[1]) + 8, 3);

            int px = (int) ((bounds[2] + bounds[0]) / 2) - h;
            nvgMoveTo(mNVGContext, px, bounds[1] - 10);
            nvgLineTo(mNVGContext, px + 7, bounds[1] + 1);
            nvgLineTo(mNVGContext, px - 7, bounds[1] + 1);
            nvgFill(mNVGContext);

            nvgFillColor(mNVGContext, Color(255, 255));
            nvgFontBlur(mNVGContext, 0.0f);
            nvgTextBox(mNVGContext, pos.x - h, pos.y, tooltipWidth,
                       widget->tooltip().c_str(), nullptr);
        }
    }

    nvgEndFrame(mNVGContext);
}

bool Screen::keyboardEvent(int key, int scancode, int action, int modifiers) {
    if (mFocusPath.size() > 0) {
        for (auto it = mFocusPath.rbegin() + 1; it != mFocusPath.rend(); ++it)
            if ((*it)->focused() && (*it)->keyboardEvent(key, scancode, action, modifiers))
                return true;
    }

    return false;
}

bool Screen::keyboardCharacterEvent(unsigned int codepoint) {
    if (mFocusPath.size() > 0) {
        for (auto it = mFocusPath.rbegin() + 1; it != mFocusPath.rend(); ++it)
            if ((*it)->focused() && (*it)->keyboardCharacterEvent(codepoint))
                return true;
    }
    return false;
}

bool Screen::resizeEvent(const Vector2i& size) {
    if (mResizeCallback) {
        mResizeCallback(size);
        return true;
    }
    return false;
}

bool Screen::cursorPosCallbackEvent(double x, double y) {
    Vector2i p((int) x, (int) y);

#if defined(_WIN32) || defined(__linux__)
    p = ((Vector2f)p / mPixelRatio);
#endif

    bool ret = false;
    mLastInteraction = getTime();
    try {
        p -= Vector2i(1, 2);

        if (!mDragActive) {
            Widget *widget = findWidget(p);
            if (widget != nullptr && widget->cursor() != mCursor) {
                mCursor = widget->cursor();
                SDL_SetCursor(mCursors[mCursor]);
            }
        } else {
            ret = mDragWidget->mouseDragEvent(
                p - mDragWidget->parent()->absolutePosition(), p - mMousePos,
                mMouseState, mModifiers);
        }

        if (!ret)
            ret = mouseMotionEvent(p, p - mMousePos, mMouseState, mModifiers);

        mMousePos = p;

        return ret;
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        return false;
    }
}

bool Screen::mouseButtonCallbackEvent(int button, int action, int modifiers) {
    mModifiers = modifiers;
    mLastInteraction = getTime();
    try {
        if (mFocusPath.size() > 1) {
            const Window *window =
                dynamic_cast<Window *>(mFocusPath[mFocusPath.size() - 2]);
            if (window && window->modal()) {
                if (!window->contains(mMousePos))
                    return false;
            }
        }

        if (action == SDL_PRESSED)
            mMouseState |= 1 << button;
        else
            mMouseState &= ~(1 << button);

        auto dropWidget = findWidget(mMousePos);
        if (mDragActive && action == SDL_RELEASED &&
            dropWidget != mDragWidget)
            mDragWidget->mouseButtonEvent(
                mMousePos - mDragWidget->parent()->absolutePosition(), button,
                false, mModifiers);

        if (dropWidget != nullptr && dropWidget->cursor() != mCursor) {
            mCursor = dropWidget->cursor();
            SDL_SetCursor(mCursors[mCursor]);
        }

        if (action == SDL_PRESSED && (button == SDL_BUTTON_LEFT || button == SDL_BUTTON_RIGHT)) {
            mDragWidget = findWidget(mMousePos);
            if (mDragWidget == this)
                mDragWidget = nullptr;
            mDragActive = mDragWidget != nullptr;
            if (!mDragActive)
                updateFocus(nullptr);
        } else {
            mDragActive = false;
            mDragWidget = nullptr;
        }

        return mouseButtonEvent(mMousePos, button, action == SDL_PRESSED,
                                mModifiers);
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        return false;
    }
}

bool Screen::keyCallbackEvent(int key, int scancode, int action, int mods) {
    mLastInteraction = getTime();
    try {
        return keyboardEvent(key, scancode, action, mods);
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
        return false;
    }
}

bool Screen::charCallbackEvent(unsigned int codepoint) {
    mLastInteraction = getTime();
    try {
        return keyboardCharacterEvent(codepoint);
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what()
                  << std::endl;
        return false;
    }
}

bool Screen::dropCallbackEvent(int count, const char **filenames) {
    std::vector<std::string> arg(count);
    for (int i = 0; i < count; ++i)
        arg[i] = filenames[i];
    return dropEvent(arg);
}

bool Screen::scrollCallbackEvent(double x, double y) {
    mLastInteraction = getTime();
    try {
        if (mFocusPath.size() > 1) {
            const Window *window =
                dynamic_cast<Window *>(mFocusPath[mFocusPath.size() - 2]);
            if (window && window->modal()) {
                if (!window->contains(mMousePos))
                    return false;
            }
        }
        return scrollEvent(mMousePos, Vector2f(x, y));
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what()
                  << std::endl;
        return false;
    }
}

bool Screen::resizeCallbackEvent(int, int) {
    Vector2i fbSize, size;
    SDL_GL_GetDrawableSize(mSDLWindow, &fbSize[0], &fbSize[1]);
    SDL_GetWindowSize(mSDLWindow, &size[0], &size[1]);

#if defined(_WIN32) || defined(__linux__)
    size = ((Vector2f)size / mPixelRatio);
#endif

    if (mFBSize == Vector2i(0, 0) || size == Vector2i(0, 0))
        return false;

    mFBSize = fbSize; mSize = size;
    mLastInteraction = getTime();

    try {
        return resizeEvent(mSize);
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in event handler: " << e.what()
                  << std::endl;
        return false;
    }
}

void Screen::updateFocus(Widget *widget) {
    for (auto w: mFocusPath) {
        if (!w->focused())
            continue;
        w->focusEvent(false);
    }
    mFocusPath.clear();
    Widget *window = nullptr;
    while (widget) {
        mFocusPath.push_back(widget);
        if (dynamic_cast<Window *>(widget))
            window = widget;
        widget = widget->parent();
    }
    for (auto it = mFocusPath.rbegin(); it != mFocusPath.rend(); ++it)
        (*it)->focusEvent(true);

    if (window)
        moveWindowToFront((Window *) window);
}

void Screen::disposeWindow(Window *window) {
    if (std::find(mFocusPath.begin(), mFocusPath.end(), window) != mFocusPath.end())
        mFocusPath.clear();
    if (mDragWidget == window)
        mDragWidget = nullptr;
    removeChild(window);
}

void Screen::centerWindow(Window *window) {
    if (window->size() == Vector2i()) {
        window->setSize(window->preferredSize(mNVGContext));
        window->performLayout(mNVGContext);
    }
    window->setPosition((mSize - window->size()) / 2);
}

void Screen::moveWindowToFront(Window *window) {
    mChildren.erase(std::remove(mChildren.begin(), mChildren.end(), window), mChildren.end());
    mChildren.push_back(window);
    /* Brute force topological sort (no problem for a few windows..) */
    bool changed = false;
    do {
        size_t baseIndex = 0;
        for (size_t index = 0; index < mChildren.size(); ++index)
            if (mChildren[index] == window)
                baseIndex = index;
        changed = false;
        for (size_t index = 0; index < mChildren.size(); ++index) {
            Popup *pw = dynamic_cast<Popup *>(mChildren[index]);
            if (pw && pw->parentWindow() == window && index < baseIndex) {
                moveWindowToFront(pw);
                changed = true;
                break;
            }
        }
    } while (changed);
}

bool Screen::handleSDLEvent(SDL_Event &event) {
    switch(event.type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if(event.key.windowID != mWindowID)
                return false;
            return keyCallbackEvent(event.key.keysym.sym, event.key.keysym.scancode, event.key.state, event.key.keysym.mod);
            break;

        case SDL_TEXTINPUT:
            if(event.text.windowID != mWindowID)
                return false;
            // TODO: handle UTF-8 input
            return charCallbackEvent(event.text.text[0]);
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            if(event.button.windowID != mWindowID)
                return false;
            return mouseButtonCallbackEvent(event.button.button, event.button.state, 0);
            break;

        case SDL_MOUSEWHEEL:
            if(event.wheel.windowID != mWindowID)
                return false;
            return scrollCallbackEvent(event.wheel.x, event.wheel.y);
            break;

        case SDL_MOUSEMOTION:
            if(event.motion.windowID != mWindowID)
                return false;
            return cursorPosCallbackEvent(event.motion.x, event.motion.y);
            break;

        case SDL_WINDOWEVENT:
            if(event.window.windowID != mWindowID)
                return false;
            switch(event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    return resizeCallbackEvent(event.window.data1, event.window.data2);
                    break;

                case SDL_WINDOWEVENT_FOCUS_GAINED:
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    return focusEvent(event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED);
                    break;
            }
            break;
    }
    return false;
}

NAMESPACE_END(nanogui)
