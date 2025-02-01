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


#ifndef DVBSUBDECODER_TYPES_HPP_
#define DVBSUBDECODER_TYPES_HPP_

#include <cstdint>

namespace dvbsubdecoder
{

/**
 * Specification version.
 */
enum class Specification
{
    /** Version 1.2.1. */
    VERSION_1_2_1,

    /** Version 1.3.1. */
    VERSION_1_3_1,
};

/**
 * STC time type.
 */
enum class StcTimeType
{
    /** High 32 bits of 33-bit (base) STC. */
    HIGH_32,//!< HIGH_32

    /** Low 32 bits of 33-bit (base) STC. */
    LOW_32, //!< LOW_32
};

/**
 * STC time.
 */
struct StcTime
{
    /**
     * Constructor.
     *
     * Sets time to type=HIGH, value=0.
     */
    StcTime() :
            m_type(StcTimeType::HIGH_32),
            m_time(0U)
    {
        // noop
    }

    /**
     * Constructor.
     *
     * @param type
     *      Type of STC time.
     * @param time
     *      Time value.
     */
    StcTime(StcTimeType type,
            std::uint32_t time) :
            m_type(type),
            m_time(time)
    {
        // noop
    }

    /** Type of STC. */
    StcTimeType m_type;

    /** Time value. */
    std::uint32_t m_time;
};

/**
 * Rectangle.
 */
struct Rectangle
{
    /** Left position (inside the rectangle). */
    std::int32_t m_x1;

    /** Top position (inside the rectangle). */
    std::int32_t m_y1;

    /** Right position (outside the rectangle). */
    std::int32_t m_x2;

    /** Bottom position (outside the rectangle). */
    std::int32_t m_y2;
};

/**
 * Bitmap.
 */
struct Bitmap
{
    /** Width. */
    std::uint32_t m_width;
    /** Height. */
    std::uint32_t m_height;
    /** Pixels buffer. */
    const std::uint8_t* m_pixels;
    /** Colour lookup table. */
    const std::uint32_t* m_clut;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_TYPES_HPP_*/
