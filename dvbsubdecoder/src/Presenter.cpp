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


#include "Presenter.hpp"

#include <array>
#include <limits>
#include <subttxrend/common/Logger.hpp>

#include "Database.hpp"
#include "DecoderClient.hpp"
#include "Consts.hpp"
#include "RenderingState.hpp"

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "Presenter");

bool operator ==(const Rectangle& rect1,
                 const Rectangle& rect2)
{
    return rect1.m_x1 == rect2.m_x1 && rect1.m_y1 == rect2.m_y1
            && rect1.m_x2 == rect2.m_x2 && rect1.m_y2 == rect2.m_y2;
}

bool operator !=(const Rectangle& rect1,
                 const Rectangle& rect2)
{
    return !(rect1 == rect2);
}

bool operator ==(const RenderingState::RegionInfo& info1,
                 const RenderingState::RegionInfo& info2)
{
    return (info1.m_id == info2.m_id) && (info1.m_version == info2.m_version)
            && (info1.m_rectangle == info2.m_rectangle);
}

/**
 * Extends the rectangle so it contains the rectangle given.
 *
 * @param target
 *      Rectangle to extend.
 * @param source
 *      Rectangle to be contained in target
 */
void extendRectangle(Rectangle& target,
                     const Rectangle& source)
{
    target.m_x1 = std::min(target.m_x1, source.m_x1);
    target.m_y1 = std::min(target.m_y1, source.m_y1);
    target.m_x2 = std::max(target.m_x2, source.m_x2);
    target.m_y2 = std::max(target.m_y2, source.m_y2);
}

} // namespace <anonmymous>

Presenter::Presenter(DecoderClient& client,
                     Database& database) :
        m_client(client),
        m_database(database)
{
    // noop
}

void Presenter::draw()
{
    // how it works:
    // 1. If the page was invalidated OR it is first call
    //    the previous state is invalid forcing the page
    //    to be redrawn including screen bounds notification
    //    (display always have valid values)
    // 2. If the page is COMPLETE then the new state has list
    //    of regions to draw. Comparison to the old state (empty
    //    if invalid) detects which regions are already on screen.
    // 2.1. The regions already on screen (same in both previous and
    //      current state) are not drawn.
    // 2.2. The regions that are only in previous state are cleared.
    // 2.3. The regions that are only in current state are drawn.
    // 3. If the page is INCOMPLETE or INVALID then the current state
    //    is empty.
    // 3.1. Page is INCOMPLETE if new PES was parsed but not all data
    //      was available. As process() parses all data before or equal
    //      to STC then the data was not in a buffer or the next PES has
    //      different PTS value - so the subtitles shall be cleared.
    // 3.2  Page is INVALID if there was no data or the page timeout
    //      occured - so the subtitles shall be cleared.

    g_logger.trace("%s", __func__);

    m_database.swapRenderingStates();

    auto& prevState = m_database.getPreviousRenderingState();
    auto& currState = m_database.getCurrentRenderingState();

    // invalid rectangle to allow simple use of min/max to
    // update the modified region boundaries.
    //
    // if nothing is modified it is fixed at the end
    Rectangle modifiedRectangle =
    { std::numeric_limits<std::int32_t>::max(),
            std::numeric_limits<std::int32_t>::max(), 0, 0 };

    buildCurrentState(currState);

    notifyScreenBounds(currState, prevState, modifiedRectangle);

    detectUnmodifiedRegions(currState, prevState);

    clearRegions(prevState, modifiedRectangle);

    drawRegions(currState, modifiedRectangle);

    if ((modifiedRectangle.m_x1 > modifiedRectangle.m_x2) ||
            (modifiedRectangle.m_y1 > modifiedRectangle.m_y2))
    {
        modifiedRectangle = { 0, 0, 0, 0 };
    }

    m_client.gfxFinish(modifiedRectangle);

    g_logger.trace("%s - finished", __func__);
}

void Presenter::invalidate()
{
    g_logger.trace("%s", __func__);

    // resetting current state so next redraw call would be
    // forced to redraw subtitles
    m_database.getCurrentRenderingState().reset();
}

void Presenter::buildCurrentState(RenderingState& state)
{
    g_logger.trace("%s", __func__);

    state.reset();

    const auto& display = m_database.getCurrentDisplay();

    state.setBounds(display.getDisplayBounds(), display.getWindowBounds());

    const auto& page = m_database.getPage();
    if (page.getState() == Page::State::COMPLETE)
    {
        const auto regionRefsCount = page.getRegionCount();
        for (std::size_t i = 0; i < regionRefsCount; ++i)
        {
            const auto& regionRef = page.getRegion(i);

            g_logger.trace("%s - processing region %u", __func__,
                    regionRef.m_regionId);

            const Region* region = m_database.getRegionById(
                    regionRef.m_regionId);
            if (!region)
            {
                g_logger.info("%s - region not found: %u", __func__,
                        regionRef.m_regionId);
                continue;
            }

            Rectangle regionRect =
            { regionRef.m_positionX, regionRef.m_positionY,
                    regionRef.m_positionX + region->getWidth(),
                    regionRef.m_positionY + region->getHeight() };

            g_logger.trace("%s - region %u rect: %u,%u-%u,%u", __func__,
                    regionRef.m_regionId, regionRect.m_x1, regionRect.m_y1,
                    regionRect.m_x2, regionRect.m_y2);

            if (!isRectangleInside(state.getWindowBounds(), regionRect))
            {
                g_logger.info(
                        "%s - rectangle for region %u does not fit in window",
                        __func__, regionRef.m_regionId);
                continue;
            }

            // shift to screen coordinates as required by API
            const auto& windowBounds = state.getWindowBounds();

            regionRect.m_x1 += windowBounds.m_x1;
            regionRect.m_y1 += windowBounds.m_y1;
            regionRect.m_x2 += windowBounds.m_x1;
            regionRect.m_y2 += windowBounds.m_y1;

            if (!state.addRegion(regionRef.m_regionId, region->getVersion(),
                    regionRect))
            {
                g_logger.info("%s - cannot add region %u", __func__,
                        regionRef.m_regionId);
                continue;
            }

            g_logger.trace("%s - region %u added", __func__,
                    regionRef.m_regionId);
        }
    }
}

