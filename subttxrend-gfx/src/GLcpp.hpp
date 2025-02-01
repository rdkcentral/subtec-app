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


#ifndef SUBTTXREND_GFX_GLCPP_HPP_
#define SUBTTXREND_GFX_GLCPP_HPP_

#include <memory>

// order is important
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

namespace glcpp
{

/**
 * Handler wrapper.
 */
template<typename T, T V_INVALID_VALUE>
class HandleWrapper
{
public:
    /** Invalid handle value. */
    static const T INVALID_VALUE = V_INVALID_VALUE;

    /** Handle type. */
    using Type = T;

    /**
     * Constructs the handle.
     *
     * @param handle
     *      Handle value.
     */
    HandleWrapper(T handle) :
            m_handle(handle)
    {
        // noop
    }

    /**
     * Constructs invalid handle.
     */
    HandleWrapper(std::nullptr_t = nullptr) :
            m_handle(INVALID_VALUE)
    {
        // noop
    }

    /**
     * Returns handle value.
     *
     * @return
     *      Handle value.
     */
    T get() const
    {
        return m_handle;
    }

    /**
     * Automatic cast to handle type.
     *
     * @return
     *      Handle value.
     */
    operator T()
    {
        return m_handle;
    }

    /**
     * Equality comparator.
     *
     * @param a
     *      First handle to compare.
     * @param b
     *      Seconds handle to compare.
     *
     * @return
     *      True if handles are equal, false otherwise.
     */
    friend bool operator ==(const HandleWrapper& a,
                            const HandleWrapper& b)
    {
        return a.m_handle == b.m_handle;
    }

    /**
     * Inequality comparator.
     *
     * @param a
     *      First handle to compare.
     * @param b
     *      Seconds handle to compare.
     *
     * @return
     *      True if handles are not equal, false otherwise.
     */
    friend bool operator !=(const HandleWrapper& a,
                            const HandleWrapper& b)
    {
        return !(a == b);
    }

private:
    /** Wrappped handle. */
    T m_handle;
};

/**
 * Handler deleter type.
 */
template<typename T, void (*DP)(typename T::Type)>
class HandleDeleter
{
public:
    /** Pointer type (used by std::unique_ptr). */
    using pointer = T;

    /**
     * Delete operation.
     *
     * @param handle
     *      Handle to delete.
     */
    void operator()(T handle)
    {
        if (handle.get() != T::INVALID_VALUE)
        {
            DP(handle);
        }
    }
};

/**
 * GL handle type helper.
 */
template<typename T, T INVALID_VALUE, void (*DP)(T)>
class HandleHelper
{
public:
    /** Handle wrapper type. */
    using HandleType = HandleWrapper<T,INVALID_VALUE>;

    /** Deleter type. */
    using DeleterType = HandleDeleter<HandleType,DP>;

    /** Pointer (handle) type. */
    using PtrType = std::unique_ptr<HandleType,DeleterType>;
};

/**
 * GL utilities.
 */
class GlUtils
{
public:
    /**
     * Generate (create) single texture.
     *
     * @return
     *      ID of the created texture, 0 on error.
     */
    static GLuint glGenTexture();

    /**
     * Deletext single texture.
     *
     * @param textureId
     *      Texture to delete.
     */
    static void glDeleteTexture(GLuint textureId);
};

/** Shader pointer (handle). */
using ShaderPtr = HandleHelper<GLuint, 0, glDeleteShader>::PtrType;

/** Program pointer (handle). */
using ProgramPtr = HandleHelper<GLuint, 0, glDeleteProgram>::PtrType;

/** Texture pointer (handle). */
using TexturePtr = HandleHelper<GLuint, 0, GlUtils::glDeleteTexture>::PtrType;

} // namespace glcpp

#endif                          // SUBTTXREND_GFX_GLCPP_HPP_
