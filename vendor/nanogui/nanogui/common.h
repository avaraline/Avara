/*
    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/**
 * \file nanogui/common.h
 *
 * \brief Common definitions used by NanoGUI.
 */

#pragma once

#if defined(_WIN32)
#  if defined(NANOGUI_BUILD)
     /* Quench a few warnings on when compiling NanoGUI on Windows */
#    pragma warning(disable : 4127) // warning C4127: conditional expression is constant
#    pragma warning(disable : 4244) // warning C4244: conversion from X to Y, possible loss of data
#  endif
#  pragma warning(disable : 4251) // warning C4251: class X needs to have dll-interface to be used by clients of class Y
#  pragma warning(disable : 4714) // warning C4714: function X marked as __forceinline not inlined
#  pragma warning(disable : 4127) // warning C4127: conditional expression is constant
#endif

#include <stdint.h>
#include <array>
#include <vector>
#include <SDL2/SDL.h>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4

/* Set to 1 to draw boxes around widgets */
//#define NANOGUI_SHOW_WIDGET_BOUNDS 1

#if !defined(NAMESPACE_BEGIN) || defined(DOXYGEN_DOCUMENTATION_BUILD)
    /**
     * \brief Convenience macro for namespace declarations
     *
     * The macro ``NAMESPACE_BEGIN(nanogui)`` will expand to ``namespace
     * nanogui {``. This is done to hide the namespace scope from editors and
     * C++ code formatting tools that may otherwise indent the entire file.
     * The corresponding ``NAMESPACE_END`` macro also lists the namespace
     * name for improved readability.
     *
     * \param name
     *     The name of the namespace scope to open
     */
    #define NAMESPACE_BEGIN(name) namespace name {
#endif
#if !defined(NAMESPACE_END) || defined(DOXYGEN_DOCUMENTATION_BUILD)
    /**
     * \brief Convenience macro for namespace declarations
     *
     * Closes a namespace (counterpart to ``NAMESPACE_BEGIN``)
     * ``NAMESPACE_END(nanogui)`` will expand to only ``}``.
     *
     * \param name
     *     The name of the namespace scope to close
     */
    #define NAMESPACE_END(name) }
#endif

#if defined(NANOGUI_SHARED)
#  if defined(_WIN32)
#    if defined(NANOGUI_BUILD)
#      define NANOGUI_EXPORT __declspec(dllexport)
#    else
#      define NANOGUI_EXPORT __declspec(dllimport)
#    endif
#  elif defined(NANOGUI_BUILD)
#    define NANOGUI_EXPORT __attribute__ ((visibility("default")))
#  else
#    define NANOGUI_EXPORT
#  endif
#else
     /**
      * If the build flag ``NANOGUI_SHARED`` is defined, this directive will expand
      * to be the platform specific shared library import / export command depending
      * on the compilation stage.  If undefined, it expands to nothing. **Do not**
      * define this directive on your own.
      */
#    define NANOGUI_EXPORT
#endif

/* Force usage of discrete GPU on laptops (macro must be invoked in main application) */
#if defined(_WIN32) && !defined(DOXYGEN_DOCUMENTATION_BUILD)
#define NANOGUI_FORCE_DISCRETE_GPU() \
    extern "C" { \
        __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1; \
        __declspec(dllexport) int NvOptimusEnablement = 1; \
    }
#else
/**
 * On Windows, exports ``AmdPowerXpressRequestHighPerformance`` and
 * ``NvOptimusEnablement`` as ``1``.
 */
#define NANOGUI_FORCE_DISCRETE_GPU()
#endif

// These will produce broken links in the docs build
#if !defined(DOXYGEN_SHOULD_SKIP_THIS)

struct NVGcontext { /* Opaque handle type, never de-referenced within NanoGUI */ };
//struct GLFWwindow { /* Opaque handle type, never de-referenced within NanoGUI */ };

struct NVGcolor;
struct NVGglyphPosition;
//struct GLFWcursor;

#endif // DOXYGEN_SHOULD_SKIP_THIS

// Define command key for windows/mac/linux
#if defined(__APPLE__) || defined(DOXYGEN_DOCUMENTATION_BUILD)
    /// If on OSX, maps to ``KMOD_GUI``.  Otherwise, maps to ``KMOD_CTRL``.
    #define SYSTEM_COMMAND_MOD KMOD_GUI
#else
    #define SYSTEM_COMMAND_MOD KMOD_CTRL
#endif

NAMESPACE_BEGIN(nanogui)

