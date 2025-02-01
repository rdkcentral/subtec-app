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


#ifndef SUBTTXREND_GFX_BLITTER_HPP_
#define SUBTTXREND_GFX_BLITTER_HPP_

#include <cstdint>
#include <cstring>
#include <algorithm>

#include <subttxrend/common/Logger.hpp>

#include "Pixmap.hpp"
#include "Types.hpp"

namespace subttxrend
{
namespace gfx
{

class Pixmap;
class ClutPixmap;
class ColorizedPixmap;

/**
 * Blitter that performs various graphical operations.
 */
class Blitter
{
public:
    /**
     * Draw position.
     */
    struct DrawPosition
    {
        /** Positioning mode. */
        enum class Mode
        {
            /** Absolute position. */
            ABSOLUTE,

            /** Centering. */
            CENTER,
        };

        /**
         * Constructor.
         *
         * @param mode
         *      Positioning mode.
         * @param x
         *      X coordinate.
         * @param y
         *      Y coordinate.
         */
        DrawPosition(Mode mode,
                     int x,
                     int y) :
                m_mode(mode),
                m_x(x),
                m_y(y)
        {
            // noop
        }

        /** Positioning mode. */
        Mode m_mode;

        /** X coordinate. */
        int m_x;

        /** Y coordinate. */
        int m_y;

        /** CENTERING positioning mode constant. */
        static const DrawPosition CENTER;
    };

    /**
     * Resize mode.
     */
    enum class ResizeMode
    {
        /** No resizing. */
        NO_RESIZE,
        /** Up/downscale (aspect ration not preserved). */
        UPDOWN,
        /** Up/downscale (aspect ration preserved). */
        UPDOWN_PRESERVE_ASPECT
    };

    /**
     * Rendering mode.
     */
    enum class RenderMode
    {
        /** Simple (single pixel copy from source to target). */
        SIMPLE,
        /** Smooth (up to 4 pixels mixed to get smoother look). */
        SMOOTH
    };

    /**
     * Clears the pixmap with transparent (0) color.
     *
     * @param dstPixmap
     *      Destination pixmap.
     */
    template<class DstPixmapType>
    static void clear(DstPixmapType& dstPixmap);

    /**
     * Fills rectangle.
     *
     * @param dstPixmap
     *      Destination pixmap.
     * @param dstRect
     *      Rectangle to fill.
     * @param value
     *      Fill color.
     */
    template<class DstPixmapType, class DstPixelType>
    static void fillRectangle(DstPixmapType& dstPixmap,
                              const Rectangle& dstRect,
                              DstPixelType value);

    /**
     * Writes (no mixing) pixmap.
     *
     * @param dstPixmap
     *      Destination pixmap.
     * @param srcPixmap
     *      Source pixmap.
     * @param srcRect
     *      Source rectangle.
     * @param dstRect
     *      Destination rectangle.
     */
    template<class SrcPixmapType, class DstPixmapType>
    static void write(DstPixmapType& dstPixmap,
                      const SrcPixmapType& srcPixmap,
                      const Rectangle& srcRect,
                      const Rectangle& dstRect);

    /**
     * Writes (with mixing) pixmap.
     *
     * @param dstPixmap
     *      Destination pixmap.
     * @param srcPixmap
     *      Source pixmap.
     * @param srcRect
     *      Source rectangle.
     * @param dstRect
     *      Destination rectangle.
     */
    template<class SrcPixmapType, class DstPixmapType>
    static void writeWithBlend(DstPixmapType& dstPixmap,
                               const SrcPixmapType& srcPixmap,
                               const Rectangle& srcRect,
                               const Rectangle& dstRect);

    /**
     * Writes (no mixing) pixmap.
     *
     * The pixamp is drawn at position 0,0 with no scalling.
     *
     * @param dstPixmap
     *      Destination pixmap.
     * @param srcPixmap
     *      Source pixmap.
     */
    template<class SrcPixmapType, class DstPixmapType>
    static void write(DstPixmapType& dstPixmap,
                      const SrcPixmapType& srcPixmap);

