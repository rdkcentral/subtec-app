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


#include "Pixmap.hpp"

#include <cstring>
#include <stdexcept>
#include <subttxrend/common/Logger.hpp>

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "Pixmap");

}

Pixmap::Pixmap()
{
    reset();
}

void Pixmap::reset()
{
    g_logger.trace("%s", __func__);

    m_width = 0;
    m_height = 0;
    m_buffer = nullptr;
}

void Pixmap::init(std::int32_t width,
                  std::int32_t height,
                  std::uint8_t* buffer)
{
    g_logger.trace("%s size=%dx%d buffer=%p", __func__, width, height, buffer);

    if (width < 0)
    {
        throw std::invalid_argument("width");
    }
    if (height < 0)
    {
        throw std::invalid_argument("height");
    }
    if (!buffer)
    {
        throw std::invalid_argument("buffer");
    }

    m_width = width;
    m_height = height;
    m_buffer = buffer;
}

std::uint8_t* Pixmap::getLine(std::int32_t line)
{
    if (!m_buffer)
    {
        throw std::logic_error("Pixmap not initialized");
    }
    if ((line < 0) || (line >= getHeight()))
    {
        throw std::invalid_argument("line");
    }

    return m_buffer + line * m_width;
}

void Pixmap::clear(std::uint8_t colorIndex)
{
    g_logger.trace("%s index=%u", __func__, colorIndex);

    if (!m_buffer)
    {
        throw std::logic_error("Pixmap not initialized");
    }

    std::size_t size = m_width * m_height;

    (void) std::memset(m_buffer, colorIndex, size);
}

} // namespace dvbsubdecoder