/// Cursor shapes available to use in GLFW.  Shape of actual cursor determined by Operating System.
enum class Cursor {
    Arrow = (int)SDL_SYSTEM_CURSOR_ARROW,  ///< The arrow cursor.
    IBeam = (int)SDL_SYSTEM_CURSOR_IBEAM,      ///< The I-beam cursor.
    Crosshair = (int)SDL_SYSTEM_CURSOR_CROSSHAIR,  ///< The crosshair cursor.
    Hand = (int)SDL_SYSTEM_CURSOR_HAND,       ///< The hand cursor.
    HResize = (int)SDL_SYSTEM_CURSOR_SIZEWE,    ///< The horizontal resize cursor.
    VResize = (int)SDL_SYSTEM_CURSOR_SIZENS    ///< The vertical resize cursor.
    //CursorCount = 6 ///< Not a cursor --- should always be last: enables a loop over the cursor types.
};

/* Import some common Eigen types */
/// Type alias to allow ``Eigen::Vector2f`` to be used as ``nanogui::Vector2f``.
using Vector2f = glm::vec2;
/// Type alias to allow ``Eigen::Vector3f`` to be used as ``nanogui::Vector3f``.
using Vector3f = glm::vec3;
/// Type alias to allow ``Eigen::Vector4f`` to be used as ``nanogui::Vector4f``.
using Vector4f = glm::vec4;
/// Type alias to allow ``Eigen::Vector2i`` to be used as ``nanogui::Vector2i``.
using Vector2i = glm::ivec2;
/// Type alias to allow ``Eigen::Vector3i`` to be used as ``nanogui::Vector3i``.
using Vector3i = glm::ivec3;
/// Type alias to allow ``Eigen::Vector4i`` to be used as ``nanogui::Vector4i``.
using Vector4i = glm::ivec4;
/// Type alias to allow ``Eigen::Matrix3f`` to be used as ``nanogui::Matrix3f``.
using Matrix3f = glm::mat3;
/// Type alias to allow ``Eigen::Matrix4f`` to be used as ``nanogui::Matrix4f``.
using Matrix4f = glm::mat4;
/// Type alias to allow ``Eigen::VectorXf`` to be used as ``nanogui::VectorXf``.
//using VectorXf = Eigen::VectorXf;
/// Type alias to allow ``Eigen::MatrixXf`` to be used as ``nanogui::MatrixXf``.
//using MatrixXf = Eigen::MatrixXf;

/**
 * Convenience typedef for things like index buffers.  You would use it the same
 * as ``Eigen::MatrixXf``, only it is storing ``uint32_t`` instead of ``float``.
 */
//using MatrixXu = Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic>;

/**
 * \class Color common.h nanogui/common.h
 *
 * \brief Stores an RGBA floating point color value.
 *
 * This class simply wraps around an ``Eigen::Vector4f``, providing some convenient
 * methods and terminology for thinking of it as a color.  The data operates in the
 * same way as ``Eigen::Vector4f``, and the following values are identical:
 *
 * \rst
 * +---------+-------------+-----------------------+-------------+
 * | Channel | Array Index | Eigen::Vector4f Value | Color Value |
 * +=========+=============+=======================+=============+
 * | Red     | ``0``       | x()                   | r()         |
 * +---------+-------------+-----------------------+-------------+
 * | Green   | ``1``       | y()                   | g()         |
 * +---------+-------------+-----------------------+-------------+
 * | Blue    | ``2``       | z()                   | b()         |
 * +---------+-------------+-----------------------+-------------+
 * | Alpha   | ``3``       | w()                   | w()         |
 * +---------+-------------+-----------------------+-------------+
 *
 * .. note::
 *    The method for the alpha component is **always** ``w()``.
 * \endrst
 *
 * You can and should still use the various convenience methods such as ``any()``,
 * ``all()``, ``head<index>()``, etc provided by Eigen.
 */
struct Color : public glm::vec4 {
    /// Default constructor: represents black (``r, g, b, a = 0``)
    Color() : Color(0, 0, 0, 0) {}

    /**
     * Makes an exact copy of the data represented by the input parameter.
     *
     * \param color
     * The four dimensional float vector being copied.
     */
    Color(const glm::vec4 &color) : glm::vec4(color) { }

    /**
     * Copies (x, y, z) from the input vector, and uses the value specified by
     * the ``alpha`` parameter for this Color object's alpha component.
     *
     * \param color
     * The three dimensional float vector being copied.
     *
     * \param alpha
     * The value to set this object's alpha component to.
     */
    Color(const glm::vec3 &color, float alpha)
        : Color(color.r, color.g, color.b, alpha) { }

