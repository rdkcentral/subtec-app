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


#ifndef SUBTTXREND_GFX_ENGINE_HPP_
#define SUBTTXREND_GFX_ENGINE_HPP_

#include <memory>

#include "Window.hpp"
#include "FontStrip.hpp"

namespace subttxrend
{
namespace gfx
{

/**
 * Graphics engine.
 *
 * Engine is the entry point for graphics module allowing window construction,
 * management and rendering.
 *
 * It also acts as a compositor for attached windows.
 *
 * @note Most methods throws std::exception-based classes on errors.
 */
class Engine
{
public:
    /**
     * Constructor.
     */
    Engine() = default;

    /**
     * Destructor.
     */
    virtual ~Engine() = default;

    /**
     * Initializes the engine.
     */
    virtual void init(const std::string &displayName = {}) = 0;

    /**
     * Shutdowns the engine.
     */
    virtual void shutdown() = 0;

    /**
     * Executes engine actions.
     */
    virtual void execute() = 0;

    /**
     * Creates the window.
     *
     * @note The window is not managed be engine until it is attached.
     *
     * @return
     *      Created window.
     */
    virtual WindowPtr createWindow() = 0;

    /**
     * Creates new font strip.
     *
     * @param glyphSize
     *      Size of single font glyph.
     * @param glyphCount
     *      Number of glyphs.
     *
     * @return
     *      Created font strip.
     */
    virtual FontStripPtr createFontStrip(const Size& glyphSize,
                                         const std::size_t glyphCount) = 0;

    /**
     * Attaches window to engine.
     *
     * @param window
     *      Window to be attached.
     */
    virtual void attach(WindowPtr window) = 0;

    /**
     * Detaches window from engine.
     *
     * @param window
     *      Window to be detached.
     */
    virtual void detach(WindowPtr window) = 0;

#ifdef __APPLE__
    /**
     * @brief Start blocking application window (block this thread)
     */
    virtual void startBlockingApplicationWindow() = 0;
#endif
};

/**
 * Graphics engine pointer.
 */
using EnginePtr = std::shared_ptr<Engine>;

} // namespace gfx
} // namespace subttxrend

#endif /*SUBTTXREND_GFX_ENGINE_HPP_*/
