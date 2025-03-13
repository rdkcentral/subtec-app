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

#include <memory>
#include <mutex>
#include <chrono>

#include <WebvttEngine.hpp>
#include <WebVTTRenderer.hpp>
#include <Timing.hpp>
#include <WebVTTDocument.hpp>

#include <subttxrend/common/Logger.hpp>
#include <subttxrend/gfx/Types.hpp>

namespace subttxrend
{
namespace webvttengine
{

class Parser;

/**
 * WebvttEngine implementation.
 */
class WebvttEngineImpl : public WebvttEngine
{

public:

    /**
     * Constructor.
     */
    WebvttEngineImpl();

    /**
     * Destructor.
     */
    virtual ~WebvttEngineImpl();

    /** @copydoc WebvttEngine::init */
    virtual void init(const common::ConfigProvider* configProvider,
                      std::weak_ptr<gfx::Window> gfxWindowPtr) override;

    /** @copydoc WebvttEngine::setRelatedVideoSize */
    virtual void setRelatedVideoSize(gfx::Size relatedVideoSize) override;

    virtual std::chrono::milliseconds getWaitTime() const override;

    /** @copydoc WebvttEngine::start */
    virtual void start() override;

    /** @copydoc WebvttEnging::stop */
    virtual void stop() override;

    /** @copydoc WebvttEnging::pause */
    virtual void pause() override;

    /** @copydoc WebvttEnging::resume */
    virtual void resume() override;

    /** @copydoc WebvttEnging::mute */
    virtual void mute() override;

    /** @copydoc WebvttEnging::unmute */
    virtual void unmute() override;

    /** @copydoc WebvttEnging::setAttributes */
    virtual void setAttributes(const WebVTTAttributes &attributes) override;

    /** @copydoc WebvttEnging::addData */
    virtual void addData(const std::uint8_t* buffer, std::size_t dataSize, std::int64_t displayOffsetMs = 0) override;

    /** @copydoc WebvttEnging::process */
    virtual void process() override;

    /** @copydoc WebvttEnging::currentMediatime */
    virtual void currentMediatime(const std::uint64_t mediatimeMs) override;

private:

    void clear();

    /**
     * Calculates current media time taking into account time passed media time was received.
     *
     * @return
     *      Estimated mediatime.
     */
    TimePoint getCurrentMediatime() const;

    /** Webvtt config parser. */
    WebVTTConfig                            m_config;

    /** Webvtt renderer. */
    std::unique_ptr<WebVTTRenderer>         m_renderer{};

    /** Last received media time in milliseconds. */
    std::int64_t                            m_lastMediatimeMs{-1};

    /** Timestamp when last mediatime was received. */
    std::chrono::system_clock::time_point   m_lastMediatimeTimestamp{};

    /** Is paused flag. */
    bool                                    m_paused{false};

    /** Time when pause was entered. */
    std::chrono::system_clock::time_point   m_pauseEnteredTime{};

    /** Total time spent in pause in milliseconds. */
    std::uint64_t                           m_pauseTimeMs{};

    /** Ordered list of subtitles. */
    std::mutex                              m_mutex;
    CueList                                 m_timeline;
    std::list<CueSharedPtr>                 m_shownDocuments;
    RegionMap                               m_cachedRegionMap;
    const gfx::Size                         DEFAULT_SURFACE_SIZE{1280, 720};
    gfx::Size                               m_surfaceSize{DEFAULT_SURFACE_SIZE};

};

} // namespace subttxrend
} // namespace webvttengine
