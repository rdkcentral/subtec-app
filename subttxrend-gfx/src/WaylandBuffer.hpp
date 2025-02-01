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


#ifndef SUBTTXREND_GFX_WAYLAND_BUFFER_HPP_
#define SUBTTXREND_GFX_WAYLAND_BUFFER_HPP_

#include <memory>

#include "waylandcpp-client/Types.hpp"
#include "waylandcpp-client/Buffer.hpp"
#include "waylandcpp-client/Shm.hpp"

namespace subttxrend
{
namespace gfx
{

/**
 * Wayland buffer.
 *
 * The buffer is protected from release when currently attached to surface.
 */
class WaylandBuffer : public std::enable_shared_from_this<WaylandBuffer>,
                      private waylandcpp::BufferListener
{
public:
    /** Buffer pointer type. */
    typedef std::shared_ptr<WaylandBuffer> Ptr;

    /**
     * Constructor.
     *
     * @param buffer
     *      Wayland buffer to be wrapped.
     * @param empty
     *      'Empty' buffer if the flag is set to true.
     */
    WaylandBuffer(waylandcpp::Buffer1::Ptr buffer,
                  bool empty);

    /**
     * Destructor.
     *
     * Releases the buffer.
     */
    virtual ~WaylandBuffer();

    /**
     * Marks the buffer as attached to surface.
     *
     * This method shall be used to protect the buffer from being release
     * when currently used for rendering by the surface. The buffer listens
     * to release events by itself and releases automatically when needed.
     *
     * @return
     *      Wayland buffer that could be attached to surface.
     */
    waylandcpp::Buffer1::Ptr markAttached();

    /**
     * Returns wayland buffer parameters.
     *
     * @return
     *      Parameters.
     */
    const waylandcpp::BufferParams& getParams() const;

    /**
     * Returns wayland buffer data pointer.
     *
     * @return
     *      Data pointer.
     */
    void* getDataPtr() const;

    /**
     * Checks if buffer is an 'empty' buffer.
     *
     * Empty buffers are used to render hidden surface.
     *
     * @return
     *      True if buffer is 'empty', false otherwise.
     */
    bool isEmpty() const
    {
        return m_empty;
    }

    /**
     * Checks if buffer is currently used (attached).
     *
     * @return
     *      True if attached, false otherwise.
     */
    bool isInUse() const
    {
        return static_cast<bool>(m_selfPtr);
    }

private:
    /** @copydoc waylandcpp::BufferListener::release */
    virtual void release(waylandcpp::BufferPtr object) override;

    /**
     * Wrapped wayland buffer.
     */
    waylandcpp::Buffer1::Ptr m_buffer;

    /**
     * Flags indicating that buffer is 'empty'.
     */
    const bool m_empty;

    /**
     * Pointer to self.
     *
     * This member is used to block release when buffer is attached to
     * the surface.
     */
    Ptr m_selfPtr;
};

} // namespace gfx
} // namespace subttxrend

#endif                          // SUBTTXREND_GFX_WAYLAND_BUFFER_HPP_
