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


#ifndef DVBSUBDECODER_PAGE_HPP_
#define DVBSUBDECODER_PAGE_HPP_

#include <stdexcept>
#include <array>

#include "Config.hpp"
#include "Consts.hpp"
#include "Types.hpp"

namespace dvbsubdecoder
{

/**
 * Subtitles page.
 *
 * Holds current page segments information.
 */
class Page
{
public:
    /**
     * Page state.
     */
    enum class State
    {
        /** Page contents are invalid. */
        INVALID,

        /** Page is currently being parsed. */
        INCOMPLETE,

        /** Page was successfully parsed. */
        COMPLETE,

        /** Page was complete but timed out. */
        TIMEDOUT
    };

    /**
     * Region reference.
     */
    struct RegionReference
    {
        /** Region identifier. */
        std::uint8_t m_regionId;

        /** Region position X. */
        std::int32_t m_positionX;

        /** Region positon Y. */
        std::int32_t m_positionY;
    };

    /**
     * Constructor.
     *
     * Creates empty page (page is reset on initialization).
     */
    Page()
    {
        reset();
    }

    /**
     * Returns current version.
     *
     * @return
     *      Current version (may be INVALID_VERSION).
     */
    std::uint8_t getVersion() const
    {
        return m_version;
    }

    /**
     * Returns current state.
     *
     * @return
     *      Current state.
     */
    State getState() const
    {
        return m_state;
    }

    /**
     * Resets the page.
     *
     * State is set to INVALID and version to INVALID_VERSION.
     */
    void reset()
    {
        m_state = State::INVALID;
        m_version = INVALID_VERSION;
        m_pts = StcTime();
        m_timeout = 0;
        m_regionCount = 0;
    }

    /**
     * Marks the page as being currently parsed.
     *
     * Sets the version to given and state to incomplete.
     *
     * @param version
     *      Version of page being parsed.
     * @param pts
     *      Page PTS.
     * @param timeout
     *      Page timeout value in seconds.
     */
    void startParsing(std::uint8_t version,
                      const StcTime& pts,
                      const std::uint32_t timeout)
    {
        m_state = State::INCOMPLETE;
        m_version = version;
        m_pts = pts;
        m_timeout = timeout;
        m_regionCount = 0;
    }

    /**
     * Finishes the parsing.
     *
     * Sets page state to COMPLETE.
     */
    void finishParsing()
    {
        if (m_state != State::INCOMPLETE)
        {
            throw std::logic_error("Wrong state");
        }
        m_state = State::COMPLETE;
    }

    /**
     * Sets page as timed out.
     */
    void setTimedOut()
    {
        if (m_state != State::COMPLETE)
        {
            throw std::logic_error("Wrong state");
        }

        m_state = State::TIMEDOUT;
        m_regionCount = 0;
    }

    /**
     * Adds region to page.
     *
     * @param regionId
     *      Region identifier.
     * @param regionAddressX
     *      Region position (window relative) - X coordinate.
     * @param regionAddressY
     *      Region position (window relative) - Y coordinate.
     *
     * @return
     *      True on success, false on error (no more space).
     */
    bool addRegion(std::uint8_t regionId,
                   std::uint16_t regionAddressX,
                   std::uint16_t regionAddressY)
    {
        if (m_state != State::INCOMPLETE)
        {
            throw std::logic_error("Wrong state");
        }

        if (m_regionCount < m_regionRefs.size())
        {
            m_regionRefs[m_regionCount].m_regionId = regionId;
            m_regionRefs[m_regionCount].m_positionX = regionAddressX;
            m_regionRefs[m_regionCount].m_positionY = regionAddressY;
            ++m_regionCount;

            return true;
        }
        else
        {
            return false;
        }
    }

    /**
     * Returns number of defined regions.
     *
     * @return
     *      Number of regions.
     */
    std::size_t getRegionCount() const
    {
        return m_regionCount;
    }

    /**
     * Returns define region.
     *
     * @param index
     *      Index of region to get.
     *
     * @return
     *      Region descriptor.
     */
    const RegionReference& getRegion(std::size_t index) const
    {
        if (index >= m_regionCount)
        {
            throw std::range_error("index");
        }

        return m_regionRefs[index];
    }

    /**
     * Returs page PTS.
     *
     * @return
     *      PTS.
     */
    const StcTime& getPts() const
    {
        return m_pts;
    }

    /**
     * Returns page timeout.
     *
     * @return
     *      Timeout value.
     */
    std::uint32_t getTimeout() const
    {
        return m_timeout;
    }

private:
    /** Current state. */
    State m_state;

    /** Version. */
    std::uint8_t m_version;

    /** Region references. */
    std::array<RegionReference, MAX_SUPPORTED_REGIONS> m_regionRefs;

    /** Number of regions. */
    std::size_t m_regionCount;

    /** Page PTS. */
    StcTime m_pts;

    /** Page timeout. */
    std::uint32_t m_timeout;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_PAGE_HPP_*/
