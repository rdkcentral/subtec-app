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


#ifndef DVBSUBDECODER_DECODERCLIENT_HPP_
#define DVBSUBDECODER_DECODERCLIENT_HPP_

#include <cstdint>

#include "Types.hpp"

namespace dvbsubdecoder
{

/**
 * DVB subtitles decoder - client interface.
 */
class DecoderClient
{
public:
    /**
     * Constructor.
     */
    DecoderClient() = default;

    /**
     * Destructor.
     */
    virtual ~DecoderClient() = default;

    /**
     * Sets display and window sizes.
     *
     * Implementation should clear the entire screen.
     *
     * @param displayBounds
     *      Display rectangle.
     * @param windowBounds
     *      Window rectangle.
     */
    virtual void gfxSetDisplayBounds(const Rectangle& displayBounds,
                                     const Rectangle& windowBounds) = 0;

    /**
     * Draws bitmap.
     *
     * @param bitmap
     *      Bitmap to draw.
     * @param srcRect
     *      Source rectangle (bitmap section to draw).
     * @param dstRect
     *      Destination rectangle (in display coordinates).
     */
    virtual void gfxDraw(const Bitmap& bitmap,
                         const Rectangle& srcRect,
                         const Rectangle& dstRect) = 0;

    /**
     * Clears rectangle.
     *
     * @param rect
     *      Rectangle to clear (in display coordinates).
     */
    virtual void gfxClear(const Rectangle& rect) = 0;

    /**
     * Finishes drawing operations.
     *
     * @param rect
     *      Rectangle modified by drawing operations (in display coordinates).
     */
    virtual void gfxFinish(const Rectangle& rect) = 0;

    /**
     * Allocates graphics memory for bitmaps.
     *
     * @param size
     *      Size of the block to allocate in bytes.
     *
     * @return
     *      Allocated block or null if memory is not available.
     */
    virtual void* gfxAllocate(std::uint32_t size) = 0;

    /**
     * Releases graphics memory.
     *
     * @param block
     *      Block of memory allocated using gfxAllocate().
     */
    virtual void gfxFree(void* block) = 0;
};

} // namespace dvbsubdecoder

#endif /*DVBSUBDECODER_DECODERCLIENT_HPP_*/
