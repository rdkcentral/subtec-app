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


#include "WaylandBackendEgl.hpp"

#include <subttxrend/common/Logger.hpp>

#include "Pixel.hpp"
#include "Pixmap.hpp"

namespace subttxrend
{
namespace gfx
{

namespace
{

subttxrend::common::Logger g_logger("Gfx", "WaylandBackendEgl");

GLchar VERTEX_SHADER_SOURCE[] = ""
        "attribute vec4 aPosition; \n"
        "attribute vec2 aTexCoordinate; \n"
        "varying vec2 vTexCoordinate; \n"
        "void main() \n"
        "{ \n"
        " vTexCoordinate = aTexCoordinate; \n"
        " gl_Position = aPosition; \n"
        "} \n";

// The shader swaps pixel format from BGRA (little endian ARGB) to RGBA.
GLchar FRAGMENT_SHADER_SOURCE[] = ""
        "precision mediump float; \n"
        "uniform sampler2D uTexture; \n"
        "uniform sampler2D uBgTexture; \n"
        "varying vec2 vTexCoordinate; \n"
        "void main() \n"
        "{ \n"
        " vec4 textColor = texture2D( uTexture, vTexCoordinate ).bgra; \n"
        " vec4 backgroundColor = texture2D( uBgTexture, vTexCoordinate ).bgra; \n"
        " gl_FragColor = textColor + backgroundColor*(1.0-textColor.a); \n"
        "} \n";

const int TEXT_TEXTURE = 0;
const int BACKGROUND_TEXTURE = 1;
} // namespace <anonymous>

//------------------------------------------

const Size WaylandBackendEgl::DEFAULT_SIZE(720, 480);

WaylandBackendEgl::WaylandBackendEgl(BackendListener* listener) :
        WaylandBackend(listener),
        m_eglContext(nullptr),
        m_eglDisplay(nullptr),
        m_eglWindow(nullptr),
        m_eglSurface(nullptr),
        m_textureUniformHandle(-1),
        m_positionAttribHandle(-1),
        m_textureCoordinateHandle(-1),
        m_clearBufferSize(0),
        m_clearBuffer()
{
    // noop
}

WaylandBackendEgl::~WaylandBackendEgl()
{
    if(m_eglContext)
    {
        eglDestroyContext(m_eglDisplay, m_eglContext);
    }
    if(m_eglSurface)
    {
        eglDestroySurface(m_eglDisplay, m_eglSurface);
    }
    if(m_eglWindow)
    {
        wl_egl_window_destroy(m_eglWindow);
    }
}

bool WaylandBackendEgl::initRendering()
{
    // TODO: create wayland wrappers

#define WITH_MULTISAMPLING

    static const EGLint configAttributes[] =
    {
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
#ifdef WITH_MULTISAMPLING
            EGL_SAMPLE_BUFFERS, 1,
            EGL_SAMPLES, 2,
#endif /*WITH_MULTISAMPLING*/
            EGL_NONE };

    static const EGLint contextAttributes[] =
    {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE };

    EGLConfig config = nullptr;
    EGLint numConfig = 0;

    g_logger.trace("%s", __func__);

    m_eglDisplay = eglGetDisplay(getDisplay()->getNativeObject());
    if (!m_eglDisplay)
    {
        g_logger.error("%s - cannot get EGL display (%d)", __func__,
                eglGetError());
        return false;
    }

    EGLint eglVersionMajor = 0;
    EGLint eglVersionMinor = 0;
    if (!eglInitialize(m_eglDisplay, &eglVersionMajor, &eglVersionMinor))
    {
        g_logger.error("%s - cannot initialize EGL (%d)", __func__,
                eglGetError());
        return false;
    }

    g_logger.trace("%s - EGL initialized %d.%d", __func__, eglVersionMajor,
            eglVersionMinor);

    if (!eglBindAPI(EGL_OPENGL_ES_API))
    {
        g_logger.error("%s - cannot bind EGL API (%d)", __func__,
                eglGetError());
        return false;
    }

    if (!eglChooseConfig(m_eglDisplay, configAttributes, &config, 1,
            &numConfig))
    {
        g_logger.error("%s - cannot choose EGL config (%d)", __func__,
                eglGetError());
        return false;
    }

    m_eglContext = eglCreateContext(m_eglDisplay, config, EGL_NO_CONTEXT,
            contextAttributes);
    if (!m_eglContext)
    {
        g_logger.error("%s - cannot create EGL context (%d)", __func__,
                eglGetError());
        return false;
    }

    EGLint clientVersion = 0;
    if (!eglQueryContext(m_eglDisplay, m_eglContext, EGL_CONTEXT_CLIENT_VERSION,
            &clientVersion))
    {
        g_logger.error("%s - cannot query client version", __func__);
        return false;
    }

    g_logger.trace("%s - EGL client version: %d", __func__, clientVersion);

    if ((m_initialSize.m_w == 0) && (m_initialSize.m_h == 0))
    {
        m_initialSize = DEFAULT_SIZE;
    }

    g_logger.trace("%s - creating EGL wayland window of size %dx%d", __func__,
            m_initialSize.m_w, m_initialSize.m_h);

    m_eglWindow = wl_egl_window_create(getSurface()->getNativeObject(),
            m_initialSize.m_w, m_initialSize.m_h);
    if (!m_eglWindow)
    {
        g_logger.error("%s - cannot create EGL wayland window", __func__);
        return false;
    }

    m_eglSurface = eglCreateWindowSurface(m_eglDisplay, config, m_eglWindow,
    NULL);
    if (!m_eglSurface)
    {
        g_logger.error("%s - cannot create EGL window surface (%d)", __func__,
                eglGetError());
        return false;
    }

    if (!eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext))
    {
        g_logger.error("%s - cannot set current EGL context (%d)", __func__,
                eglGetError());
        return false;
    }

