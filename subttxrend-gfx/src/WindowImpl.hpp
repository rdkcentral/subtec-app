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


#ifndef SUBTTXREND_GFX_WINDOW_IMPL_HPP_
#define SUBTTXREND_GFX_WINDOW_IMPL_HPP_

#include <vector>

#include "Window.hpp"
#include "Surface.hpp"
#include "DrawContext.hpp"
#include "EngineHooks.hpp"
#include "PrerenderedFontImpl.hpp"

namespace subttxrend
{
namespace gfx
{

class Pixmap;
class ColorizedPixmap;

/**
 * Window - implementation.
 */
class WindowImpl : public Window, private DrawContext
{
public:
    /**
     * Constructor.
     */
    WindowImpl();

    /**
     * Destructor.
     */
    virtual ~WindowImpl();

    /** @copydoc Window::addKeyEventListener */
    virtual void addKeyEventListener(KeyEventListener* listener) override;

    virtual void removeKeyEventListener(KeyEventListener* listener) override;

    /** @copydoc Window::getBounds */
    virtual Rectangle getBounds() const override;

    /** @copydoc Window::getDrawContext */
    virtual DrawContext& getDrawContext() override;

    /** @copydoc Window::getPreferredSize */
    virtual Size getPreferredSize() const override;

    /** @copydoc Window::setSize */
    virtual void setSize(const Size& newSize) override;
    virtual Size getSize() const override;


    /** @copydoc Window::setVisible */
    virtual void setVisible(bool visible) override;

    /** @copydoc Window::clear */
    virtual void clear() override;

    /** @copydoc Window::update */
    virtual void update() override;

    /**
     * Sets engine hooks.
     *
     * @param hooks
     *      Hooks to set (nullptr to unset).
     */
    void setEngineHooks(EngineHooks* hooks);

    /**
     * Checks if window is visible.
     *
     * @return
     *      True if visible, false otherwise.
     */
    bool isVisible() const;

    /**
     * Returns window pixmap.
     *
     * @return
     *      Pixmap.
     */
    Pixmap& getPixmap();
    
    /**
     * Returns window background pixmap.
     *
     * @return
     *      Pixmap.
     */
    Pixmap& getBgPixmap();

    /**
     * Processes key event.
     *
     * Dispatches the event to registered listeners.
     *
     * @param event
     *      Event descriptor.
     */
    void processKeyEvent(const KeyEvent& event);

    /**
     * Sets preferred window size that is most suitable for composer.
     *
     * @param size
     *      Preferred window size.
     */
    void setPreferredSize(Size size);

    /**
     * Sets text drawing direction
     *
     * @param dir
     *      Direction.
     */
    void setDrawDirection(DrawDirection dir) override;

private:
    virtual void fillRectangle(ColorArgb color,
                               const Rectangle& rectangle) override;

    virtual void drawUnderline(ColorArgb color,
                               const Rectangle& rectangle) override;

    virtual void drawPixmap(const ClutBitmap& bitmap,
                            const Rectangle& srcRect,
                            const Rectangle& dstRect) override;

    virtual void drawGlyph(const FontStripPtr& fontStrip,
                           std::int32_t glyphIndex,
                           const Rectangle& rect,
                           ColorArgb fgColor,
                           ColorArgb bgColor) override;

    virtual void drawString(PrerenderedFont &font,
                            const Rectangle &rect,
                            const std::vector<GlyphData> &glyphs,
                            const ColorArgb fgColor,
                            const ColorArgb bgColor,
                            int outlineSize = 0,
                            int verticalOffset = 0) override;

    virtual void drawBitmap(const Bitmap& bitmap,const Rectangle& dstRect) override;

    /** Window visiblity flag. */
    bool m_visible;

    /** Engine hooks. */
    EngineHooks* m_hooks;

    /** Collection of registered key listeners. */
    std::vector<KeyEventListener*> m_keyEventListeners;

    /** Surface for drawing. */
    std::unique_ptr<Surface> m_drawingSurface;

    /** Surface for rendering (with contents ready). */
    std::unique_ptr<Surface> m_readySurface;

    /** Surface for drawing background */
    std::unique_ptr<Surface> m_bgDrawingSurface;

    /** Surface for rendering background */
    std::unique_ptr<Surface> m_bgReadySurface;

    /** Store drawing direction for non-trivial scenarios */
    DrawDirection drawDir;

    /** Suggested window size as requested by backend. */
    Size m_preferredSize;
    Size m_size;
};

/**
 * Window implementation pointer.
 */
using WindowImplPtr = std::shared_ptr<WindowImpl>;

} // namespace gfx
} // namespace subttxrend

#endif /*SUBTTXREND_GFX_WINDOW_IMPL_HPP_*/
