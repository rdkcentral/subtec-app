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


#ifndef DVBSUBDECODER_PIXELWRITER_HPP_
#define DVBSUBDECODER_PIXELWRITER_HPP_

#include <cstdint>

#include "Pixmap.hpp"

namespace dvbsubdecoder
{

/**
 * Writer for object pixels.
 *
 * Writes decoded object pixels to region's pixmap.
 */
class PixelWriter
{
public:
    /**
     * Constructor.
     *
     * @param nonModifyingColourFlag
     *      Non-modifying colour mode flag.
     * @param depth
     *      Region depth (number of bits per pixel).
     * @param pixmap
     *      Pixmap to store the pixels.
     * @param startX
     *      Start position (X coordinate).
     * @param startY
     *      Start position (Y coordinate).
     */
    PixelWriter(bool nonModifyingColourFlag,
                std::uint8_t depth,
                Pixmap& pixmap,
                std::int32_t startX,
                std::int32_t startY);

    /**
     * Returns pixmap depth.
     *
     * @return
     *      Number of bits per pixel.
     */
    std::uint8_t getDepth() const
    {
        return m_depth;
    }

    /**
     * Sets pixmap pixels.
     *
     * @param pixelCode
     *      Code of the pixel(s) to write (CLUT index).
     *      Shall be in range defined by pixmap depth.
     * @param count
     *      Number of pixels to store.
     */
    void setPixels(std::uint8_t pixelCode,
                   std::uint32_t count);

    /**
     * Ends current line.
     *
     * After calling this method the writer position is set to start X position
     * two lines below current line (top and bottom interlace lines are
     * written separately).
     */
    void endLine();

private:
    /**
     * Initializes the current line for writing.
     */
    void initLine();

    /** Non modifying colour mode flag. */
    const bool m_nonModifyingColourFlag;

    /** Region's depth. */
    const std::uint8_t m_depth;

    /** Pixmap to which pixels are written. */
    Pixmap& m_pixmap;

    /** Start position in line (X coordinate). */
    std::int32_t m_startX;
    /** Current line (Y coordinate). */
    std::int32_t m_currY;

    /** Pointer to current pixel. */
    std::uint8_t* m_currentPtr;
    /** Pointer to end of current line. */
    std::uint8_t* m_lineEndPtr;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_PIXELWRITER_HPP_*/
