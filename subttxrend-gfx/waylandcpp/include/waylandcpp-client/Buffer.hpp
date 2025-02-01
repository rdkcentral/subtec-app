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


#ifndef WAYLANDCPP_BUFFER_HPP_
#define WAYLANDCPP_BUFFER_HPP_

#include "Types.hpp"

#include <string>

#include "PixelFormat.hpp"

namespace waylandcpp
{

class BufferListenerWrapper;

/**
 * Wayland buffer events listener.
 */
class BufferListener
{
public:
    /**
     * Constructor.
     */
    BufferListener() = default;

    /**
     * Destructor.
     */
    virtual ~BufferListener() = default;

    /**
     * compositor releases buffer
     *
     * @param object
     *      Object for which event is called.
     */
    virtual void release(BufferPtr object) = 0;
};

/** Parameters. */
struct BufferParams
{
    /**
     * Constructor.
     *
     * @param offset
     *      Start offset in the memort buffer.
     * @param width
     *      Width in pixels.
     * @param height
     *      Height in pixels.
     * @param format
     *      Pixel format.
     */
    BufferParams(int32_t offset,
                 int32_t width,
                 int32_t height,
                 PixelFormat format) :
            m_offset(offset),
            m_width(width),
            m_height(height),
            m_stride(width * PixelFormatUtils::getBytesPerPixel(format)),
            m_format(format)
    {
        // noop
    }

    /**
     * Constructor.
     *
     * @param offset
     *      Start offset in the memort buffer.
     * @param width
     *      Width in pixels.
     * @param height
     *      Height in pixels.
     * @param stride
     *      Line stride (bytes per line).
     * @param format
     *      Pixel format.
     */
    BufferParams(int32_t offset,
                 int32_t width,
                 int32_t height,
                 int32_t stride,
                 PixelFormat format) :
            m_offset(offset),
            m_width(width),
            m_height(height),
            m_stride(stride),
            m_format(format)
    {
        // noop
    }

    /**
     * Returns minimum size of memory required.
     *
     * @return
     *      Minimum required memory size in bytes.
     */
    std::size_t getMemSize() const
    {
        return m_stride * m_height;
    }

    /** Start offset in the memort buffer. */
    int32_t m_offset;

    /** Width in pixels. */
    int32_t m_width;

    /** Height in pixels. */
    int32_t m_height;

    /** Line stride (bytes per line). */
    int32_t m_stride;

    /** Pixel format. */
    PixelFormat m_format;
};

/**
 * Buffer (common).
 */
class Buffer : public Proxy<Buffer, wl_buffer>
{
    friend class ShmPool1;

public:
    /** Pointer type. */
    typedef std::shared_ptr<Buffer> Ptr;

    /**
     * Returns object wayland interface.
     *
     * @return
     *      Pointer to wayland interface.
     */
    static const wl_interface* getWlInterface();

    /**
     * Destructor.
     */
    virtual ~Buffer();

    /**
     * Returns parameters used during buffer creation.
     *
     * @return
     *      Buffer parameters.
     */
    const BufferParams& getParams() const;

    /** @copydoc Proxy::setUserData */
    virtual void setUserData(void* userData) override final;

    /** @copydoc Proxy::getUserData */
    virtual void* getUserData() const override final;

    /**
     * Sets object listner.
     *
     * @param listener
     *      Listner to set.
     *
     * @retval true
     *      Success.
     * @retval false
     *      Failure (e.g. listener already set).
     */
    bool setListener(BufferListener* listener);

    /**
     * Returns pointer to buffer data.
     *
     * @return
     *      Pointe rto buffer data.
     */
    void* getDataPtr();

protected:
    /**
     * Constructor.
     *
     * For derived types.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param file
     *      Buffer file.
     * @param params
     *      Buffer parameters.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    Buffer(WaylandObjectType* const wlObject,
           const FilePtr& file,
           const BufferParams& params,
           DestructorFunc const destructorFunction);

private:
    /** Wrapper for object listeners. */
    std::unique_ptr<BufferListenerWrapper> m_listenerWrapper;

    /** Buffer file. */
    FilePtr m_file;

    /** Buffer parameters. */
    BufferParams m_params;
};

/**
 * Buffer (version 1).
 */
class Buffer1 : public Buffer
{
public:
    /** Pointer type. */
    typedef std::shared_ptr<Buffer1> Ptr;

    /** Minimum required object version. */
    static const uint32_t OBJECT_VERSION = 1;

    /**
     * Constructor.
     *
     * Uses default destructor function.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param file
     *      Buffer file.
     * @param params
     *      Buffer parameters.
     */
    Buffer1(WaylandObjectType* const wlObject,
            const FilePtr& file,
            const BufferParams& params);

protected:
    /**
     * Constructor.
     *
     * For derived types.
     *
     * @param wlObject
     *      Wrapped wayland object.
     * @param file
     *      Buffer file.
     * @param params
     *      Buffer parameters.
     * @param destructorFunction
     *      Function to be used to destroy the object.
     */
    Buffer1(WaylandObjectType* const wlObject,
            const FilePtr& file,
            const BufferParams& params,
            DestructorFunc const destructorFunction) :
            Buffer(wlObject, file, params, destructorFunction)
    {
        // noop
    }
};

} // namespace waylandcpp

#endif /*WAYLANDCPP_BUFFER_HPP_*/
