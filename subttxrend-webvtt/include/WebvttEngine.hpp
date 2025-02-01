/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
*/

#pragma once

#include <cstdint>
#include <memory>
#include <chrono>

#include <subttxrend/common/ConfigProvider.hpp>
#include <subttxrend/gfx/Engine.hpp>

namespace subttxrend
{
namespace webvttengine
{

class WebVTTAttributes;

class WebvttEngine
{
public:

    /**
     * Constructor.
     */
    WebvttEngine() = default;

    /**
     * Destructor.
     */
    virtual ~WebvttEngine() = default ;

    /**
     * Perform initialization.
     *
     * @param configProvider
     *      Configuration.
     * @param gfxWindow
     *      Window to draw.
     */
    virtual void init(const common::ConfigProvider* configProvider,
                      std::weak_ptr<gfx::Window> gfxWindow) = 0;

    /**
     * Notifies size of related video content. Used for position and size calculations.
     *
     * @param relatedVideoSize
     */
    virtual void setRelatedVideoSize(gfx::Size relatedVideoSize) = 0;

    /**
     * Starts subtitles rendering.
     */
    virtual void start() = 0;

    /**
     * Stops subtitles rendering.
     */
    virtual void stop() = 0;

    /**
     * Pauses subtitle processing.
     */
    virtual void pause() = 0;

    /**
     * Resumes subtitle processing.
     */
    virtual void resume() = 0;

    /**
     * Mute subtitle display.
     */
    virtual void mute() = 0;

    /**
     * Unmute subtitle display.
     */
    virtual void unmute() = 0;

    /**
     * Set WebVTT attributes.
     *
     * @param attributes
     *      WebVtt attributes.
     */
    virtual void setAttributes(const WebVTTAttributes &attributes) = 0;

    /**
     * Adds data packet with subtitles data.
     *
     * @param buffer
     *      Buffer with WebVtt data packet.
     * @param length
     *      Length of data buffer.
     * @param displayOffsetMs
     *      offset to be applied to display start/end in ms.
     *
     * @return
     *      True on success, false on error (subtitles stopped, no more
     *      space in buffer, etc.).
     */
    virtual void addData(const std::uint8_t* buffer,
                 std::size_t dataSize, std::int64_t displayOffsetMs) = 0;

    virtual std::chrono::milliseconds getWaitTime() const = 0;

    /**
     * Performs data processing.
     */
    virtual void process() = 0;

    /**
     * Set current media time.
     *
     * @param mediatime
     *      Current media time in miliseconds from start.
     */
    virtual void currentMediatime(const std::uint64_t mediatimeMs) = 0;

};

} // namespace webvttlengine
} // namespace subttxrend
