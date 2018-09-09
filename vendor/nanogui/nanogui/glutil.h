/*
    nanogui/glutil.h -- Convenience classes for accessing OpenGL >= 3.x

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <nanogui/opengl.h>
#include <map>
#include <glm/gtc/type_ptr.hpp>
#include <string>

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace half_float { class half; }
#endif

#if !defined(GL_HALF_FLOAT) || defined(DOXYGEN_DOCUMENTATION_BUILD)
    /// Ensures that ``GL_HALF_FLOAT`` is defined properly for all platforms.
    #define GL_HALF_FLOAT 0x140B
#endif

NAMESPACE_BEGIN(nanogui)

class GLUniformBuffer;

//  ----------------------------------------------------

/**
 * \class GLShader glutil.h nanogui/glutil.h
 *
 * Helper class for compiling and linking OpenGL shaders and uploading
 * associated vertex and index buffers from Eigen matrices.
 */
class NANOGUI_EXPORT GLShader {
public:
    /**
     * \struct Buffer glutil.h nanogui/glutil.h
     *
     * A wrapper struct for maintaining various aspects of items being managed
     * by OpenGL.  Buffers are created when \ref GLShader::uploadAttrib is
     * called.
     */
    struct Buffer {
        GLuint id;      ///< The identifier used with OpenGL.
        GLuint glType;  ///< The OpenGL type of this buffer.
        GLuint dim;     ///< The dimension of this buffer (typically the row width).
        GLuint compSize;///< The size (in bytes) of an individual element in this buffer.
        GLuint size;    ///< The total number of elements represented by this buffer.
        int version;    ///< The current version if this buffer.
    };

    /// Create an unitialized OpenGL shader
    GLShader()
        : mVertexShader(0), mFragmentShader(0), mGeometryShader(0),
          mProgramShader(0), mVertexArrayObject(0) { }

    /**
     * \brief Initialize the shader using the specified source strings.
     *
     * \param name
     *     The name this shader will be registered as.
     *
     * \param vertex_str
     *     The source of the vertex shader as a string.
     *
     * \param fragment_str
     *     The source of the fragment shader as a string.
     *
     * \param geometry_str
     *     The source of the geometry shader as a string.  The default value is
     *     the empty string, which indicates no geometry shader will be used.
     */
    bool init(const std::string &name, const std::string &vertex_str,
              const std::string &fragment_str,
              const std::string &geometry_str = "");

    /**
     * \brief Initialize the shader using the specified files on disk.
     *
     * \param name
     *     The name this shader will be registered as.
     *
     * \param vertex_fname
     *     The path to the file containing the source of the fragment shader.
     *
     * \param fragment_fname
     *     The path to the file containing the source of the vertex shader.
     *
     * \param geometry_fname
     *     The path to the file containing the source of the geometry shader.
     *     The default value is the empty string, which indicates no geometry
     *     shader will be used.
     */
    bool initFromFiles(const std::string &name,
                       const std::string &vertex_fname,
                       const std::string &fragment_fname,
                       const std::string &geometry_fname = "");

    /// Return the name of the shader
    const std::string &name() const { return mName; }

    /**
     * Set a preprocessor definition.  Custom preprocessor definitions must be
     * added **before** initializing the shader (e.g., via \ref initFromFiles).
     * See also: \ref mDefinitions.
     */
    void define(const std::string &key, const std::string &value) { mDefinitions[key] = value; }

    /**
     * Select this shader for subsequent draw calls.  Simply executes ``glUseProgram``
     * with \ref mProgramShader, and ``glBindVertexArray`` with \ref mVertexArrayObject.
     */
    void bind();

    /// Release underlying OpenGL objects
    void free();

    /// Return the handle of a named shader attribute (-1 if it does not exist)
    GLint attrib(const std::string &name, bool warn = true) const;

    /// Return the handle of a uniform attribute (-1 if it does not exist)
    GLint uniform(const std::string &name, bool warn = true) const;

    /// Upload an index buffer
    template <typename Matrix> void uploadIndices(const Matrix &M, int version = -1) {
        uploadAttrib("indices", M, version);
    }

    /// Invalidate the version numbers associated with attribute data
    void invalidateAttribs();

