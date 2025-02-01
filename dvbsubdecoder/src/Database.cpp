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


#include "Database.hpp"

#include <subttxrend/common/Logger.hpp>

#include "PixmapAllocator.hpp"

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "Database");

} // namespace <anonmymous>

Database::Database(Specification specVersion,
                   PixmapAllocator& pixmapAllocator) :
        m_pixmapAllocator(pixmapAllocator),
        m_isEpochStart(false),
        m_status(specVersion),
        m_currentDisplay(),
        m_parsedDisplay(),
        m_renderingStatePrevious(&m_renderingStates[0]),
        m_renderingStateCurrent(&m_renderingStates[1])
{
    g_logger.trace("%s", __func__);

    m_pixmapAllocator.reset();
}

Database::~Database()
{
    g_logger.trace("%s", __func__);

    epochReset();
}

void Database::epochReset()
{
    g_logger.trace("%s", __func__);

    for (std::size_t i = 0; i < m_regions.getCount(); ++i)
    {
        removeRegionObjects(m_regions.getByIndex(i));
    }
    m_regions.reset();

    m_cluts.reset();
    m_pixmapAllocator.reset();
    m_page.reset();

    m_isEpochStart = true;
}

void Database::commitPage()
{
    // page is commited (complete)
    // - so we are no longer at start of epoch
    m_isEpochStart = false;
}

bool Database::canAddRegion() const
{
    // only allowed if:
    // - is epoch start (regions are being gathered)
    // - page is being parsed
    // - and there is space for regions
    return m_isEpochStart && m_page.getState() == Page::State::INCOMPLETE
            && m_regions.canAdd();
}

Region* Database::addRegionAndClut(std::uint8_t regionId,
                                   std::int32_t width,
                                   std::int32_t height,
                                   std::uint8_t compatibilityLevel,
                                   std::uint8_t depth,
                                   std::uint8_t clutId)
{
    g_logger.trace("%s - region=%u size=%dx%d comp=%u depth=%u clut=%u",
            __func__, regionId, width, height, compatibilityLevel, depth,
            clutId);

    if (!canAddRegion())
    {
        g_logger.info("%s - region allocation disabled", __func__);
        return nullptr;
    }

    if ((width <= 0) || (height <= 0))
    {
        g_logger.info("%s - invalid size", __func__);
        return nullptr;
    }

    if (getRegionById(regionId))
    {
        g_logger.info("%s - region already exists", __func__);
        return nullptr;
    }

    auto clut = getClutById(clutId);

    // check if it is needed to allocate clut
    if (!clut && !m_cluts.canAdd())
    {
        g_logger.info("%s - cannot add CLUT", __func__);
        return nullptr;
    }

    // check if there is memory for pixmap
    const std::size_t pixmapSize = width * height;
    if (!m_pixmapAllocator.canAllocate(pixmapSize))
    {
        g_logger.info("%s - cannot allocate pixmap memory (%zu bytes)",
                __func__, pixmapSize);
        return nullptr;
    }

    // check if there is memory for region
    if (!m_regions.canAdd())
    {
        g_logger.info("%s - cannot add region", __func__);
        return nullptr;
    }

    // allocate clut if needed
    if (!clut)
    {
        clut = m_cluts.add(clutId);
        if (!clut)
        {
            // shall never happen
            g_logger.error("%s - adding CLUT failed", __func__);
            return nullptr;
        }
        else
        {
            clut->reset();
        }
    }

    // allocate pixmap
    auto pixmap = m_pixmapAllocator.allocate(pixmapSize);
    if (!pixmap)
    {
        // shall never happen as it is checked before
        throw std::logic_error("allocation of pixmap memory failed");
    }

    // allocate region
    auto region = m_regions.add(regionId);
    if (!region)
    {
        // shall never happen as it is checked before
        // (the bitmap memory is lost until allocator reset - new epoch)
        throw std::logic_error("allocation of region failed");
    }

    region->init(width, height, pixmap, compatibilityLevel, depth, clut);

    g_logger.trace("%s - region added (%u -> %p)", __func__, regionId, region);

    return region;
}

void Database::swapRenderingStates()
{
    std::swap(m_renderingStateCurrent, m_renderingStatePrevious);
}

RenderingState& Database::getCurrentRenderingState()
{
    return *m_renderingStateCurrent;
}

RenderingState& Database::getPreviousRenderingState()
{
    return *m_renderingStatePrevious;
}

void Database::removeRegionObjects(Region* region)
{
    for (auto objectRef = region->removeFirstObject(); objectRef; objectRef =
            region->removeFirstObject())
    {
        m_objectRefsPool.release(objectRef);
    }
}

bool Database::addRegionObject(Region* region,
                               std::uint16_t objectId,
                               std::int32_t positionX,
                               std::int32_t positionY)
{
    auto objectRef = m_objectRefsPool.alloc();
    if (objectRef)
    {
        objectRef->m_objectId = objectId;
        objectRef->m_positionX = positionX;
        objectRef->m_positionY = positionY;

        region->addObject(objectRef);

        return true;
    }
    else
    {
        return false;
    }
}

} // namespace dvbsubdecoder
