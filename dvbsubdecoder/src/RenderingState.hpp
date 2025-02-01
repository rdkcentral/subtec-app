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


#ifndef DVBSUBDECODER_RENDERINGSTATE_HPP_
#define DVBSUBDECODER_RENDERINGSTATE_HPP_

#include <cstdint>
#include <array>
#include <stdexcept>

#include "Types.hpp"
#include "Config.hpp"

namespace dvbsubdecoder
{

/**
 * Subtitles rendering state.
 *
 * Describes window rectangle and rendered regions.
 */
class RenderingState
{
public:
    /**
     * Single rendered region information.
     */
    struct RegionInfo
    {
        /** Region identifier. */
        std::uint8_t m_id;
        /** Region version. */
        std::uint8_t m_version;
        /** Region rectangle (location on display). */
        Rectangle m_rectangle;
        /**
         * Region dirty flag.
         *
         * The flag is used during calculations to determine which regions
         * must be drawn.
         */
        bool m_dirty;
    };

    /**
     * Constructor.
     */
    RenderingState()
    {
        reset();
    }

    /**
     * Resets the state.
     *
     * The display rectangle is invalidated and all regions are removed.
     */
    void reset()
    {
        m_displayBounds =
        {   0, 0, 0, 0};
        m_windowBounds =
        {   0, 0, 0, 0};
        m_regionCount = 0;
    }

    /**
     * Returns display bounds.
     *
     * @return
     *      Display bounds.
     */
    const Rectangle& getDisplayBounds() const
    {
        return m_displayBounds;
    }

    /**
     * Returns window bounds.
     *
     * @return
     *      Window bounds.
     */
    const Rectangle& getWindowBounds() const
    {
        return m_windowBounds;
    }

    /**
     * Sets display and window bounds.
     *
     * @param displayBounds
     *      Display bounds.
     * @param windowBounds
     *      Window bounds.
     */
    void setBounds(const Rectangle& displayBounds,
            const Rectangle& windowBounds)
    {
        m_displayBounds = displayBounds;
        m_windowBounds = windowBounds;
    }

    /**
     * Removes all regions.
     */
    void removeAllRegions()
    {
        m_regionCount = 0;
    }

    /**
     * Marks all currently defined regions as dirty.
     */
    void markAllRegionsAsDirty()
    {
        for (std::size_t i = 0; i < m_regionCount; ++i)
        {
            m_regions[i].m_dirty = true;
        }
    }

    /**
     * Unmarks region from being dirty.
     *
     * @param index
     *      Index of region to unmark.
     */
    void unmarkRegionAsDirtyByIndex(std::size_t index)
    {
        if (index >= m_regionCount)
        {
            throw std::range_error("index");
        }
        m_regions[index].m_dirty = false;
    }

    /**
     * Adds region.
     *
     * @param id
     *      Region identifier.
     * @param version
     *      Region version.
     * @param rectangle
     *      Region rectangle (display coordinates).
     *
     * @return
     *      True on success, false on error (no space for region).
     */
    bool addRegion(std::uint8_t id,
            std::uint8_t version,
            const Rectangle& rectangle)
    {
        if (m_regionCount < m_regions.size())
        {
            m_regions[m_regionCount].m_id = id;
            m_regions[m_regionCount].m_version = version;
            m_regions[m_regionCount].m_rectangle = rectangle;
            m_regions[m_regionCount].m_dirty = true;

            ++m_regionCount;

            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * Returns number of currently defined regions.
     *
     * @return
     *      Number of regions.
     */
    std::size_t getRegionCount() const
    {
        return m_regionCount;
    }

    /**
     * Return define region.
     *
     * @param index
     *      Index of region to get.
     *
     * @return
     *      Region information.
     */
    const RegionInfo& getRegionByIndex(std::size_t index) const
    {
        if (index >= m_regionCount)
        {
            throw std::range_error("index");
        }

        return m_regions[index];
    }

public:
    /** Display bounds. */
    Rectangle m_displayBounds;

    /** Window bounds. */
    Rectangle m_windowBounds;

    /** Array with regions. */
    std::array<RegionInfo, MAX_SUPPORTED_REGIONS> m_regions;

    /** Number of currently defined regions. */
    std::size_t m_regionCount;
};

}
 // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_RENDERINGSTATE_HPP_*/