    /// Completely free an existing attribute buffer
    void freeAttrib(const std::string &name);

    /// Check if an attribute was registered a given name
    bool hasAttrib(const std::string &name) const {
        auto it = mBufferObjects.find(name);
        if (it == mBufferObjects.end())
            return false;
        return true;
    }

    /// Create a symbolic link to an attribute of another GLShader. This avoids duplicating unnecessary data
    void shareAttrib(const GLShader &otherShader, const std::string &name, const std::string &as = "");

    /// Return the version number of a given attribute
    int attribVersion(const std::string &name) const {
        auto it = mBufferObjects.find(name);
        if (it == mBufferObjects.end())
            return -1;
        return it->second.version;
    }

    /// Reset the version number of a given attribute
    void resetAttribVersion(const std::string &name) {
        auto it = mBufferObjects.find(name);
        if (it != mBufferObjects.end())
            it->second.version = -1;
    }

    /// Draw a sequence of primitives
    void drawArray(int type, uint32_t offset, uint32_t count);

    /// Draw a sequence of primitives using a previously uploaded index buffer
    void drawIndexed(int type, uint32_t offset, uint32_t count);

    /// Initialize a uniform parameter with a 4x4 matrix (float)
    void setUniform(const std::string &name, const glm::mat4 &mat, bool warn = true) {
        glUniformMatrix4fv(uniform(name, warn), 1, GL_FALSE, glm::value_ptr(mat));
    }

    /// Initialize a uniform parameter with a 3x3 matrix (float)
    void setUniform(const std::string &name, const glm::mat3 &mat, bool warn = true) {
        glUniformMatrix3fv(uniform(name, warn), 1, GL_FALSE, glm::value_ptr(mat));
    }

    /// Initialize a uniform parameter with a 4D vector (float)
    void setUniform(const std::string &name, const glm::vec4 &vec, bool warn = true) {
        glUniform4fv(uniform(name, warn), 1, glm::value_ptr(vec));
    }

    /// Initialize a uniform parameter with a 3D vector (float)
    void setUniform(const std::string &name, const glm::vec3 &vec, bool warn = true) {
        glUniform3fv(uniform(name, warn), 1, glm::value_ptr(vec));
    }

    /// Initialize a uniform parameter with a boolean value
    void setUniform(const std::string &name, bool value, bool warn = true) {
        glUniform1i(uniform(name, warn), (int)value);
    }

    /// Initialize a uniform parameter with an integer value
    void setUniform(const std::string &name, int value, bool warn = true) {
        glUniform1i(uniform(name, warn), value);
    }

    /// Initialize a uniform parameter with a floating point value
    void setUniform(const std::string &name, float value, bool warn = true) {
        glUniform1f(uniform(name, warn), value);
    }

    /// Initialize a uniform buffer with a uniform buffer object
    void setUniform(const std::string &name, const GLUniformBuffer &buf, bool warn = true);

    /// Return the size of all registered buffers in bytes
    size_t bufferSize() const {
        size_t size = 0;
        for (auto const &buf : mBufferObjects)
            size += buf.second.size;
        return size;
    }

    /**
     * \brief (Advanced) Returns a reference to the specified \ref GLShader::Buffer.
     *
     * \rst
     * .. danger::
     *
     *    Extreme caution must be exercised when using this method.  The user is
     *    discouraged from explicitly storing the reference returned, as it can
     *    change, become deprecated, or no longer reside in
     *    :member:`mBufferObjects <nanogui::GLShader::mBufferObjects>`.
     *
     *    There are generally very few use cases that justify using this method
     *    directly.  For example, if you need the version of a buffer, call
     *    :func:`attribVersion <nanogui::GLShader::attribVersion>`.  If you want
     *    to share data between :class:`GLShader <nanogui::GLShader>` objects,
     *    call :func:`shareAttrib <nanogui::GLShader::shareAttrib>`.
     *
     *    One example use case for this method is sharing data between different
     *    GPU pipelines such as CUDA or OpenCL.  When sharing data, you
     *    typically need to map pointers between the API's.  The returned
     *    buffer's :member:`Buffer::id <nanogui::GLShader::Buffer::id>` is the
     *    ``GLuint`` you will want to map to the other API.
     *
     *    In short, only use this method if you absolutely need to.
     * \endrst
     *
     * \param name
     *     The name of the desired attribute.
     *
     * \return
     *     A reference to the current buffer associated with ``name``.  Should
     *     not be explicitly stored.
     *
     * \throws std::runtime_error
     *     If ``name`` is not found.
     */
    const Buffer &attribBuffer(const std::string &name);

public:
    /* Low-level API */
    void uploadAttrib(const std::string &name, size_t size, int dim,
                       uint32_t compSize, GLuint glType, bool integral,
                       const void *data, int version = -1);
    void downloadAttrib(const std::string &name, size_t size, int dim,
                       uint32_t compSize, GLuint glType, void *data);

protected:
    /// The registered name of this GLShader.
    std::string mName;

