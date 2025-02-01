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


#ifndef SUBTTXREND_DVBSUB_DECODER_CLIENT_GFX_RENDERER_HPP_
#define SUBTTXREND_DVBSUB_DECODER_CLIENT_GFX_RENDERER_HPP_

#include <memory>

#include <dvbsubdecoder/DecoderClient.hpp>

#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/gfx/Window.hpp>

namespace subttxrend
{
namespace dvbsub
{

class TimeSource;

/**
 * Decoder client rendering the subtitle graphics.
 */
class DecoderClientGfxRenderer : private dvbsubdecoder::DecoderClient
{
public:
    /**
     * Constructor.
     */
    DecoderClientGfxRenderer();

    /**
     * Destructor.
     */
    virtual ~DecoderClientGfxRenderer();

    /**
     * Returns decoder client interface.
     *
     * @return
     *      Decoder client interface.
     */
    dvbsubdecoder::DecoderClient& getDecoderClient();

    /**
     * Initialize GFX elements.
     *
     * @param gfxWindow
     *      Graphics engine to use.
     */
    void gfxInit(gfx::Window* gfxWindow);

    /**
     * Releases GFX elements.
     */
    void gfxShutdown();

    /**
     * Shows GFX.
     */
    void gfxShow();

    /**
     * Hides GFX.
     */
    void gfxHide();

private:
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
    virtual void gfxSetDisplayBounds(const dvbsubdecoder::Rectangle& displayBounds,
                                     const dvbsubdecoder::Rectangle& windowBounds)
                                             override;

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
    virtual void gfxDraw(const dvbsubdecoder::Bitmap& bitmap,
                         const dvbsubdecoder::Rectangle& srcRect,
                         const dvbsubdecoder::Rectangle& dstRect) override;

    /**
     * Clears rectangle.
     *
     * @param rect
     *      Rectangle to clear (in display coordinates).
     */
    virtual void gfxClear(const dvbsubdecoder::Rectangle& rect) override;

    /**
     * Finishes drawing operations.
     *
     * @param rect
     *      Rectangle modified by drawing operations (in display coordinates).
     */
    virtual void gfxFinish(const dvbsubdecoder::Rectangle& rect) override;

    /**
     * Allocates graphics memory for bitmaps.
     *
     * @param size
     *      Size of the block to allocate in bytes.
     *
     * @return
     *      Allocated block or null if memory is not available.
     */
    virtual void* gfxAllocate(std::uint32_t size) override;

    /**
     * Releases graphics memory.
     *
     * @param block
     *      Block of memory allocated using gfxAllocate().
     */
    virtual void gfxFree(void* block) override;

    /** GFX elements - window. */
    gfx::Window* m_gfxWindow;
};

} // namespace dvbsub
} // namespace subttxrend

#endif /*SUBTTXREND_DVBSUB_DECODER_CLIENT_GFX_RENDERER_HPP_*/