    m_program = std::move(linkShaderProgram());
    if (!m_program)
    {
        g_logger.error("%s - cannot init GL program", __func__);
        return false;
    }

    glUseProgram(m_program.get());

    m_textureUniformHandle = glGetUniformLocation(m_program.get(), "uTexture");
    m_textureBgUniformHandle = glGetUniformLocation(m_program.get(), "uBgTexture");
    m_positionAttribHandle = glGetAttribLocation(m_program.get(), "aPosition");
    m_textureCoordinateHandle = glGetAttribLocation(m_program.get(),
            "aTexCoordinate");

    g_logger.trace("%s - Handles: texsampler=%d position=%d texcoords=%d",
            __func__, m_textureUniformHandle, m_positionAttribHandle,
            m_textureCoordinateHandle);

    glDisable(GL_DEPTH_TEST);

    g_logger.trace("%s - finished with success", __func__);

    return true;
}

bool WaylandBackendEgl::checkRequiredInterfaces() const
{
    // no extra interfaces needed
    return true;
}

void WaylandBackendEgl::currentOutputModeChanged(const Size& size)
{
    if ((m_initialSize.m_w == 0) && (m_initialSize.m_h == 0))
    {
        g_logger.trace("%s - initial size set to %dx%d", __func__, size.m_w,
                size.m_h);

        m_initialSize = size;
    }
}

void WaylandBackendEgl::surfaceResizeRequested(const Size& size)
{
    if (m_eglWindow)
    {
        g_logger.trace("%s - resizing window to %dx%d", __func__, size.m_w,
                size.m_h);

        wl_egl_window_resize(m_eglWindow, size.m_w, size.m_h, 0, 0);
    }

    getListener()->onPreferredSize(size);
}

bool WaylandBackendEgl::prepareTexture(const Size& contentSize, int textureId)
{
    auto& texPtr = m_textures[textureId];
    auto& textureSize = m_textureSizes[textureId];
    if (!texPtr)
    {
        texPtr = glcpp::TexturePtr(glcpp::GlUtils::glGenTexture());
        if (!texPtr)
        {
            g_logger.error("%s - texture not created, error=%d", __func__,
                    glGetError());
            return false;
        }
    }

    if ((textureSize.m_w < contentSize.m_w)
            || (textureSize.m_h < contentSize.m_h))
    {
        // need to resize the texture
        glActiveTexture(GL_TEXTURE0 + textureId);
        glBindTexture(GL_TEXTURE_2D, texPtr.get());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // try to use integer size first
        glGetError(); // clear error
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, contentSize.m_w,
                contentSize.m_h, 0, GL_RGBA /*GL_BGRA*/,
                GL_UNSIGNED_BYTE /*GL_UNSIGNED_INT_8_8_8_8_REV*/, nullptr);
            
        if (glGetError() == GL_NO_ERROR)
        {
            textureSize = contentSize;

            g_logger.trace("%s - texture of size %dx%d created", __func__,
                    contentSize.m_w, contentSize.m_h);
            return true;
        }
        else
        {
            g_logger.info("%s - texture init to size: %dx%d failed", __func__,
                    contentSize.m_w, contentSize.m_h);
        }

        return false;
    }

    return true;
}

