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


#ifndef SUBTTXREND_TTXT_RENDERER_HPP_
#define SUBTTXREND_TTXT_RENDERER_HPP_

#include <cstdint>
#include <subttxrend/common/ConfigProvider.hpp>
#include <subttxrend/gfx/Engine.hpp>

#include "TimeSource.hpp"

namespace subttxrend
{
namespace ttxt
{

/**
 * Renderer for Teletext.
 */
class Renderer
{
public:
    /**
     * Constructor.
     */
    Renderer() = default;

    /**
     * Destructor.
     */
    virtual ~Renderer() = default;

    /**
     * Initializes the renderer.
     *
     * @param gfxWindow
     *      Window to draw on.
     * @param gfxEngine
     *      Graphics engine to use.
     * @param configProvider
     *      Configuration provider.
     * @param timeSource
     *      Time source to use.
     *
     * @return
     *      True on success, false on error.
     */
    virtual bool init(gfx::Window* gfxWindow,
                      const gfx::EnginePtr& gfxEngine,
                      const common::ConfigProvider* configProvider,
                      TimeSource* timeSource) = 0;

    /**
     * Performs shutdown.
     */
    virtual void shutdown() = 0;

    /**
     * Performs data processing.
     */
    virtual void processData() = 0;

    /**
     * Adds PES packet with subtitles data.
     *
     * @param buffer
     *      Buffer with PES data.
     * @param length
     *      Length of PES data.
     *
     * @return
     *      True on success, false on error (subtitles stopped, no more
     *      space in buffer, etc.).
     */
    virtual bool addPesPacket(const void* buffer,
                              const std::uint16_t length) = 0;

    /**
     * Stops subtitles rendering.
     *
     * @return
     *      True on success, false on error.
     */
    virtual bool stop() = 0;

    /**
     * Checks if subtitles rendering is enabled.
     *
     * @return
     *      True if started, false if stopped.
     */
    virtual bool isStarted() const = 0;

    /**
     * Mutes the subtitles rendering.
     *
     * Muted subtitles are decoded but not displayed.
     */
    virtual void mute() = 0;

    /**
     * Unmutes the subtitles rendering.
     */
    virtual void unmute() = 0;

    /**
     * Checks if subtitles rendering is muted.
     *
     * @return
     *      True if started, false if muted.
     */
    virtual bool isMuted() const = 0;
};

} // namespace ttxt
} // namespace subttxrend

#endif /*SUBTTXREND_TTXT_RENDERER_HPP_*/
