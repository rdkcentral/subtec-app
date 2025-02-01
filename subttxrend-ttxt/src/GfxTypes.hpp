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


#ifndef SUBTTXREND_TTXT_GFX_TYPES_HPP_
#define SUBTTXREND_TTXT_GFX_TYPES_HPP_

#include <cstdint>
#include <string>
#include <subttxrend/gfx/Types.hpp>

namespace subttxrend
{
namespace ttxt
{

/** Zoom mode. */
enum class ZoomMode
{
    /** Nothing is zoomed. */
    NONE,
    /** Top part of page is zoomed. */
    TOP,
    /** Bottom part of page is zoomed. */
    BOTTOM
};

/** Size. */
using Size = gfx::Size;

/**
 * Rectangle.
 */
using Rect = gfx::Rectangle;

/**
 * Font information.
 */
struct FontInfo
{
    /** Font name. */
    std::string m_name;
    /** Glyph (block) size. */
    Size m_glyphSize;
    /** Font character size. */
    Size m_charSize;
};

/**
 * Rows rangel.
 */
struct RowRange
{
    /**
     * Constructor.
     */
    RowRange() :
            m_first(0),
            m_count(0)
    {
        // noop
    }

    /**
     * Constructor.
     *
     * @param first
     *      First row index.
     * @param count
     *      Number of rows.
     */
    RowRange(std::uint32_t first,
             std::uint32_t count) :
            m_first(first),
            m_count(count)
    {
        // noop
    }

    /** First row index. */
    std::uint32_t m_first;
    /** Number of rows. */
    std::uint32_t m_count;
};

} // namespace ttxt
} // namespace subttxrend

#endif /*SUBTTXREND_TTXT_GFX_TYPES_HPP_*/
