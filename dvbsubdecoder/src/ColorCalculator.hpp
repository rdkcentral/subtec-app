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


#ifndef DVBSUBDECODER_COLORCALCULATOR_HPP_
#define DVBSUBDECODER_COLORCALCULATOR_HPP_

#include <cstdint>

namespace dvbsubdecoder
{

/**
 * Y/Cr/Cb/T Color.
 */
struct ColorYCrCbT
{
    /** Color component - Y. */
    std::uint8_t m_y;
    /** Color component - Cr. */
    std::uint8_t m_cr;
    /** Color component - Cb. */
    std::uint8_t m_cb;
    /** Color component - Y. */
    std::uint8_t m_t;
};

/**
 * A/R/G/B Color.
 */
struct ColorARGB
{
    /**
     * Converts representation to uint32 integer.
     *
     * @return
     *      Encoded color value.
     */
    std::uint32_t toUint32() const
    {
        std::uint32_t a = m_a;
        std::uint32_t r = m_r;
        std::uint32_t g = m_g;
        std::uint32_t b = m_b;

        return (a << 24) | (r << 16) | (g << 8) | (b << 0);
    }

    /** Color component - A. */
    std::uint8_t m_a;
    /** Color component - R. */
    std::uint8_t m_r;
    /** Color component - G. */
    std::uint8_t m_g;
    /** Color component - B. */
    std::uint8_t m_b;
};

/**
 * Utility for color calculcations.
 */
class ColorCalculator
{
public:
    /**
     * Converts YCrCbT color to ARGB.
     *
     * @param ycrcbt
     *      Color to convert.
     *
     * @return
     *      Converted color value.
     */
    ColorARGB toARGB(const ColorYCrCbT ycrcbt);
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_COLORCALCULATOR_HPP_*/