bool WaylandBackendEgl::drawOnTexture(const Size& contentSize)
{
    g_logger.trace("%s", __func__);

    class TextureRenderer : public BackendWindowEnumerator
    {
    public:
        TextureRenderer(const std::array<Size, 2>& textureSizes) :
                m_textureSizes(textureSizes)
        {
            // noop
        }

        virtual void processWindow(const Pixmap& pixmap) override
        {
            drawPixmapOnTexture(pixmap, TEXT_TEXTURE);
        }
#if BACKEND_TYPE == 2
        virtual void processWindow(const Pixmap& pixmap, const Pixmap& bgPixmap) override
        {
            drawPixmapOnTexture(pixmap, TEXT_TEXTURE);
            drawPixmapOnTexture(bgPixmap, BACKGROUND_TEXTURE);
        }
#endif

    private:
        void drawPixmapOnTexture(const Pixmap& pixmap, int textureId)
        {
            const auto pw = pixmap.getWidth();
            const auto ph = pixmap.getHeight();
            const auto& textureSize = m_textureSizes[textureId];

            if ((pw > textureSize.m_w) || (ph > textureSize.m_h))
            {
                g_logger.info("%s - pixmap larger than texture, skipping",
                        __func__);
                return;
            }

            const auto tx = (textureSize.m_w - pw) / 2;
            const auto ty = (textureSize.m_h - ph) / 2;

            glActiveTexture(GL_TEXTURE0 + textureId);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize.m_w, textureSize.m_h, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, pixmap.getLine(0).ptr());

        }
        const std::array<Size, 2> m_textureSizes;
    };

    TextureRenderer renderer(m_textureSizes);

    getListener()->enumerateVisibleWindows(renderer);

    return true;
}

void WaylandBackendEgl::drawTexturedObject(const Size& contentSize)
{
    g_logger.trace("%s - size=%dx%d", __func__, contentSize.m_w,
            contentSize.m_h);

    if (!m_textures[TEXT_TEXTURE])
    {
        g_logger.info("%s - text texture 0 not available, skipping", __func__);
        return;
    }
    if (!m_textures[BACKGROUND_TEXTURE])
    {
        g_logger.info("%s - background texture not available, skipping", __func__);
        return;
    }

    // 23
    // 14
    // fan: 1 2 3 4 gives 1 2 3 + 1 3 4
    static const GLfloat vVertices[] =
    { -1.0f, -1.0f, 0.0f, -1.0f, +1.0f, 0.0f, +1.0f, +1.0f, 0.0f, +1.0f, -1.0f,
            0.0f, };

    static const GLfloat vTexCoords[] =
    { 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f };

    // Set the viewport
    // TODO ??
    // glViewport(0, 0, 100, 100);

    // Use the program object
    glUseProgram(m_program.get());

    // Set the active texture unit to texture unit 0.
    glActiveTexture(GL_TEXTURE0);

    // Bind the texture to this unit.
    glBindTexture(GL_TEXTURE_2D, m_textures[TEXT_TEXTURE].get());

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_textures[BACKGROUND_TEXTURE].get());

    // Tell the texture uniform sampler to use this texture in the shader
    // by binding to texture unit 0.
    glUniform1i(m_textureUniformHandle, 0);
    glUniform1i(m_textureBgUniformHandle, 1);

    // Load the vertex data
    glVertexAttribPointer(m_positionAttribHandle, 3, GL_FLOAT, GL_FALSE, 0,
            vVertices);
    glEnableVertexAttribArray(m_positionAttribHandle);

    glVertexAttribPointer(m_textureCoordinateHandle, 2, GL_FLOAT, GL_FALSE, 0,
            vTexCoords);
    glEnableVertexAttribArray(m_textureCoordinateHandle);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    (void) checkForGlError();
}

void WaylandBackendEgl::redraw(const waylandcpp::Surface1::Ptr& surface)
{
    static bool visible = false;
    bool visibleChanged = false;

    g_logger.trace("%s", __func__);

    if (!m_eglDisplay)
    {
        g_logger.trace("%s - no display", __func__);
        return;
    }
    if (!m_eglSurface)
    {
        g_logger.trace("%s - no surface", __func__);
        return;
    }

    if (!m_program)
    {
        g_logger.trace("%s - no program", __func__);
        return;
    }

    const bool renderRequested = m_renderRequested.exchange(false);

    auto contentSize = calculateContentSize();

    const bool anythingToDraw = (contentSize.m_w > 0) || (contentSize.m_h > 0);

    // clear with transparent color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // render windows
    if (anythingToDraw)
    {
        if(!visible)
        {
            visible = true;
            visibleChanged = true;
        }

        if (renderRequested)
        {
            if (!prepareTexture(contentSize, TEXT_TEXTURE))
            {
                g_logger.error("%s - cannot prepare text texture of size %dx%d",
                        __func__, contentSize.m_w, contentSize.m_h);
                return;
            }
            if (!prepareTexture(contentSize, BACKGROUND_TEXTURE))
            {
                g_logger.error("%s - cannot prepare background texture of size %dx%d",
                        __func__, contentSize.m_w, contentSize.m_h);
                return;
            }

            if (!drawOnTexture(contentSize))
            {
                g_logger.error("%s - cannot draw contents", __func__);
                return;
            }
        }

        drawTexturedObject(contentSize);
    }
    else
    {
        if(visible)
        {
            visible = false;
            visibleChanged = true;
        }
    }

#if defined(WESTEROS)
   if(visibleChanged)
   {
        g_logger.info("%s - %s", __func__, visible ? "Something to draw, make surface visible" : "Nothing to draw, hide surface");
        if(getSimpleShell())
        {
            wl_simple_shell_set_visible(getSimpleShell()->getNativeObject(), m_currentSurfaceId, visible ? 1 : 0);
        }
   }
#endif

    m_frameReady = false;

    eglSwapBuffers(m_eglDisplay, m_eglSurface);
}

