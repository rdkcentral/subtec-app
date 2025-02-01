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


#include "Region.hpp"

#include <stdexcept>

#include <subttxrend/common/Logger.hpp>

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "Region");

} // namespace <anonmymous>

Region::Region()
{
    reset();
}

void Region::reset()
{
    g_logger.trace("%s", __func__);

    if (m_objectRefs.getFirst())
    {
        throw std::logic_error("Region cannot be reset with objects assigned");
    }

    m_id = 0;
    m_version = INVALID_VERSION;

    m_pixmap.reset();

    m_clut = nullptr;

    m_compatibilityLevel = 0xFF;
    m_depth = 0xFF;

    m_backgroundIndex = 0;
}

void Region::init(std::int32_t width,
                  std::int32_t height,
                  std::uint8_t* pixmapMemory,
                  std::uint8_t compatibilityLevel,
                  std::uint8_t depth,
                  Clut* clut)
{
    g_logger.trace("%s - size=%dx%d pixmap=%p comp=%u depth=%u clut=%p",
            __func__, width, height, pixmapMemory, compatibilityLevel, depth,
            clut);

    // m_id would be set separately
    // m_version would be set separately

    m_pixmap.init(width, height, pixmapMemory);

    m_clut = clut;

    m_compatibilityLevel = compatibilityLevel;
    m_depth = depth;

    // m_backgroundIndex would be set separately
}

std::uint8_t Region::getId() const
{
    return m_id;
}

void Region::setId(std::uint8_t id)
{
    g_logger.trace("%s - id=%u", __func__, id);

    m_id = id;
}

std::uint8_t Region::getVersion() const
{
    return m_version;
}

void Region::setVersion(std::uint8_t version)
{
    g_logger.trace("%s - version=%u", __func__, version);

    m_version = version;
}

std::uint8_t Region::getBackgroundIndex() const
{
    return m_backgroundIndex;
}

void Region::setBackgroundIndex(std::uint8_t index)
{
    g_logger.trace("%s - index=%u", __func__, index);

    m_backgroundIndex = index;
}

std::int32_t Region::getWidth() const
{
    return m_pixmap.getWidth();
}

std::int32_t Region::getHeight() const
{
    return m_pixmap.getHeight();
}

std::uint8_t Region::getCompatibilityLevel() const
{
    return m_compatibilityLevel;
}

std::uint8_t Region::getDepth() const
{
    return m_depth;
}

std::uint8_t Region::getClutId() const
{
    if (!m_clut)
    {
        throw std::logic_error("Clut not set");
    }

    return m_clut->getId();
}

Pixmap& Region::getPixmap()
{
    return m_pixmap;
}

Clut* Region::getClut()
{
    return m_clut;
}

const std::uint32_t* Region::getClutArray() const
{
    if (!m_clut)
    {
        throw std::logic_error("Clut not set");
    }

    switch (m_depth)
    {
    case RegionDepthBits::DEPTH_2BIT:
        return m_clut->getArray2bit();
    case RegionDepthBits::DEPTH_4BIT:
        return m_clut->getArray4bit();
    case RegionDepthBits::DEPTH_8BIT:
        return m_clut->getArray8bit();
    default:
        throw std::logic_error("Unsupported depth");
    }
}

} // namespace dvbsubdecoder
