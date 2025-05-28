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


#ifndef SUBTTXREND_TTML_TTMLENGINEIMPL_HPP
#define SUBTTXREND_TTML_TTMLENGINEIMPL_HPP

#include <memory>
#include <mutex>
#include <chrono>

#include "TtmlEngine.hpp"
#include "TtmlRenderer.hpp"
#include "Parser/Timing.hpp"
#include "TtmlTransformer.hpp"
#include <subttxrend/common/Logger.hpp>

namespace subttxrend
{
namespace ttmlengine
{

class Parser;

/**
 * TtmlEngine implementation.
 */
class TtmlEngineImpl : public TtmlEngine
{

public:

    /**
     * Constructor.
     */
    TtmlEngineImpl();

    /**
     * Destructor.
     */
    virtual ~TtmlEngineImpl();

    /** @copydoc TtmlEngine::init */
    virtual void init(const common::ConfigProvider* configProvider,
                      gfx::Window*  gfxEngine,
                      common::Properties const& properties) override;

    /** @copydoc TtmlEngine::setRelatedVideoSize */
    virtual void setRelatedVideoSize(gfx::Size relatedVideoSize) override;

    /** @copydoc TtmlEngine::start */
    virtual void start() override;

    /** @copydoc TtmlEngine::stop */
    virtual void stop() override;

    /** @copydoc TtmlEngine::flush */
    virtual void flush() override;

    /** @copydoc TtmlEngine::pause */
    virtual void pause() override;

    /** @copydoc TtmlEngine::resume */
    virtual void resume() override;

    /** @copydoc TtmlEngine::mute */
    virtual void mute() override;

    /** @copydoc TtmlEngine::unmute */
    virtual void unmute() override;

    /** @copydoc TtmlEngine::addData */
    virtual void addData(const std::uint8_t* buffer, std::size_t dataSize, std::int64_t displayOffsetMs = 0) override;

    /** @copydoc TtmlEngine::process */
    virtual void process() override;

    /** @copydoc TtmlEngine::getWaitTime */
    virtual std::chrono::milliseconds getWaitTime() const override;

    /** @copydoc TtmlEngine::currentMediatime */
    virtual void currentMediatime(const std::uint64_t mediatimeMs) override;

    /** @copydoc TtmlEngine::setSubtitleInfo */
    virtual void setSubtitleInfo(const std::string& contentType, const std::string& subsInfo) override;


private:

    void clear();

    /**
     * Calculates current media time taking into account time passed media time was received.
     *
     * @return
     *      Estimated mediatime.
     */
    TimePoint getCurrentMediatime() const;

    /**
     * Merge last document of m_shownDocuments with first of m_timeline
     * if both represents the same image (does not work for text subtitles)
     */
    bool mergeImages();
    /**
     * Debug feature. Creates data container for timing data to be displayed on screen.
     */
    void createTimingDoc();

    /**
     * Debug feature. Updated current media time displayed on screen.
     *
     * @return
     *      True if it's time to refresh timing data.
     */
    bool timingUpdate();

    /** Ttml parser. */
    std::unique_ptr<Parser> m_parser;

    /** Logger object. */
    mutable subttxrend::common::Logger m_logger;

    /** Ttml renderer. */
    std::unique_ptr<TtmlRenderer> m_renderer{};

    /** Last received media time in milliseconds. */
    std::int64_t m_lastMediatimeMs{-1};

    /** Timestamp when last mediatime was received. */
    std::chrono::system_clock::time_point m_lastMediatimeTimestamp{};

    /** Is paused flag. */
    bool m_paused{false};

    /** Time when pause was entered. */
    std::chrono::system_clock::time_point m_pauseEnteredTime{};

    /** Total time spent in pause in milliseconds. */
    std::uint64_t m_pauseTimeMs{};

    /** DEBUG FEATURES */

    /** Data dumper - used for debugging purposes. */
    DataDumper m_dataDumper;

    /** Debug feature - show current media time on screen. */
    bool m_showMediatime{false};

    /** Path file ttml data to be read from,*/
    std::string m_pathTtmlFromFile;

    /** Ordered list of subtitles. */
    mutable std::mutex m_mutex;
    std::list<IntermediateDocument> m_timeline;
    std::list<IntermediateDocument> m_shownDocuments;

    /** Hold current media time to display. */
    std::unique_ptr<IntermediateDocument> m_timingDoc;
    std::string* m_displayedText{};

    /** Intermediate Document transformer*/
    TtmlTransformer m_docTransformer;

    std::chrono::steady_clock::time_point m_displayTime{std::chrono::steady_clock::now()};
    bool m_startTimer{true};
};

} // namespace subttxrend
} // namespace ttmlengine

#endif /* SUBTTXREND_TTML_TTMLENGINEIMPL_HPP */
