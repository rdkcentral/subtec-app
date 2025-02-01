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


#include "PixelWriter.hpp"

#include <stdexcept>
#include <subttxrend/common/Logger.hpp>

#include "Pixmap.hpp"

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "PixelWriter");

} // namespace <anonmymous>

PixelWriter::PixelWriter(bool nonModifyingColourFlag,
                         std::uint8_t depth,
                         Pixmap& pixmap,
                         std::int32_t startX,
                         std::int32_t startY) :
        m_nonModifyingColourFlag(nonModifyingColourFlag),
        m_depth(depth),
        m_pixmap(pixmap),
        m_startX(startX),
        m_currY(startY)
{
    g_logger.trace("%s - depth=%u pixmap=%dx%d start=%d,%d", __func__, depth,
            pixmap.getWidth(), pixmap.getHeight(), startX, startY);

    if ((depth <= 0) || (depth > 8))
    {
        throw std::invalid_argument("depth");
    }
    if (startX < 0)
    {
        throw std::invalid_argument("startX");
    }
    if (startY < 0)
    {
        throw std::invalid_argument("startY");
    }

    initLine();
}

void PixelWriter::setPixels(std::uint8_t pixelCode,
                            std::uint32_t count)
{
    if (!m_currentPtr)
    {
        return;
    }

    if ((pixelCode >> m_depth) != 0)
    {
        throw std::invalid_argument("pixelCode");
    }

    if (m_nonModifyingColourFlag && (pixelCode == 1))
    {
        if (m_lineEndPtr - m_currentPtr <= static_cast<std::ptrdiff_t>(count))
        {
            m_currentPtr = nullptr;
        }
        else
        {
            m_currentPtr += count;
        }
    }
    else
    {
        while (count-- > 0)
        {
            if (m_currentPtr < m_lineEndPtr)
            {
                *m_currentPtr++ = pixelCode;
            }
            else
            {
                m_currentPtr = nullptr;
                break;
            }
        }
    }
}

void PixelWriter::endLine()
{
    m_currY += 2;
    initLine();
}

void PixelWriter::initLine()
{
    g_logger.trace("%s - pos=%d,%d", __func__, m_startX, m_currY);

    if ((m_currY >= 0) && (m_currY < m_pixmap.getHeight()) && (m_startX >= 0)
            && (m_startX < m_pixmap.getWidth()))
    {
        auto linePtr = m_pixmap.getLine(m_currY);
        m_currentPtr = linePtr + m_startX;
        m_lineEndPtr = linePtr + m_pixmap.getWidth();

        g_logger.trace("%s - line=%p curr=%p end=%p", __func__, linePtr,
                m_currentPtr, m_lineEndPtr);
    }
    else
    {
        m_currentPtr = nullptr;
        m_lineEndPtr = nullptr;

        g_logger.trace("%s - outside, skipping", __func__);
    }
}

} // namespace dvbsubdecoder