    /**
     * Writes (no mixing) pixmap.
     *
     * @param dstPixmap
     *      Destination pixmap.
     * @param srcPixmap
     *      Source pixmap.
     * @param position
     *      Position on which pixmap would be written.
     * @param resizeMode
     *      Resize mode to use.
     * @param renderMode
     *      Render mode to use.
     */
    template<class SrcPixmapType, class DstPixmapType>
    static void write(DstPixmapType& dstPixmap,
                      const SrcPixmapType& srcPixmap,
                      DrawPosition position,
                      ResizeMode resizeMode,
                      RenderMode renderMode);

private:
    /**
     * Calculates the rectangles.
     *
     * @param srcPixRect
     *      Source pixmap rectangle (bounds).
     * @param dstPixRect
     *      Destination pixmap rectangle (bounds).
     * @param position
     *      Operation position.
     * @param resizeMode
     *      Operation resize mode.
     * @param outSrcRect
     *      Calculated source rectangle.
     * @param outDstRect
     *      Calculated destination rectangle.
     */
    static void calculate(const Rectangle& srcPixRect,
                          const Rectangle& dstPixRect,
                          DrawPosition position,
                          ResizeMode resizeMode,
                          Rectangle& outSrcRect,
                          Rectangle& outDstRect);

    /**
     * Checks if rectangle could be used by operation.
     *
     * @param pixmap
     *      Pixmap.
     * @param rect
     *      Rectangle associated with pixmap.
     *
     * @return
     *      True if rectangle could be used to perform the operation,
     *      false otherwise.
     */
    template<class PixmapType>
    static bool checkRectangle(const PixmapType& pixmap,
                               const Rectangle& rect);

    /**
     * Copies line.
     *
     * @param srcPixmap
     *      Source pixmap.
     * @param srcRect
     *      Source rectangle.
     * @param dstPixmap
     *      Destination pixmap.
     * @param dstRect
     *      Destination rectangle.
     * @param line
     *      Line number (relative to rectangles!).
     * @param count
     *      Number of items (pixels) to copy.
     */
    template<class SrcPixmapType, class DstPixmapType>
    static void copyLine(const SrcPixmapType& srcPixmap,
                         const Rectangle& srcRect,
                         DstPixmapType& dstPixmap,
                         const Rectangle& dstRect,
                         int line,
                         std::size_t count);

    /**
     * Copies line with mixing.
     *
     * @param srcPixmap
     *      Source pixmap.
     * @param srcRect
     *      Source rectangle.
     * @param dstPixmap
     *      Destination pixmap.
     * @param dstRect
     *      Destination rectangle.
     * @param line
     *      Line number (relative to rectangles!).
     * @param count
     *      Number of items (pixels) to copy.
     */
    template<class SrcPixmapType, class DstPixmapType>
    static void alphaBlendLine(const SrcPixmapType& srcPixmap,
                               const Rectangle& srcRect,
                               DstPixmapType& dstPixmap,
                               const Rectangle& dstRect,
                               int line,
                               std::size_t count);

    /**
     * Performs bit blit operation.
     *
     * @note rectangles must be of equal sizes!
     *
     * @param srcPixmap
     *      Source pixmap.
     * @param srcRect
     *      Source rectangle.
     * @param dstPixmap
     *      Destination pixmap.
     * @param dstRect
     *      Destination rectangle.
     */
    template<class SrcPixmapType, class DstPixmapType>
    static void pixmapBitBlit(const SrcPixmapType& srcPixmap,
                              const Rectangle& srcRect,
                              DstPixmapType& dstPixmap,
                              const Rectangle& dstRect);

    /**
     * Performs bit blit operation with mixing.
     *
     * @note rectangles must be of equal sizes!
     *
     * @param srcPixmap
     *      Source pixmap.
     * @param srcRect
     *      Source rectangle.
     * @param dstPixmap
     *      Destination pixmap.
     * @param dstRect
     *      Destination rectangle.
     */
    template<class SrcPixmapType, class DstPixmapType>
    static void alphaBlendPixmaps(const SrcPixmapType& srcPixmap,
                                  const Rectangle& srcRect,
                                  DstPixmapType& dstPixmap,
                                  const Rectangle& dstRect);
    /**
     * Performs stretch blit operation (simple).
     *
     * @param srcPixmap
     *      Source pixmap.
     * @param srcRect
     *      Source rectangle.
     * @param dstPixmap
     *      Destination pixmap.
     * @param dstRect
     *      Destination rectangle.
     */
    template<class SrcPixmapType, class DstPixmapType>
    static void pixmapStretchBlitSimple(const SrcPixmapType& srcPixmap,
                                        const Rectangle& srcRect,
                                        DstPixmapType& dstPixmap,
                                        const Rectangle& dstRect);