    /**
     * Copies (x, y, z) from the input vector, casted as floats first and then
     * divided by ``255.0``, and uses the value specified by the ``alpha``
     * parameter, casted to a float and divided by ``255.0`` as well, for this
     * Color object's alpha component.
     *
     * \param color
     * The three dimensional integer vector being copied, will be divided by ``255.0``.
     *
     * \param alpha
     * The value to set this object's alpha component to, will be divided by ``255.0``.
     */
    Color(const glm::ivec3 &color, int alpha)
        : Color((float)color.r / 255.f, (float)color.g / 255.f, (float)color.b / 255.f, alpha / 255.f) { }

    /**
     * Copies (x, y, z) from the input vector, and sets the alpha of this color
     * to be ``1.0``.
     *
     * \param color
     * The three dimensional float vector being copied.
     */
    Color(const glm::vec3 &color) : Color(color, 1.0f) {}

    /**
     * Copies (x, y, z) from the input vector, casting to floats and dividing by
     * ``255.0``.  The alpha of this color will be set to ``1.0``.
     *
     * \param color
     * The three dimensional integer vector being copied, will be divided by ``255.0``.
     */
    Color(const glm::ivec3 &color)
        : Color(color.r, color.g, color.b, 255) { }

    /**
     * Copies (x, y, z, w) from the input vector, casting to floats and dividing
     * by ``255.0``.
     *
     * \param color
     * The three dimensional integer vector being copied, will be divided by ``255.0``.
     */
    Color(const glm::ivec4 &color)
        : Color(color.r, color.g, color.b, color.a) { }

    /**
     * Creates the Color ``(intensity, intensity, intensity, alpha)``.
     *
     * \param intensity
     * The value to be used for red, green, and blue.
     *
     * \param alpha
     * The alpha component of the color.
     */
    Color(float intensity, float alpha)
        : Color(intensity, intensity, intensity, alpha) { }

    /**
     * Creates the Color ``(intensity, intensity, intensity, alpha) / 255.0``.
     * Values are casted to floats before division.
     *
     * \param intensity
     * The value to be used for red, green, and blue, will be divided by ``255.0``.
     *
     * \param alpha
     * The alpha component of the color, will be divided by ``255.0``.
     */
    Color(int intensity, int alpha)
        : Color(intensity, intensity, intensity, alpha) { }

    /**
     * Explicit constructor: creates the Color ``(r, g, b, a)``.
     *
     * \param r
     * The red component of the color.
     *
     * \param g
     * The green component of the color.
     *
     * \param b
     * The blue component of the color.
     *
     * \param a
     * The alpha component of the color.
     */
    Color(float r, float g, float b, float a) : Color(glm::vec4(r, g, b, a)) { }

    /**
     * Explicit constructor: creates the Color ``(r, g, b, a) / 255.0``.
     * Values are casted to floats before division.
     *
     * \param r
     * The red component of the color, will be divided by ``255.0``.
     *
     * \param g
     * The green component of the color, will be divided by ``255.0``.
     *
     * \param b
     * The blue component of the color, will be divided by ``255.0``.
     *
     * \param a
     * The alpha component of the color, will be divided by ``255.0``.
     */
    Color(int r, int g, int b, int a)
        : Color((float)r / 255.f, (float)g / 255.f, (float)b / 255.f, (float)a / 255.f) { }

    /// Allows for conversion between this Color and NanoVG's representation.
    inline operator const NVGcolor &() const;
};

// skip the forward declarations for the docs
#ifndef DOXYGEN_SHOULD_SKIP_THIS

/* Forward declarations */
template <typename T> class ref;
class AdvancedGridLayout;
class BoxLayout;
class Button;
class CheckBox;
class ComboBox;
class GLFramebuffer;
class GLShader;
class GridLayout;
class GroupLayout;
class Label;
class Layout;
class MessageDialog;
class Object;
class Popup;
class PopupButton;
class ProgressBar;
class Screen;
class Slider;
class StackedWidget;
class TabHeader;
class TabWidget;
class TextBox;
class GLCanvas;
class Theme;
class ToolButton;
class VScrollPanel;
class Widget;
class Window;

#endif // DOXYGEN_SHOULD_SKIP_THIS

/**
 * Static initialization; should be called once before invoking **any** NanoGUI
 * functions **if** you are having NanoGUI manage OpenGL / GLFW.  This method
 * is effectively a wrapper call to ``glfwInit()``, so if you are managing
 * OpenGL / GLFW on your own *do not call this method*.
 *
 * \rst
 * Refer to :ref:`nanogui_example_3` for how you might go about managing OpenGL
 * and GLFW on your own, while still using NanoGUI's classes.
 * \endrst
 */
extern NANOGUI_EXPORT void init();

