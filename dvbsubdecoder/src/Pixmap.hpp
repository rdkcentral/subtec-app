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


#ifndef DVBSUBDECODER_PIXMAP_HPP_
#define DVBSUBDECODER_PIXMAP_HPP_

#include <cstdint>

namespace dvbsubdecoder
{

/**
 * Region object pixmap.
 */
class Pixmap
{
public:
    /**
     * Constructor.
     */
    Pixmap();

    /**
     * Destructor.
     */
    ~Pixmap() = default;

    /**
     * Returns width.
     *
     * @return
     *      Width in pixels.
     */
    std::int32_t getWidth() const
    {
        return m_width;
    }

    /**
     * Returns height.
     *
     * @return
     *      Height in pixels.
     */
    std::int32_t getHeight() const
    {
        return m_height;
    }

    /**
     * Returns pointer to pixmap data.
     *
     * @return
     *      Pointer to pixmap data.
     */
    std::uint8_t* getBuffer() const
    {
        return m_buffer;
    }

    /**
     * Resets the pixmap.
     *
     * The pixmap is empty and has no memory associates after reset.
     */
    void reset();

    /**
     * Initializes the pixmap.
     *
     * @param width
     *      Width in pixels.
     * @param height
     *      Height in pixels.
     * @param buffer
     *      Buffer to store pixels.
     */
    void init(std::int32_t width,
              std::int32_t height,
              std::uint8_t* buffer);

    /**
     * Returns pointer to a single line.
     *
     * @param line
     *      Line number. Shall be in 0..height-1 range.
     *
     * @return
     *      Pointer to line pixels.
     */
    std::uint8_t* getLine(std::int32_t line);

    /**
     * Clears the pixmap.
     *
     * The pixmap is filled with given color.
     *
     * @param colorIndex
     *      CLUT color index.
     */
    void clear(std::uint8_t colorIndex);

private:
    /** Current width in pixels. */
    std::int32_t m_width;

    /** Current height in pixels. */
    std::int32_t m_height;

    /** Pointer to pixels data. */
    std::uint8_t* m_buffer;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_PIXMAP_HPP_*/