void WaylandBackendEgl::interfaceAdded(waylandcpp::Registry1::Ptr registry,
                                       uint32_t name,
                                       std::string interface,
                                       uint32_t version)
{
    // noop
}

void WaylandBackendEgl::interfaceRemoved(waylandcpp::Registry1::Ptr registry,
                                         uint32_t name)
{
    // noop
}

bool WaylandBackendEgl::checkForGlError()
{
    auto error = glGetError();

    if (error != GL_NO_ERROR)
    {
        const char* message = nullptr;

        switch (error)
        {
        case GL_INVALID_ENUM:
            message = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            message = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            message = "GL_INVALID_OPERATION";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            message = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            message = "GL_OUT_OF_MEMORY";
            break;
        default:
            message = "unknown";
            break;
        }

        g_logger.error("%s - GL error occurred: %s (%d)", __func__, message,
                error);

        return true;
    }

    return false;
}

glcpp::ShaderPtr WaylandBackendEgl::compileShader(const GLchar* const shaderSrc,
                                                  GLenum type)
{
    glcpp::ShaderPtr shader(glCreateShader(type));
    if (shader)
    {
        glShaderSource(shader.get(), 1, &shaderSrc, NULL);

        glCompileShader(shader.get());

        GLint compiled = 0;
        glGetShaderiv(shader.get(), GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            g_logger.error("%s - cannot compile shader of type=%d", __func__,
                    static_cast<int>(type));

            GLint infoLen = 0;
            glGetShaderiv(shader.get(), GL_INFO_LOG_LENGTH, &infoLen);

            if (infoLen > 1)
            {
                std::unique_ptr<char[]> infoLog(new char[infoLen]);

                glGetShaderInfoLog(shader.get(), infoLen, NULL, infoLog.get());

                g_logger.error("%s - Error compiling shader: %s", __func__,
                        infoLog.get());
            }

            shader.reset();
        }
    }
    else
    {
        g_logger.error("%s - cannot create shader of type=%d", __func__,
                static_cast<int>(type));
    }

    if (checkForGlError())
    {
        shader.reset();
    }

    return std::move(shader);
}

glcpp::ProgramPtr WaylandBackendEgl::linkShaderProgram()
{
    auto vertexShader = compileShader(VERTEX_SHADER_SOURCE, GL_VERTEX_SHADER);
    if (!vertexShader)
    {
        g_logger.error("%s - cannot load vertex shader", __func__);
        return nullptr;
    }

    auto fragmentShader = compileShader(FRAGMENT_SHADER_SOURCE,
    GL_FRAGMENT_SHADER);
    if (!fragmentShader)
    {
        g_logger.error("%s - cannot load fragment shader", __func__);
        return nullptr;
    }

    // Create the program object
    glcpp::ProgramPtr program(glCreateProgram());
    if (program)
    {
        glAttachShader(program.get(), vertexShader.get());
        glAttachShader(program.get(), fragmentShader.get());

        glLinkProgram(program.get());

        GLint linked = 0;
        glGetProgramiv(program.get(), GL_LINK_STATUS, &linked);
        if (!linked)
        {
            g_logger.error("%s - program link failed", __func__);

            GLint infoLen = 0;
            glGetProgramiv(program.get(), GL_INFO_LOG_LENGTH, &infoLen);

            if (infoLen > 1)
            {
                std::unique_ptr<char[]> infoLog(new char[infoLen]);

                glGetProgramInfoLog(program.get(), infoLen, NULL,
                        infoLog.get());
                g_logger.error("%s - Error linking program: %s", __func__,
                        infoLog.get());
            }

            program.reset();
        }
    }
    else
    {
        g_logger.error("%s - cannot create program", __func__);
    }

    if (checkForGlError())
    {
        program.reset();
    }

    return std::move(program);
}

} // namespace gfx
} // namespace subttxrend
