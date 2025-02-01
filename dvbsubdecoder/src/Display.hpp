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


#ifndef DVBSUBDECODER_DISPLAY_HPP_
#define DVBSUBDECODER_DISPLAY_HPP_

#include "Types.hpp"
#include "Consts.hpp"

namespace dvbsubdecoder
{

/**
 * Display related data.
 *
 * Holds display and window definition.
 * If display bounds were not defined in stream then SD resolution is used
 * as defined in specification.
 * If window bounds were not defined then they are assumed to be equal to
 * display.
 */
class Display
{
public:
    /** Default display size (SD resolution). */
    static const std::int32_t DEFAULT_SD_WIDTH = 720;

    /** Default display size (SD resolution). */
    static const std::int32_t DEFAULT_SD_HEIGHT = 576;

    /**
     * Constructor.
     *
     * Resets display object.
     */
    Display()
    {
        reset();
    }

    /**
     * Sets display definition.
     *
     * @param version
     *      Display definition version.
     * @param displayBounds
     *      Display bounds.
     * @param windowBounds
     *      Window bounds.
     */
    void set(std::uint8_t version,
             const Rectangle& displayBounds,
             const Rectangle& windowBounds)
    {
        m_version = version;
        m_displayBounds = displayBounds;
        m_windowBounds = windowBounds;
    }

    /**
     * Resets display object.
     *
     * Sets version to INVALID and bounds to default size.
     */
    void reset()
    {
        m_version = INVALID_VERSION;
        m_displayBounds = { 0, 0, DEFAULT_SD_WIDTH, DEFAULT_SD_HEIGHT };
        m_windowBounds = m_displayBounds;
    }

    /**
     * Returns version.
     *
     * @return
     *      Current version (may be invalid).
     */
    std::uint8_t getVersion() const
    {
        return m_version;
    }

    /**
     * Returns display bounds.
     *
     * @return
     *      Display bounds rectangle.
     */
    const Rectangle& getDisplayBounds() const
    {
        return m_displayBounds;
    }

    /**
     * Returns window bounds.
     *
     * @return
     *      Window bounds rectangle.
     */
    const Rectangle& getWindowBounds() const
    {
        return m_windowBounds;
    }

private:
    /** Current version, may be INVALID_VERSION. */
    std::uint8_t m_version;

    /** Display bounds. */
    Rectangle m_displayBounds;

    /** Window bounds. */
    Rectangle m_windowBounds;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_DISPLAY_HPP_*/