void Presenter::notifyScreenBounds(RenderingState& currentState,
                                   RenderingState& previousState,
                                   Rectangle& modifiedRectangle)
{
    if ((currentState.getDisplayBounds() != previousState.getDisplayBounds())
            || (currentState.getWindowBounds()
                    != previousState.getWindowBounds()))
    {
        g_logger.trace("%s - notifying screen bounds", __func__);

        m_client.gfxSetDisplayBounds(currentState.getDisplayBounds(),
                currentState.getWindowBounds());

        // screen was cleared, so remove all old regions
        previousState.removeAllRegions();

        extendRectangle(modifiedRectangle, currentState.getDisplayBounds());
    }
}

void Presenter::detectUnmodifiedRegions(RenderingState& currentState,
                                        RenderingState& previousState)
{
    currentState.markAllRegionsAsDirty();
    previousState.markAllRegionsAsDirty();

    const auto currCount = currentState.getRegionCount();
    const auto prevCount = previousState.getRegionCount();

    for (std::size_t currIndex = 0; currIndex < currCount; ++currIndex)
    {
        const auto& currRegion = currentState.getRegionByIndex(currIndex);

        for (std::size_t prevIndex = 0; prevIndex < prevCount; ++prevIndex)
        {
            const auto& prevRegion = previousState.getRegionByIndex(prevIndex);

            if (currRegion == prevRegion)
            {
                currentState.unmarkRegionAsDirtyByIndex(currIndex);
                previousState.unmarkRegionAsDirtyByIndex(prevIndex);
            }
        }
    }
}

void Presenter::clearRegions(const RenderingState& previousState,
                             Rectangle& modifiedRectangle)
{
    const auto regionCount = previousState.getRegionCount();

    g_logger.trace("%s - count: %zu", __func__, regionCount);

    for (std::size_t index = 0; index < regionCount; ++index)
    {
        const auto& regionInfo = previousState.getRegionByIndex(index);

        if (regionInfo.m_dirty)
        {
            m_client.gfxClear(regionInfo.m_rectangle);

            g_logger.trace("%s - region %zu (id=%u) cleared at %d,%d-%d,%d",
                    __func__, index, regionInfo.m_id,
                    regionInfo.m_rectangle.m_x1, regionInfo.m_rectangle.m_y1,
                    regionInfo.m_rectangle.m_x2, regionInfo.m_rectangle.m_y2);

            extendRectangle(modifiedRectangle, regionInfo.m_rectangle);
        }
        else
        {
            g_logger.trace("%s - region %zu (id=%u) marked as not modified",
                    __func__, index, regionInfo.m_id);
        }
    }
}

void Presenter::drawRegions(const RenderingState& currentState,
                            Rectangle& modifiedRectangle)
{
    const auto regionCount = currentState.getRegionCount();

    g_logger.trace("%s - count: %zu", __func__, regionCount);

    for (std::size_t index = 0; index < regionCount; ++index)
    {
        const auto& regionInfo = currentState.getRegionByIndex(index);

        if (regionInfo.m_dirty)
        {
            const auto region = m_database.getRegionById(regionInfo.m_id);

            if (!region)
            {
                g_logger.fatal("%s - region %zu (id=%u) not found", __func__,
                        index, regionInfo.m_id);
                continue;
            }

            const auto clutArray = region->getClutArray();
            if (!clutArray)
            {
                g_logger.fatal("%s - region %zu (id=%u) clut not found",
                        __func__, index, regionInfo.m_id);
                continue;
            }

            const auto& pixmap = region->getPixmap();

            Bitmap bitmap =
            { static_cast<std::uint32_t>(pixmap.getWidth()),
                    static_cast<std::uint32_t>(pixmap.getHeight()),
                    pixmap.getBuffer(), clutArray };

            Rectangle srcRect =
            { 0, 0, pixmap.getWidth(), pixmap.getHeight() };

            m_client.gfxDraw(bitmap, srcRect, regionInfo.m_rectangle);

            g_logger.trace("%s - region %zu (id=%u) drawn at %d,%d-%d,%d",
                    __func__, index, regionInfo.m_id,
                    regionInfo.m_rectangle.m_x1, regionInfo.m_rectangle.m_y1,
                    regionInfo.m_rectangle.m_x2, regionInfo.m_rectangle.m_y2);

            extendRectangle(modifiedRectangle, regionInfo.m_rectangle);
        }
        else
        {
            g_logger.trace("%s - region %zu (%u) marked as not modified",
                    __func__, index, regionInfo.m_id);
        }
    }
}

bool Presenter::isRectangleInside(const Rectangle& bounds,
                                  const Rectangle& rect)
{
    // check validity
    if ((bounds.m_x1 >= bounds.m_x2) || (bounds.m_y1 >= bounds.m_y2))
    {
        return false;
    }
    if ((rect.m_x1 >= rect.m_x2) || (rect.m_y1 >= rect.m_y2))
    {
        return false;
    }

    if ((rect.m_x1 < bounds.m_x1) || (rect.m_x2 > bounds.m_x2))
    {
        return false;
    }
    if ((rect.m_y1 < bounds.m_y1) || (rect.m_y2 > bounds.m_y2))
    {
        return false;
    }

    return true;
}

} // namespace dvbsubdecoder