    /**
     * Performs stretch blit operation (smooth - pixel mixing).
     *
     * @param srcPixmap
     *      Source pixmap.
     * @param srcRect
     *      Source rectangle.
     * @param dstPixmap
     *      Destination pixmap.
     * @param dstRect
     *      Destination rectangle.
     */
    template<class SrcPixmapType, class DstPixmapType>
    static void pixmapStretchBlitSmooth(const SrcPixmapType& srcPixmap,
                                        const Rectangle& srcRect,
                                        DstPixmapType& dstPixmap,
                                        const Rectangle& dstRect);

    /**
     * Performs blit operation.
     *
     * @param srcPixmap
     *      Source pixmap.
     * @param srcRect
     *      Source rectangle.
     * @param dstPixmap
     *      Destination pixmap.
     * @param dstRect
     *      Destination rectangle.
     * @param renderMode
     *      Rendering mode.
     */
    template<class SrcPixmapType, class DstPixmapType>
    static void pixmapBlit(const SrcPixmapType& srcPixmap,
                           const Rectangle& srcRect,
                           DstPixmapType& dstPixmap,
                           const Rectangle& dstRect,
                           Blitter::RenderMode renderMode);

    /** Logger. */
    static subttxrend::common::Logger m_logger;
};

template<class DstPixmapType>
inline void Blitter::clear(DstPixmapType& dstPixmap)
{
    auto width = dstPixmap.getWidth();
    auto height = dstPixmap.getHeight();

    for (int y = 0; y < height; ++y)
    {
        auto lineBeg = dstPixmap.getLine(y);
        auto lineEnd = lineBeg + width;
        auto lineLenBytes = reinterpret_cast<uint8_t*>(lineEnd.ptr())
                - reinterpret_cast<uint8_t*>(lineBeg.ptr());

        (void) std::memset(lineBeg.ptr(), 0x00, lineLenBytes);
    }
}

template<class DstPixmapType, class DstPixelType>
inline void Blitter::fillRectangle(DstPixmapType& dstPixmap,
                                   const Rectangle& dstRect,
                                   DstPixelType value)
{
    int width = dstRect.m_w;
    int safeX = dstPixmap.getWidth()/10;

    if (!checkRectangle(dstPixmap, dstRect))
    {
	if ((dstRect.m_w > dstPixmap.getWidth()) || (dstRect.m_x > (dstPixmap.getWidth() - dstRect.m_w)))
	    width = dstPixmap.getWidth() - safeX - dstRect.m_x;
        else
            return;
    }

    const int lx = dstRect.m_x + width;
    const int ly = dstRect.m_y + dstRect.m_h;

    for (int cy = dstRect.m_y; cy < ly; ++cy)
    {
        auto line = dstPixmap.getLine(cy) + dstRect.m_x;

        for (int cx = dstRect.m_x; cx < lx; ++cx)
        {
            *line = value;
            ++line;
        }
    }
}

template<class SrcPixmapType, class DstPixmapType>
inline void Blitter::write(DstPixmapType& dstPixmap,
                           const SrcPixmapType& srcPixmap,
                           const Rectangle& srcRect,
                           const Rectangle& dstRect)
{
    pixmapBlit(srcPixmap, srcRect, dstPixmap, dstRect, RenderMode::SMOOTH);
}

template<class SrcPixmapType, class DstPixmapType>
inline void Blitter::writeWithBlend(DstPixmapType& dstPixmap,
                                    const SrcPixmapType& srcPixmap,
                                    const Rectangle& srcRect,
                                    const Rectangle& dstRect)
{
    if (!checkRectangle(srcPixmap, srcRect))
    {
        return;
    }
    if (!checkRectangle(dstPixmap, dstRect))
    {
        return;
    }

    alphaBlendPixmaps(srcPixmap, srcRect, dstPixmap, dstRect);
}

template<class SrcPixmapType, class DstPixmapType>
inline void Blitter::write(DstPixmapType& dstPixmap,
                           const SrcPixmapType& srcPixmap)
{
    Rectangle rect
    { 0, 0, srcPixmap.getWidth(), srcPixmap.getHeight() };

    if (!checkRectangle(srcPixmap, rect))
    {
        return;
    }
    if (!checkRectangle(dstPixmap, rect))
    {
        return;
    }

    pixmapBitBlit(srcPixmap, rect, dstPixmap, rect);
}

template<class SrcPixmapType, class DstPixmapType>
inline void Blitter::write(DstPixmapType& dstPixmap,
                           const SrcPixmapType& srcPixmap,
                           DrawPosition position,
                           ResizeMode resizeMode,
                           RenderMode renderMode)
{
    const Rectangle srcPixRect
    { 0, 0, srcPixmap.getWidth(), srcPixmap.getHeight() };
    const Rectangle dstPixRect
    { 0, 0, dstPixmap.getWidth(), dstPixmap.getHeight() };

    Rectangle srcRect;
    Rectangle dstRect;

    calculate(srcPixRect, dstPixRect, position, resizeMode, srcRect, dstRect);

    pixmapBlit(srcPixmap, srcRect, dstPixmap, dstRect, renderMode);
}

template<class PixmapType>
inline bool Blitter::checkRectangle(const PixmapType& pixmap,
                                    const Rectangle& rect)
{
    // rectangle position or size is invalid
    if ((rect.m_x < 0) || (rect.m_y < 0) || (rect.m_w < 0) || (rect.m_h < 0))
    {
        m_logger.oserror(
                __LOGGER_FUNC__, " - invalid rectangle pos: ", rect.m_x, ' ', rect.m_y, ' ', rect.m_w, ' ', rect.m_h);
        return false;
    }

    // rectangle size empty
    if ((rect.m_w == 0) || (rect.m_h == 0))
    {
        // exit silently - nothing to be done.
        return false;
    }

    // rectangle size bigger than pixmap
    if ((rect.m_w > pixmap.getWidth()) || (rect.m_h > pixmap.getHeight()))
    {
        m_logger.oserror(__LOGGER_FUNC__,
                       " - invalid rectangle size: ",
                       rect.m_w,
                       'x',
                       rect.m_h,
                       " vs ",
                       pixmap.getWidth(),
                       'x',
                       pixmap.getHeight());
        return false;
    }

    // rectangle outside pixmap
    int remainingWidth = pixmap.getWidth() - rect.m_w;
    int remainingHeight = pixmap.getHeight() - rect.m_h;
    if ((rect.m_x > remainingWidth) || (rect.m_y > remainingHeight))
    {
        m_logger.oserror(__LOGGER_FUNC__,
                       " - invalid remaining size: ",
                       rect.m_x,
                       ',',
                       rect.m_y,
                       ',',
                       rect.m_w,
                       ',',
                       rect.m_h,
                       " vs ",
                       pixmap.getWidth(),
                       ',',
                       pixmap.getHeight());
        return false;
    }

    // valid
    return true;
}

template<class SrcPixmapType, class DstPixmapType>
inline void Blitter::copyLine(const SrcPixmapType& srcPixmap,
                              const Rectangle& srcRect,
                              DstPixmapType& dstPixmap,
                              const Rectangle& dstRect,
                              int line,
                              std::size_t count)
{
    auto srcLine = srcPixmap.getLine(line + srcRect.m_y) + srcRect.m_x;
    auto dstLine = dstPixmap.getLine(line + dstRect.m_y) + dstRect.m_x;

    for (std::size_t i = 0; i < count; ++i)
    {
        *dstLine = *srcLine;
        ++dstLine;
        ++srcLine;
    }
}

/**
 * Copies line.
 *
 * @param srcPixmap
 *      Source pixmap.
 * @param srcRect
 *      Source rectangle.
 * @param dstPixmap
 *      Destination pixmap.
 * @param dstRect
 *      Destination rectangle.
 * @param line
 *      Line number (relative to rectangles!).
 * @param count
 *      Number of items (pixels) to copy.
 */
template<>
inline void Blitter::copyLine<Pixmap, Pixmap>(const Pixmap& srcPixmap,
                                              const Rectangle& srcRect,
                                              Pixmap& dstPixmap,
                                              const Rectangle& dstRect,
                                              int line,
                                              std::size_t count)
{
    auto srcLine = srcPixmap.getLine(line + srcRect.m_y) + srcRect.m_x;
    auto dstLine = dstPixmap.getLine(line + dstRect.m_y) + dstRect.m_x;

    (void) std::memcpy(dstLine.ptr(), srcLine.ptr(),
            sizeof(PixelArgb8888) * count);
}

template<class SrcPixmapType, class DstPixmapType>
inline void Blitter::alphaBlendLine(const SrcPixmapType& srcPixmap,
                                    const Rectangle& srcRect,
                                    DstPixmapType& dstPixmap,
                                    const Rectangle& dstRect,
                                    int line,
                                    std::size_t count)
{
    auto srcLine = srcPixmap.getLine(line + srcRect.m_y) + srcRect.m_x;
    auto dstLine = dstPixmap.getLine(line + dstRect.m_y) + dstRect.m_x;
    PixelArgb8888 srcPix;
    PixelArgb8888 dstPix;
    uint8_t outAlpha;

    for (std::size_t i = 0; i < count; ++i)
    {
        srcPix = *srcLine;
        dstPix = *dstLine;

        // Blend Alpha component
        const uint8_t dstAlpha_1_minus_srcAlpha = dstPix.m_a * (255 - srcPix.m_a) / 255;
        outAlpha = srcPix.m_a + dstAlpha_1_minus_srcAlpha;

        if(outAlpha > 0)
        {
            dstPix.m_b = (srcPix.m_b * srcPix.m_a + dstPix.m_b * dstAlpha_1_minus_srcAlpha) / outAlpha;
            dstPix.m_g = (srcPix.m_g * srcPix.m_a + dstPix.m_g * dstAlpha_1_minus_srcAlpha) / outAlpha;
            dstPix.m_r = (srcPix.m_r * srcPix.m_a + dstPix.m_r * dstAlpha_1_minus_srcAlpha) / outAlpha;
        }
        else
        {
            dstPix.m_b = 0;
            dstPix.m_g = 0;
            dstPix.m_r = 0;
        }

        dstPix.m_a = outAlpha;
        *dstLine = dstPix;

        ++dstLine;
        ++srcLine;
    }
}

template<class SrcPixmapType, class DstPixmapType>
inline void Blitter::alphaBlendPixmaps(const SrcPixmapType& srcPixmap,
                                       const Rectangle& srcRect,
                                       DstPixmapType& dstPixmap,
                                       const Rectangle& dstRect)
{
    int copyW = srcRect.m_w;
    int copyH = srcRect.m_h;

    for (int y = 0; y < copyH; ++y)
    {
        alphaBlendLine(srcPixmap, srcRect, dstPixmap, dstRect, y, copyW);
    }
}

template<class SrcPixmapType, class DstPixmapType>
inline void Blitter::pixmapBitBlit(const SrcPixmapType& srcPixmap,
                                   const Rectangle& srcRect,
                                   DstPixmapType& dstPixmap,
                                   const Rectangle& dstRect)
{
    int copyW = srcRect.m_w;
    int copyH = srcRect.m_h;

    for (int y = 0; y < copyH; ++y)
    {
        copyLine(srcPixmap, srcRect, dstPixmap, dstRect, y, copyW);
    }
}

template<class SrcPixmapType, class DstPixmapType>
inline void Blitter::pixmapStretchBlitSimple(const SrcPixmapType& srcPixmap,
                                             const Rectangle& srcRect,
                                             DstPixmapType& dstPixmap,
                                             const Rectangle& dstRect)
{
    const int RATIO_SHIFT = 16;

    int wRatio = (srcRect.m_w << RATIO_SHIFT) / dstRect.m_w;
    int hRatio = (srcRect.m_h << RATIO_SHIFT) / dstRect.m_h;

    for (int dstY = 0; dstY < dstRect.m_h; ++dstY)
    {
        int32_t srcY = dstY;
        srcY *= hRatio;
        srcY >>= RATIO_SHIFT;

        auto dstLineStart = dstPixmap.getLine(dstY + dstRect.m_y) + dstRect.m_x;
        auto srcLineStart = srcPixmap.getLine(srcY + srcRect.m_y) + srcRect.m_x;

        auto dstPixel = dstLineStart;
        for (int dstX = 0; dstX < dstRect.m_w; ++dstX)
        {
            int32_t srcX = dstX;
            srcX *= wRatio;
            srcX >>= RATIO_SHIFT;

            auto srcPixel = srcLineStart + srcX;

            *dstPixel = *srcPixel;
            ++dstPixel;
        }
    }
}

template<class SrcPixmapType, class DstPixmapType>
inline void Blitter::pixmapStretchBlitSmooth(const SrcPixmapType& srcPixmap,
                                             const Rectangle& srcRect,
                                             DstPixmapType& dstPixmap,
                                             const Rectangle& dstRect)
{
    const int RATIO_SHIFT = 16;

    int wRatio = (srcRect.m_w << RATIO_SHIFT) / dstRect.m_w;
    int hRatio = (srcRect.m_h << RATIO_SHIFT) / dstRect.m_h;

    typename SrcPixmapType::ConstLineIterator srcLineStartPrev;

    for (int dstY = 0; dstY < dstRect.m_h; ++dstY)
    {
        int32_t srcY = dstY;
        srcY *= hRatio;
        srcY >>= RATIO_SHIFT;

        auto dstLineStart = dstPixmap.getLine(dstY + dstRect.m_y) + dstRect.m_x;
        auto srcLineStart = srcPixmap.getLine(srcY + srcRect.m_y) + srcRect.m_x;

        int srcXprev = -1;

        auto dstPixel = dstLineStart;
        for (int dstX = 0; dstX < dstRect.m_w; ++dstX)
        {
            int32_t srcX = dstX;
            srcX *= wRatio;
            srcX >>= RATIO_SHIFT;

            auto srcPixel = *(srcLineStart + srcX);

            int ta = srcPixel.m_a;
            int tr = srcPixel.m_r;
            int tg = srcPixel.m_g;
            int tb = srcPixel.m_b;
            int count = 1;

            if (srcXprev >= 0)
            {
                srcPixel = *(srcLineStart + srcXprev);

                ta += srcPixel.m_a;
                tr += srcPixel.m_r;
                tg += srcPixel.m_g;
                tb += srcPixel.m_b;
                ++count;
            }

            if (srcLineStartPrev)
            {
                srcPixel = *(srcLineStartPrev + srcX);

                ta += srcPixel.m_a;
                tr += srcPixel.m_r;
                tg += srcPixel.m_g;
                tb += srcPixel.m_b;
                ++count;

                if (srcXprev >= 0)
                {
                    srcPixel = *(srcLineStartPrev + srcXprev);

                    ta += srcPixel.m_a;
                    tr += srcPixel.m_r;
                    tg += srcPixel.m_g;
                    tb += srcPixel.m_b;
                    ++count;
                }
            }

            PixelArgb8888 srcPixelValue(ta / count, tr / count, tg / count,
                    tb / count);

            *dstPixel = srcPixelValue;
            ++dstPixel;

            srcXprev = srcX;
        }

        srcLineStartPrev = srcLineStart;
    }
}

template<class SrcPixmapType, class DstPixmapType>
inline void Blitter::pixmapBlit(const SrcPixmapType& srcPixmap,
                                const Rectangle& srcRect,
                                DstPixmapType& dstPixmap,
                                const Rectangle& dstRect,
                                Blitter::RenderMode renderMode)
{
    if (!checkRectangle(srcPixmap, srcRect))
    {
        return;
    }
    if (!checkRectangle(dstPixmap, dstRect))
    {
        return;
    }

    m_logger.ostrace(__LOGGER_FUNC__, " src: ", srcRect, " dst: ", dstRect);
    if ((srcRect.m_w == dstRect.m_w) && (srcRect.m_h == dstRect.m_h))
    {
        auto t = m_logger.timing("pixmapBitBlit");
        pixmapBitBlit(srcPixmap, srcRect, dstPixmap, dstRect);
    }
    else
    {
        m_logger.oswarning(__LOGGER_FUNC__, " src: ", srcRect, " dst: ", dstRect);
        if (renderMode == Blitter::RenderMode::SIMPLE)
        {
            auto t = m_logger.timing("pixmapStretchBlitSimple");
            pixmapStretchBlitSimple(srcPixmap, srcRect, dstPixmap, dstRect);
        }
        else if (renderMode == Blitter::RenderMode::SMOOTH)
        {
            auto t = m_logger.timing("pixmapStretchBlitSmooth");
            pixmapStretchBlitSmooth(srcPixmap, srcRect, dstPixmap, dstRect);
        }
        else
        {
            m_logger.oserror(__LOGGER_FUNC__, " - invalid render mode");
        }
    }
}

} // namespace gfx
} // namespace subttxrend

#endif                          // SUBTTXREND_GFX_BLITTER_HPP_