    /// The vertex shader of this GLShader (as returned by ``glCreateShader``).
    GLuint mVertexShader;

    /// The fragment shader of this GLShader (as returned by ``glCreateShader``).
    GLuint mFragmentShader;

    /// The geometry shader (if requested) of this GLShader (as returned by ``glCreateShader``).
    GLuint mGeometryShader;

    /// The OpenGL program (as returned by ``glCreateProgram``).
    GLuint mProgramShader;

    /// The vertex array associated with this GLShader (as returned by ``glGenVertexArrays``).
    GLuint mVertexArrayObject;

    /**
     * The map of string names to buffer objects representing the various
     * attributes that have been uploaded using \ref uploadAttrib.
     */
    std::map<std::string, Buffer> mBufferObjects;

    /**
     * \rst
     * The map of preprocessor names to values (if any have been created).  If
     * a definition was added seeking to create ``#define WIDTH 256``, the key
     * would be ``"WIDTH"`` and the value would be ``"256"``.  These definitions
     * will be included automatically in the string that gets compiled for the
     * vertex, geometry, and fragment shader code.
     * \endrst
     */
    std::map<std::string, std::string> mDefinitions;
};

//  ----------------------------------------------------

/**
 * \class GLUniformBuffer glutil.h nanogui/glutil.h
 *
 * \brief Helper class for creating OpenGL Uniform Buffer objects.
 */
class NANOGUI_EXPORT GLUniformBuffer {
public:
    /// Default constructor: unusable until you call the ``init()`` method
    GLUniformBuffer() : mID(0), mBindingPoint(0) { }

    /// Create a new uniform buffer
    void init();

    /// Release underlying OpenGL object
    void free();

    /// Bind the uniform buffer to a specific binding point
    void bind(int index);

    /// Release/unbind the uniform buffer
    void release();

    /// Update content on the GPU using data
    void update(const std::vector<uint8_t> &data);

    /// Return the binding point of this uniform buffer
    int getBindingPoint() const { return mBindingPoint; }
private:
    GLuint mID;
    int mBindingPoint;
};

//  ----------------------------------------------------

/**
 * \class GLFramebuffer glutil.h nanogui/glutil.h
 *
 * \brief Helper class for creating framebuffer objects.
 */
class NANOGUI_EXPORT GLFramebuffer {
public:
    /// Default constructor: unusable until you call the ``init()`` method
    GLFramebuffer() : mFramebuffer(0), mDepth(0), mColor(0), mSamples(0) { }

    /// Create a new framebuffer with the specified size and number of MSAA samples
    void init(const Vector2i &size, int nSamples);

    /// Release all associated resources
    void free();

    /// Bind the framebuffer object
    void bind();

    /// Release/unbind the framebuffer object
    void release();

    /// Blit the framebuffer object onto the screen
    void blit();

    /// Return whether or not the framebuffer object has been initialized
    bool ready() { return mFramebuffer != 0; }

    /// Return the number of MSAA samples
    int samples() const { return mSamples; }

    /// Quick and dirty method to write a TGA (32bpp RGBA) file of the framebuffer contents for debugging
    void downloadTGA(const std::string &filename);
protected:
    GLuint mFramebuffer, mDepth, mColor;
    Vector2i mSize;
    int mSamples;
};

NAMESPACE_END(nanogui)
