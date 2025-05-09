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


#ifndef SUBTTXREND_TTML_TTML_ENGINE_HPP
#define SUBTTXREND_TTML_TTML_ENGINE_HPP

#include <cstdint>
#include <chrono>
#include <string>

#include <subttxrend/common/ConfigProvider.hpp>
#include <subttxrend/common/Properties.hpp>
#include <subttxrend/gfx/Engine.hpp>

namespace subttxrend
{
namespace ttmlengine
{

class TtmlEngine
{
public:

    /**
     * Constructor.
     */
    TtmlEngine() = default;

    /**
     * Destructor.
     */
    virtual ~TtmlEngine() = default ;

    /**
     * Perform initialization.
     *
     * @param configProvider
     *      Configuration.
     * @param gfxWindow
     *      Window to draw.
     */
    virtual void init(const common::ConfigProvider* configProvider,
                      gfx::Window* gfxWindow,
                      common::Properties const& properties) = 0;

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
     * Flushes subtitle state.
     */
    virtual void flush() = 0;

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
     * Adds data packet with subtitles data.
     *
     * @param buffer
     *      Buffer with TTML data packet.
     * @param length
     *      Length of data buffer.
     * @param displayOffsetMs
     *      offset to be applied to display start/end in ms.
     */
    virtual void addData(const std::uint8_t* buffer,
                 std::size_t dataSize, std::int64_t displayOffsetMs) = 0;

    /**
     * Performs data processing.
     */
    virtual void process() = 0;

    /**
     * Returns wait time to next process() call.
     *
     * @return
     *      Time to wait before next call to process(). std::chrono::milliseconds::zero()
     *      for unknown time - when there is no data queued or in paused state.
     */
    virtual std::chrono::milliseconds getWaitTime() const = 0;

    /**
     * Set current media time.
     *
     * @param mediatime
     *      Current media time in miliseconds from start.
     */
    virtual void currentMediatime(const std::uint64_t mediatimeMs) = 0;

    /**
     * Set subtitle info.
     *
     * @param contentType
     *      Content type: vod/live/...
     * @param subsInfo
     *      Subtitle info carrying information about subtitle language and type.
     *      Format: 3 letter language code and type, e.g. "eng nrm".
     */
    virtual void setSubtitleInfo(const std::string& contentType, const std::string& subsInfo) = 0;

};

} // namespace ttmlengine
} // namespace subttxrend

#endif /* SUBTTXREND_TTML_TTML_ENGINE_HPP */
