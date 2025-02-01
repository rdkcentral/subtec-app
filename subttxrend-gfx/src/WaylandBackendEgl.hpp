/*****************************************************************************
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2021 Liberty Global Service B.V.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*****************************************************************************/


#ifndef SUBTTXREND_GFX_WAYLAND_BACKEND_EGL_HPP_
#define SUBTTXREND_GFX_WAYLAND_BACKEND_EGL_HPP_

#include "WaylandBackend.hpp"
#include "GLcpp.hpp"
#include "Types.hpp"

// TODO: fixme
class wl_egl_window;

namespace subttxrend
{
namespace gfx
{


class PixelArgb8888;

/**
 * Rendering backend using wayland-egl.
 */
class WaylandBackendEgl : public WaylandBackend
{
public:
    /**
     * Constructor.
     *
     * @param listener
     *      Listener for backend events.
     */
    WaylandBackendEgl(BackendListener* listener);

    /**
     * Destructor.
     */
    virtual ~WaylandBackendEgl();

protected:
    /** @copydoc WaylandBackend::initRendering */
    virtual bool initRendering() override;

    /** @copydoc WaylandBackend::checkRequiredInterfaces */
    virtual bool checkRequiredInterfaces() const override;

    /** @copydoc WaylandBackend::currentOutputModeChanged */
    virtual void currentOutputModeChanged(const Size& size) override;

    /** @copydoc WaylandBackend::surfaceResizeRequested */
    virtual void surfaceResizeRequested(const Size& size) override;

    /** @copydoc WaylandBackend::redraw */
    virtual void redraw(const waylandcpp::Surface1::Ptr& surface) override;

    /** @copydoc WaylandBackend::interfaceAdded */
    virtual void interfaceAdded(waylandcpp::Registry1::Ptr registry,
                                uint32_t name,
                                std::string interface,
                                uint32_t version) override;

    /** @copydoc WaylandBackend::interfaceRemoved */
    virtual void interfaceRemoved(waylandcpp::Registry1::Ptr registry,
                                  uint32_t name) override;

private:
    /**
     * Prepares texture of given size.
     *
     * @param contentSize
     *      Requested texture size.
     *
     * @return
     *      True on success, false on error.
     */
    bool prepareTexture(const Size& contentSize, int textureId);

    /**
     * Draws contents on texture.
     *
     * @param contentSize
     *      Content size.
     *
     * @return
     *      True on success, false on error.
     */
    bool drawOnTexture(const Size& contentSize);

    /**
     * Draws textured object on GL context.
     *
     * @param contentSize
     *      Content size.
     */
    void drawTexturedObject(const Size& contentSize);

    /**
     * Checks if OpenGL error occurred.
     *
     * Check for error and print error message if error occurred.
     * The errors are cleared after the call.
     *
     * @return
     *      True if error occurred, false otherwise.
     */
    bool checkForGlError();

    /**
     * Compiles the shader.
     *
     * @param shaderSrc
     *      Shader source string.
     * @param type
     *      Type of shader.
     *
     * @return
     *      Shader identifier, 0 on error.
     */
    glcpp::ShaderPtr compileShader(const GLchar* const shaderSrc,
                                   GLenum type);

    /**
     * Links the shader program.
     *
     * @return
     *      Program object, zero on error.
     */
    glcpp::ProgramPtr linkShaderProgram();

    /** EGL context object. */
    EGLContext m_eglContext;

    /** EGL display object. */
    EGLDisplay m_eglDisplay;

    /** Wayland GL window. */
    wl_egl_window* m_eglWindow;

    /** EGL surface object. */
    EGLSurface m_eglSurface;

    /** Initial window size. */
    Size m_initialSize;

    /** GL textures. */
    std::array<glcpp::TexturePtr,2> m_textures;

    /** Current textures sizes. */
    std::array<Size, 2> m_textureSizes;

    /** GL shader program. */
    glcpp::ProgramPtr m_program;

    /** GL program handle - Texture with text handle. */
    GLint m_textureUniformHandle;

    /** GL program handle - Texture with background handle. */
    GLint m_textureBgUniformHandle;

    /** GL program handle - Position. */
    GLint m_positionAttribHandle;

    /** GL program handle - Texture. */
    GLint m_textureCoordinateHandle;

    /** Size of the buffer for clearing the texture. */
    std::int32_t m_clearBufferSize;

    /** Pointer of the buffer for clearing the texture. */
    std::unique_ptr<PixelArgb8888[]> m_clearBuffer;

    /** Default size. */
    static const Size DEFAULT_SIZE;
};

} // namespace gfx
} // namespace subttxrend

#endif                          // SUBTTXREND_GFX_WAYLAND_BACKEND_EGL_HPP_