/// Static shutdown; should be called before the application terminates.
extern NANOGUI_EXPORT void shutdown();

/**
 * \brief Enter the application main loop
 *
 * \param refresh
 *     NanoGUI issues a redraw call whenever an keyboard/mouse/.. event is
 *     received. In the absence of any external events, it enforces a redraw
 *     once every ``refresh`` milliseconds. To disable the refresh timer,
 *     specify a negative value here.
 *
 * \param detach
 *     This parameter only exists in the Python bindings. When the active
 *     \c Screen instance is provided via the \c detach parameter, the
 *     ``mainloop()`` function becomes non-blocking and returns
 *     immediately (in this case, the main loop runs in parallel on a newly
 *     created thread). This feature is convenient for prototyping user
 *     interfaces on an interactive Python command prompt. When
 *     ``detach != None``, the function returns an opaque handle that
 *     will release any resources allocated by the created thread when the
 *     handle's ``join()`` method is invoked (or when it is garbage
 *     collected).
 *
 * \remark
 *     Unfortunately, Mac OS X strictly requires all event processing to take
 *     place on the application's main thread, which is fundamentally
 *     incompatible with this type of approach. Thus, NanoGUI relies on a
 *     rather crazy workaround on Mac OS (kudos to Dmitriy Morozov):
 *     ``mainloop()`` launches a new thread as before but then uses
 *     libcoro to swap the thread execution environment (stack, registers, ..)
 *     with the main thread. This means that the main application thread is
 *     hijacked and processes events in the main loop to satisfy the
 *     requirements on Mac OS, while the thread that actually returns from this
 *     function is the newly created one (paradoxical, as that may seem).
 *     Deleting or ``join()``ing the returned handle causes application to
 *     wait for the termination of the main loop and then swap the two thread
 *     environments back into their initial configuration.
 */
extern NANOGUI_EXPORT void mainloop(int refresh = 50);

/// Request the application main loop to terminate (e.g. if you detached mainloop).
extern NANOGUI_EXPORT void leave();

/// Return whether or not a main loop is currently active
extern NANOGUI_EXPORT bool active();

extern NANOGUI_EXPORT double getTime();
extern NANOGUI_EXPORT void getTime(double t);

/**
 * \brief Open a native file open/save dialog.
 *
 * \param filetypes
 *     Pairs of permissible formats with descriptions like
 *     ``("png", "Portable Network Graphics")``.
 *
 * \param save
 *     Set to ``true`` if you would like subsequent file dialogs to open
 *     at whatever folder they were in when they close this one.
 */
extern NANOGUI_EXPORT std::string
file_dialog(const std::vector<std::pair<std::string, std::string>> &filetypes,
            bool save);

/**
 * \brief Open a native file open dialog, which allows multiple selection.
 *
 * \param filetypes
 *     Pairs of permissible formats with descriptions like
 *     ``("png", "Portable Network Graphics")``.
 *
 * \param save
 *     Set to ``true`` if you would like subsequent file dialogs to open
 *     at whatever folder they were in when they close this one.
 *
 * \param multiple
 *     Set to ``true`` if you would like to be able to select multiple
 *     files at once. May not be simultaneously true with \p save.
 */
extern NANOGUI_EXPORT std::vector<std::string>
file_dialog(const std::vector<std::pair<std::string, std::string>> &filetypes,
            bool save, bool multiple);

#if defined(__APPLE__) || defined(DOXYGEN_DOCUMENTATION_BUILD)
/**
 * \brief Move to the application bundle's parent directory
 *
 * This is function is convenient when deploying .app bundles on OSX. It
 * adjusts the file path to the parent directory containing the bundle.
 */
extern NANOGUI_EXPORT void chdir_to_bundle_parent();
#endif

/**
 * \brief Convert a single UTF32 character code to UTF8.
 *
 * \rst
 * NanoGUI uses this to convert the icon character codes
 * defined in :ref:`file_nanogui_entypo.h`.
 * \endrst
 *
 * \param c
 *     The UTF32 character to be converted.
 */
extern NANOGUI_EXPORT std::array<char, 8> utf8(int c);

extern NANOGUI_EXPORT uint32_t utf8_decode(char *p, size_t len);

/// Convenience function for instanting a PNG icon from the application's data segment (via bin2c)
#define nvgImageIcon(ctx, name) nanogui::__nanogui_get_image(ctx, #name, name##_png, name##_png_size)

/// Helper function used by nvgImageIcon
extern NANOGUI_EXPORT int __nanogui_get_image(NVGcontext *ctx, const std::string &name, uint8_t *data, uint32_t size);

NAMESPACE_END(nanogui)
