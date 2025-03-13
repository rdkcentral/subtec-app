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


#include "WindowImpl.hpp"

#include <subttxrend/common/Logger.hpp>

#include "ClutPixmap.hpp"
#include "FontStripImpl.hpp"
#include "ColorizedPixmap.hpp"

#include <uchar.h> // newer compilers would have <cuchar>

#include <cassert>
#include <cmath>
#include <iostream>

namespace subttxrend
{
namespace gfx
{

namespace
{

common::Logger g_logger("Gfx", "WindowImpl");

const gfx::Size DEFAULT_WINDOW_SIZE{1280, 720};

class NullEngineHooks : public EngineHooks
{
    virtual void requestRedraw() override {};
    virtual void forceRedraw() override {};
    virtual void lock() override {};
    virtual void unlock() override{};
};

static NullEngineHooks nullEngineHooks;

class HooksScopedLock
{
public:
    HooksScopedLock(EngineHooks* hooks) : m_hooks(hooks)
    {
        assert(hooks != nullptr);
        m_hooks->lock();
    }

    ~HooksScopedLock()
    {
        m_hooks->unlock();
    }

private:
    EngineHooks* m_hooks;
};

}

WindowImpl::WindowImpl() :
        m_visible(false),
        m_hooks(&nullEngineHooks),
        drawDir(DrawDirection::LEFT_TO_RIGHT),
        m_preferredSize{0, 0}
{
    g_logger.trace("%s", __func__);

    m_drawingSurface.reset(new Surface());
    m_readySurface.reset(new Surface());

    m_bgDrawingSurface.reset(new Surface());
    m_bgReadySurface.reset(new Surface());
}

WindowImpl::~WindowImpl()
{
    g_logger.trace("%s", __func__);
}

void WindowImpl::addKeyEventListener(KeyEventListener* listener)
{
    if (!listener)
    {
        throw std::invalid_argument("listener");
    }

    m_keyEventListeners.push_back(listener);
}

void WindowImpl::removeKeyEventListener(KeyEventListener* listener) {
    m_keyEventListeners.erase(std::remove(m_keyEventListeners.begin(), m_keyEventListeners.end(), listener),
                              m_keyEventListeners.end());
}

Rectangle WindowImpl::getBounds() const
{
    auto size = m_readySurface->getSize();

    return Rectangle
    { 0, 0, size.m_w, size.m_h };
}

DrawContext& WindowImpl::getDrawContext()
{
    return *this;
}

Size WindowImpl::getPreferredSize() const
{
    HooksScopedLock lock{m_hooks};

    gfx::Size returnedSize{DEFAULT_WINDOW_SIZE};

    if ((m_preferredSize.m_w != 0) && (m_preferredSize.m_h != 0))
    {
        returnedSize = m_preferredSize;
    }
    else
    {
        g_logger.error("%s preferred window size not set, using default w=%d h=%d",
            __func__,
            returnedSize.m_w,
            returnedSize.m_h);
    }

    return returnedSize;
}

Size WindowImpl::getSize() const
{
    return m_size;
}

void WindowImpl::setSize(const Size& newSize)
{
    g_logger.trace("%s w=%d h=%d", __func__, newSize.m_w, newSize.m_h);

    const PixelArgb8888 TRANSPARENT_COLOR(0x00, 0x00, 0x00, 0x00);

    HooksScopedLock lock{m_hooks};

    m_size = newSize;
    m_drawingSurface->resize(newSize.m_w, newSize.m_h, TRANSPARENT_COLOR);
    m_readySurface->resize(newSize.m_w, newSize.m_h, TRANSPARENT_COLOR);
#if BACKEND_TYPE == BACKEND_TYPE_EGL
    m_bgReadySurface->resize(newSize.m_w, newSize.m_h, TRANSPARENT_COLOR);
    m_bgDrawingSurface->resize(newSize.m_w, newSize.m_h, TRANSPARENT_COLOR);
#endif
    if (m_visible)
    {
        m_hooks->requestRedraw();
    }
}

void WindowImpl::setVisible(bool visible)
{
    g_logger.trace("%s visible=%d", __func__, visible ? 1 : 0);

    m_visible = visible;

    HooksScopedLock lock{m_hooks};
    if (m_visible)
    {
        m_hooks->requestRedraw();
    }
    else
    {
        // if force when going out of view, there may not be another chance to update
        // before being visible again
        m_hooks->forceRedraw();
    }
}

void WindowImpl::update()
{
    g_logger.trace("%s", __func__);

    HooksScopedLock lock{m_hooks};

    std::swap(m_drawingSurface, m_readySurface);
#if BACKEND_TYPE == BACKEND_TYPE_EGL
    std::swap(m_bgDrawingSurface, m_bgReadySurface);
#endif

    if (m_visible)
    {
        m_hooks->requestRedraw();
    }
}

void WindowImpl::clear()
{
    g_logger.trace("%s", __func__);

    HooksScopedLock lock{m_hooks};

    Blitter::clear(m_drawingSurface->getPixmap());
    Blitter::clear(m_bgDrawingSurface->getPixmap());
    m_hooks->requestRedraw();
}

void WindowImpl::setEngineHooks(EngineHooks* hooks)
{
    if (hooks)
    {
        m_hooks = hooks;
    }
    else
    {
        m_hooks = &nullEngineHooks;
    }
}

bool WindowImpl::isVisible() const
{
    return m_visible;
}

Pixmap& WindowImpl::getPixmap()
{
    g_logger.trace("%s", __func__);

    return m_readySurface->getPixmap();
}

Pixmap& WindowImpl::getBgPixmap()
{
    g_logger.trace("%s", __func__);

    return m_bgReadySurface->getPixmap();
}

void WindowImpl::processKeyEvent(const KeyEvent& event)
{
    const auto count = m_keyEventListeners.size();

    for (std::size_t i = 0; i < count; ++i)
    {
        m_keyEventListeners[i]->onKeyEvent(event);
    }
}

void WindowImpl::setPreferredSize(Size size)
{
    // only save the first notification, subsequent calls will report last set size
    if ((m_preferredSize.m_w == 0) && (m_preferredSize.m_h == 0))
    {
        m_preferredSize = size;
    }
}

void WindowImpl::setDrawDirection(DrawDirection dir)
{
    drawDir = dir;
}

#define VERBOSE_LOGGING 0

void WindowImpl::fillRectangle(ColorArgb color,
                               const Rectangle& rectangle)
{
#if VERBOSE_LOGGING
    g_logger.trace("%s - color=%02X%02X%02X%02X rect=%d,%d,%d,%d", __func__,
            color.m_a, color.m_r, color.m_g, color.m_b, rectangle.m_x,
            rectangle.m_y, rectangle.m_w, rectangle.m_h);
    auto t = g_logger.timing(__func__);
#endif

#if BACKEND_TYPE == BACKEND_TYPE_EGL
    Blitter::fillRectangle(m_bgDrawingSurface->getPixmap(), rectangle,
            PixelArgb8888(color.m_a, color.m_r, color.m_g, color.m_b));

    //clear the drawing surface in place where the bgDrawingSurface is filled.
    Blitter::fillRectangle(m_drawingSurface->getPixmap(), rectangle,
            PixelArgb8888(0, 0, 0, 0));
#else
    Blitter::fillRectangle(m_drawingSurface->getPixmap(), rectangle,
            PixelArgb8888(color.m_a, color.m_r, color.m_g, color.m_b));
#endif
}

void WindowImpl::drawUnderline(ColorArgb color,
                               const Rectangle& rectangle)
{
#if VERBOSE_LOGGING
    g_logger.trace("%s - color=%02X%02X%02X%02X rect=%d,%d,%d,%d", __func__,
            color.m_a, color.m_r, color.m_g, color.m_b, rectangle.m_x,
            rectangle.m_y, rectangle.m_w, rectangle.m_h);
    auto t = g_logger.timing(__func__);
#endif
    Blitter::fillRectangle(m_drawingSurface->getPixmap(), rectangle,
            PixelArgb8888(color.m_a, color.m_r, color.m_g, color.m_b));
}

void WindowImpl::drawPixmap(const ClutBitmap& bitmap,
                            const Rectangle& srcRect,
                            const Rectangle& dstRect)
{
#if VERBOSE_LOGGING
    g_logger.trace("%s - bitmap=%p(%dx%d) src=%d,%d,%d,%d dst=%d,%d,%d,%d",
            __func__, bitmap.m_pixels, bitmap.m_width, bitmap.m_height,
            srcRect.m_x, srcRect.m_y, srcRect.m_w, srcRect.m_h, dstRect.m_x,
            dstRect.m_y, dstRect.m_w, dstRect.m_h);

    auto t = g_logger.timing(__func__);
#endif
    ClutPixmap srcPixmap(bitmap.m_pixels, bitmap.m_width, bitmap.m_height,
            bitmap.m_stride, bitmap.m_clut, bitmap.m_clutSize);
    Blitter::write(m_drawingSurface->getPixmap(), srcPixmap, srcRect, dstRect);
}

void WindowImpl::drawBitmap(const Bitmap& bitmap, const Rectangle& dstRect)
{
#if VERBOSE_LOGGING
    auto t = g_logger.timing("drawBitmap 1");
#endif
    const Pixmap srcPixmap = {const_cast<std::uint8_t*>(bitmap.m_buffer.data()), int32_t(bitmap.m_width), int32_t(bitmap.m_height), bitmap.m_stride };
    {
#if VERBOSE_LOGGING
        auto t = g_logger.timing("drawBitmap 2");
#endif
        Rectangle srcRect = {0,0,int32_t(bitmap.m_width),int32_t(bitmap.m_height)};

        {
#if VERBOSE_LOGGING
            auto t = g_logger.timing("drawBitmap 3");
#endif
            Blitter::write(m_drawingSurface->getPixmap(), srcPixmap, srcRect, dstRect);
        }
    }
}

void WindowImpl::drawGlyph(const FontStripPtr& fontStrip,
                           std::int32_t glyphIndex,
                           const Rectangle& rect,
                           ColorArgb fgColor,
                           ColorArgb bgColor)
{
#if VERBOSE_LOGGING
    g_logger.trace("%s - index=%d rect=%d,%d,%d,%d", __func__, glyphIndex,
            rect.m_x, rect.m_y, rect.m_w, rect.m_h);
#endif

    if (!fontStrip)
    {
        throw std::invalid_argument("fontStrip");
    }

    // do not cast to shared pointer, no need to hold it
    FontStripImpl* fontImpl = static_cast<FontStripImpl*>(fontStrip.get());

    const auto& alphaPixmap = fontImpl->getPixmap();
    auto glyphRect = fontImpl->getGlyphRect(glyphIndex);

    if ((glyphRect.m_w > 0) && (glyphRect.m_h > 0))
    {
        ColorizedPixmap colorizedPixmap(alphaPixmap, fgColor, bgColor);

        Blitter::write(m_drawingSurface->getPixmap(), colorizedPixmap,
                glyphRect, rect);
    }
}

void WindowImpl::drawString(PrerenderedFont &font_,
                            const Rectangle &rectangle,
                            const std::vector<GlyphData> &glyphs,
                            const ColorArgb fgColor,
                            const ColorArgb bgColor,
                            int outlineSize,
                            int verticalOffset)
{
    PrerenderedFontImpl& font = static_cast<PrerenderedFontImpl&>(font_);
    float penX = 0;
    float penY = 0;
    float xStart = 0;
    float yStart = 0;

#if VERBOSE_LOGGING
    auto t = g_logger.timing(__func__);
#endif
    for (auto& glyphInfo : glyphs)
    {
        uint32_t glyphIndex = glyphInfo.glyphIndex;

        auto charInfo = font.getCharInfo(glyphIndex, outlineSize);

        if (charInfo)
        {
            int baselineOffset = 0;

            auto ascender = font.getFontAscender();
            auto descender = font.getFontDescender();

            if (ascender != 0)
            {
                const auto fontMinMax =  ascender + abs(descender);
                const auto fontSizeDiff = font.getFontHeight() - fontMinMax;
                baselineOffset = ascender + (fontSizeDiff / 2);
            }
            else
            {
                baselineOffset  = font.getFontHeight() - charInfo->descender;
            }

            const auto &alphaPixmap = charInfo->surface->getPixmap();

            Rectangle sourceRect = {
                    charInfo->atlasXOffset,
                    charInfo->atlasYOffset,
                    charInfo->bitmapWidth,
                    charInfo->bitmapHeight };

            if ((sourceRect.m_w > 0) && (sourceRect.m_h > 0))
            {

                ColorizedPixmap colorizedPixmap(alphaPixmap, fgColor, bgColor);
                float xOffset = 0;
                float yOffset = 0;

                switch(drawDir)
                {
                    case DrawDirection::LEFT_TO_RIGHT:
                    case DrawDirection::TOP_BOTTOM:
                        xOffset = glyphInfo.xOffset + charInfo->bitmapLeft;
                        yOffset = baselineOffset - charInfo->bitmapTop + verticalOffset;
                        xStart = rectangle.m_x;
                        yStart = rectangle.m_y;
                        break;
                    case DrawDirection::RIGHT_TO_LEFT:
                        xOffset = -(glyphInfo.xOffset + charInfo->bitmapWidth);
                        yOffset = baselineOffset - charInfo->bitmapTop + verticalOffset;
                        xStart = rectangle.m_x + rectangle.m_w;
                        yStart = rectangle.m_y;
                        break;
                    case DrawDirection::BOTTOM_TOP:
                        xOffset = glyphInfo.xOffset + charInfo->bitmapLeft;
                        yOffset = baselineOffset - charInfo->bitmapTop + verticalOffset;
                        xStart = rectangle.m_x;
                        yStart = rectangle.m_y + rectangle.m_h;
                        break;
                }

#if VERBOSE_LOGGING
                g_logger.info("%s start: x=%f y=%f offset: x=%f y=%f baselineoffset=%d bitmap: height=%d top=%d",
                                 __LOGGER_FUNC__,
                                 xStart,
                                 yStart,
                                 xOffset,
                                 yOffset,
                                 baselineOffset,
                                 charInfo->bitmapHeight,
                                 charInfo->bitmapTop);
#endif // VERBOSE_LOGGING

                Rectangle destinationRect = {
                        static_cast<int>(std::ceil(xStart + penX + xOffset)),
                        static_cast<int>(std::ceil(yStart + penY + yOffset)),
                        sourceRect.m_w,
                        sourceRect.m_h };
#if BACKEND_TYPE == BACKEND_TYPE_EGL
                Blitter::write(m_drawingSurface->getPixmap(), colorizedPixmap, sourceRect, destinationRect);
#else
                Blitter::writeWithBlend(m_drawingSurface->getPixmap(), colorizedPixmap, sourceRect, destinationRect);
#endif
            }

            switch(drawDir)
            {
                case DrawDirection::LEFT_TO_RIGHT:
                    penX += glyphInfo.advanceX;
                    break;
                case DrawDirection::RIGHT_TO_LEFT:
                    penX -= glyphInfo.advanceX;
                    break;
                case DrawDirection::TOP_BOTTOM:
                    penY += font.getFontHeight();
                    break;
                case DrawDirection::BOTTOM_TOP:
                    penY -= font.getFontHeight();
                    break;
            }
        }
        else
        {
            // shouldn't happen; oh well ...
            g_logger.warning("didn't find glyph for index %d", glyphIndex);
        }
    }
}

} // namespace gfx
} // namespace subttxrend
