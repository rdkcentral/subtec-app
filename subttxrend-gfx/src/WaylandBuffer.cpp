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


#include "WaylandBuffer.hpp"

#include <subttxrend/common/Logger.hpp>

#include "waylandcpp-client/File.hpp"

namespace subttxrend
{
namespace gfx
{

namespace
{

subttxrend::common::Logger g_logger("Gfx", "WaylandBuffer");

} // namespace <anonymous>

WaylandBuffer::WaylandBuffer(waylandcpp::Buffer1::Ptr buffer,
                             bool empty) :
        m_buffer(buffer),
        m_empty(empty)
{
    g_logger.trace("%s - Created (%p)", __func__, this);

    m_buffer->setListener(this);
}

WaylandBuffer::~WaylandBuffer()
{
    g_logger.trace("%s - Released (%p)", __func__, this);
}

waylandcpp::Buffer1::Ptr WaylandBuffer::markAttached()
{
    g_logger.trace("%s - %p", __func__, this);

    m_selfPtr = shared_from_this();
    return m_buffer;
}

const waylandcpp::BufferParams& WaylandBuffer::getParams() const
{
    return m_buffer->getParams();
}

void* WaylandBuffer::getDataPtr() const
{
    return m_buffer->getDataPtr();
}

void WaylandBuffer::release(waylandcpp::BufferPtr buffer)
{
    g_logger.trace("%s - %p buffer=%p", __func__, this, buffer.get());

    // must be last op
    m_selfPtr.reset();
}

} // namespace gfx
} // namespace subttxrend
