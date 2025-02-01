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


#ifndef SUBTTXREND_GFX_WINDOW_HPP_
#define SUBTTXREND_GFX_WINDOW_HPP_

#include <memory>

#include "DrawContext.hpp"
#include "Types.hpp"
#include "KeyEventListener.hpp"

namespace subttxrend
{
namespace gfx
{

enum class DrawDirection
{
    LEFT_TO_RIGHT,
    RIGHT_TO_LEFT,
    TOP_BOTTOM,
    BOTTOM_TOP
};

/**
 * Window.
 */
class Window
{
public:
    /**
     * Constructor.
     */
    Window() = default;

    /**
     * Destructor.
     */
    virtual ~Window() = default;

    /**
     * Adds key event listener.
     *
     * @param listener
     *      Listener to add.
     */
    virtual void addKeyEventListener(KeyEventListener* listener) = 0;

    virtual void removeKeyEventListener(KeyEventListener* listener) = 0;

    /**
     * Returns window bounds.
     *
     * @note The position would be 0,0 and the size would be the
     *       current window size.
     *
     * @return
     *      Window bounds.
     */
    virtual Rectangle getBounds() const = 0;

    /**
     * Returns context for drawing.
     *
     * @return
     *      Context object.
     */
    virtual DrawContext& getDrawContext() = 0;

    /**
     * Gets preferred window size, that is, one most convenient for composer to draw.
     *
     * @return Preferred window size.
     */
    virtual Size getPreferredSize() const = 0;

    /**
     * Sets window size.
     *
     * @param newSize
     *      New window size.
     */
    virtual void setSize(const Size& newSize) = 0;
    virtual Size getSize() const = 0;

    /**
     * Sets windows visibility.
     *
     * @param visible
     *      True if window should be visible, false otherwise.
     */
    virtual void setVisible(bool visible) = 0;

    /**
     * Clears window.
     */
    virtual void clear() = 0;

    /**
     * Updates window context.
     *
     * This calls renders the window using the engine if needed.
     * Without calling this method changes done in window may will not
     * be visible.
     */
    virtual void update() = 0;

    /**
     * Sets text drawing direction
     *
     * @param dir
     *      Direction.
     */
    virtual void setDrawDirection(DrawDirection dir) = 0;

};

/**
 * Window pointer.
 */
using WindowPtr = std::shared_ptr<Window>;

} // namespace gfx
} // namespace subttxrend

#endif /*SUBTTXREND_GFX_WINDOW_HPP_*/
