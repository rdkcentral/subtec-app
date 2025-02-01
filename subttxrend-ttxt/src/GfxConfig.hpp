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


#ifndef SUBTTXREND_TTXT_GFX_CONFIG_HPP_
#define SUBTTXREND_TTXT_GFX_CONFIG_HPP_

#include <subttxrend/common/ConfigProvider.hpp>

#include "GfxTypes.hpp"

namespace subttxrend
{
namespace ttxt
{

/**
 * Configuration wrapper.
 */
class GfxConfig
{
public:
    /**
     * Constructor.
     */
    GfxConfig() :
            m_flashPeriodMs(),
            m_bgAlpha()
    {
        // noop
    }

    /**
     * Initializes the configuration.
     *
     * Fills the configuration with data from the given provider.
     *
     * @param configProvider
     *      Provider to use.
     */
    void init(const common::ConfigProvider* configProvider);

    /**
     * Returns window size in pixels.
     *
     * @return
     *      Window size.
     */
    Size getWindowSizePixels() const
    {
        return m_windowSizePixels;
    }

    /**
     * Returns grid cell size in pixels.
     *
     * @return
     *      Grid cell size in pixels.
     */
    Size getGridCellSizePixels() const
    {
        return m_gridCellSizePixels;
    }

    /**
     * Returns G0/G2 font information.
     *
     * @return
     *      Font information.
     */
    const FontInfo& getFontInfoG0G2() const
    {
        return m_fontInfoG0G2;
    }

    /**
     * Returns flash period in milliseconds.
     *
     * @return
     *      Flash period.
     */
    std::int32_t getFlashPeriodMs() const
    {
        return m_flashPeriodMs;
    }

    /**
     * Returns default background alpha.
     *
     * @return
     *      Background alpha.
     */
    std::uint8_t getDefaultBackgroundAlpha() const
    {
        return m_bgAlpha;
    }

private:
    /** Window size in pixels. */
    Size m_windowSizePixels;

    /** Grid cell size in pixels. */
    Size m_gridCellSizePixels;

    /** G0/G2 font information. */
    FontInfo m_fontInfoG0G2;

    /** Flash period. */
    std::int32_t m_flashPeriodMs;

    /** Default background alpha. */
    std::uint8_t m_bgAlpha;
};

} // namespace ttxt
} // namespace subttxrend

#endif /*SUBTTXREND_TTXT_GFX_CONFIG_HPP_*/
