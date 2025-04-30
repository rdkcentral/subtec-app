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

#include <cassert>

#include <inttypes.h>

#include "TtmlEngineImpl.hpp"
#include "Parser/Parser.hpp"

#include <iostream>

namespace subttxrend
{
namespace ttmlengine
{

#define DISPLAY_TIMEOUT 1

TtmlEngineImpl::TtmlEngineImpl() :
        m_logger("TtmlEngine", "TtmlEngineImpl", this)
{
    m_logger.osinfo(__LOGGER_FUNC__, " created");
}

TtmlEngineImpl::~TtmlEngineImpl()
{
    // noop
}

void TtmlEngineImpl::clear()
{
    if (m_renderer)
    {
        m_renderer->clearscreen();
        m_renderer->update();
        m_renderer->clearState();
    }

    if (m_parser)
    {
        m_parser->reset();
    }

    m_lastMediatimeMs = -1;
    m_lastMediatimeTimestamp = std::chrono::system_clock::time_point::min();

    m_paused = false;
    m_pauseEnteredTime = std::chrono::system_clock::time_point::min();
    m_pauseTimeMs = 0;
}

void TtmlEngineImpl::init(const common::ConfigProvider* configProvider,
                          gfx::Window* gfxWindow,
                          common::Properties const& properties)
{
    m_logger.osinfo(__LOGGER_FUNC__);

    assert(configProvider);

    m_dataDumper.setup(configProvider);

    m_showMediatime = (configProvider->getInt("SHOW_MEDIATIME", 0) != 0);
    if (m_showMediatime)
    {
        createTimingDoc();
    }

    m_parser = std::make_unique<Parser>();
    m_renderer = std::make_unique<TtmlRenderer>(configProvider, gfxWindow, m_dataDumper);

    m_docTransformer.setProperties(properties);

    auto ttmlFromFile = configProvider->get("READ_FROM_FILE");
    m_logger.osinfo(__LOGGER_FUNC__, " ttmlFromFile=", ttmlFromFile);
    if (not ttmlFromFile.empty())
    {
        auto data = m_dataDumper.readTtmlFromFile(ttmlFromFile);
        addData(data.data(), data.size());
        m_useTtmlFromFile = true;
    }
    clear();
}

void TtmlEngineImpl::setRelatedVideoSize(gfx::Size relatedVideoSize)
{
    m_renderer->setRelatedVideoSize(relatedVideoSize);
}

void TtmlEngineImpl::start()
{
    m_logger.osinfo(__LOGGER_FUNC__);
    assert(m_parser);

    std::lock_guard<std::mutex> lock{m_mutex};
    clear();
    m_renderer->show();
}

void TtmlEngineImpl::stop()
{
    m_logger.osinfo(__LOGGER_FUNC__);

    std::lock_guard<std::mutex> lock{m_mutex};
    clear();
    m_renderer->hide();
}

void TtmlEngineImpl::pause()
{
    m_logger.osdebug("pause received");

    std::lock_guard<std::mutex> lock{m_mutex};
    if (!m_paused)
    {
        m_pauseEnteredTime = std::chrono::system_clock::now();
    }
    m_paused = true;
}

void TtmlEngineImpl::resume()
{
    m_logger.osdebug("resume received");

    std::lock_guard<std::mutex> lock{m_mutex};
    if (m_paused)
    {
        using namespace std::chrono;
        m_pauseTimeMs += duration_cast<milliseconds>(system_clock::now() - m_pauseEnteredTime).count();
        m_logger.osdebug("pauseTimeMs: ", m_pauseTimeMs);
    }
    m_paused = false;
}

void TtmlEngineImpl::mute()
{
    m_logger.osdebug("mute received");
    std::lock_guard<std::mutex> lock{m_mutex};
    m_renderer->hide();
}

void TtmlEngineImpl::unmute()
{
    m_logger.osdebug("unmute received");
    std::lock_guard<std::mutex> lock{m_mutex};
    m_renderer->show();
}

void TtmlEngineImpl::addData(const std::uint8_t* buffer,
                             std::size_t bufferSize,
                             std::int64_t displayOffsetMs)
{
    m_logger.ostrace(__LOGGER_FUNC__,
                   " buffer=",
                   static_cast<const void*>(buffer),
                   " size=",
                   bufferSize,
                   " displayOffsetMs=",
                   displayOffsetMs,
                   " mediatime=",
                   getCurrentMediatime().toStr());

    if (m_useTtmlFromFile)
    {
        m_logger.osinfo("using ttml from file, skipping data");
        return;
    }

    m_dataDumper.dump(buffer, bufferSize);
    assert(m_parser);

    std::lock_guard<std::mutex> lock{m_mutex};

    auto preParseTimeLineSize = m_timeline.size();
    std::list<IntermediateDocument> docList = m_parser->parse(buffer, bufferSize);

    for(auto& doc : docList){
        doc.applyDisplayOffset(displayOffsetMs);
        m_docTransformer.transform(doc);
    }

    m_timeline.merge(docList);

    if (preParseTimeLineSize == m_timeline.size())
    {
        m_logger.osinfo("no data added, empty document received?");
    }

//#define VERBOSE_DEBUGGING
#ifdef VERBOSE_DEBUGGING
    if (!m_timeline.empty() && (preParseTimeLineSize != m_timeline.size()))
    {
        auto& doc = timeline.back();
        TimePoint& start = doc.m_timing.getStartTimeRef();
        TimePoint& end = doc.m_timing.getEndTimeRef();

        const std::string& docStr = doc.toStr();

        m_logger.osinfo(__LOGGER_FUNC__, " added ", docStr);
    }
#endif
}

void TtmlEngineImpl::currentMediatime(const std::uint64_t mediatimeMs)
{
    {
        std::lock_guard<std::mutex> lock{m_mutex};

        m_lastMediatimeMs = mediatimeMs;
        m_lastMediatimeTimestamp = std::chrono::system_clock::now();
        m_pauseTimeMs = 0;
    }
    const std::string& currentMediaTimeStr = getCurrentMediatime().toStr();
    m_logger.osdebug(__LOGGER_FUNC__, " mediatime=", currentMediaTimeStr, " (mediaTimeMs=", mediatimeMs, ")");
}

bool TtmlEngineImpl::mergeImages()
{
    bool merged{};
    if (!m_timeline.empty() && !m_shownDocuments.empty()) {
        auto& doc = m_timeline.front();
        auto& lastDoc = m_shownDocuments.back();
        if (lastDoc.isSameImage(doc) && lastDoc.m_timing.isContinous(doc.m_timing)) {
            lastDoc.m_timing.merge(doc.m_timing);
            m_logger.ostrace(__LOGGER_FUNC__, " extend duration to: ", lastDoc.toStr());
            m_timeline.pop_front();
            merged = true;
        }
    }
    return merged;
}

void TtmlEngineImpl::setSubtitleInfo(const std::string& contentType, const std::string& subsInfo)
{
    m_logger.osinfo(__LOGGER_FUNC__, " contentType = ", contentType, " subtitleInfo = ", subsInfo);
    m_docTransformer.setSubtitleInfo(contentType, subsInfo);
}

void TtmlEngineImpl::process()
{
    bool needUpdate = false;
    bool newDocumentAdded = false;
    TimePoint startOfTheNewDoc;
    std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - m_displayTime).count();
    if(m_startTimer && (elapsedTime > DISPLAY_TIMEOUT))
    {
        needUpdate = true;
    }
    std::list<IntermediateDocument> shownDocuments;
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        if ((m_lastMediatimeMs != -1) && (!m_paused)) {

            assert(m_renderer);

            auto const currentMediaTime = getCurrentMediatime();
            auto currentMediaTimeMs = currentMediaTime.toMilliseconds();

            // merge if possible to extend image duration prior
            // to removing expired ones
            while (mergeImages()){
                // intentionally empty
            };

            // remove no-loger visible ones
            m_shownDocuments.remove_if([&needUpdate, currentMediaTimeMs](IntermediateDocument& doc) {
                auto end = doc.m_timing.getEndTimeRef().toMilliseconds();
                bool ret = end < currentMediaTimeMs;
                if (!ret) {
                    needUpdate = false;
                }
                return ret;
            });

            // add new subtitles to show
            while (!m_timeline.empty()) {

                // try to merge next in a queue to the previous one first
                // if it is imposible continue normal processing
                if (!mergeImages()) {

                    auto& doc = m_timeline.front();

                    auto const start = doc.m_timing.getStartTimeRef().toMilliseconds();
                    auto const end = doc.m_timing.getEndTimeRef().toMilliseconds();

                    if (start  <=  currentMediaTimeMs && end > currentMediaTimeMs) {
                        m_logger.osinfo(__LOGGER_FUNC__,
                                " mediaTime=",
                                currentMediaTime.toStr(), // TODO operator <<
                                " displaying: ",
                                doc.toStr(), " diff: ", (currentMediaTimeMs - start).count()); // TODO operator<<

                        if (!needUpdate) {
                            needUpdate = true;
                            newDocumentAdded = true;
                            startOfTheNewDoc = doc.m_timing.getStartTimeRef();
                        }
                        m_shownDocuments.emplace_back(doc);
                        m_timeline.pop_front();
                    } else if (start > currentMediaTimeMs) {
                        break;
                    } else {
                        m_logger.oswarning(__LOGGER_FUNC__,
                                " mediaTime=",
                                currentMediaTime.toStr(),
                                " skipping: ",
                                doc.toStr()); // TODO operator<<
                        m_timeline.pop_front();
                    }
                }
            }

            shownDocuments = m_shownDocuments;
            if (m_showMediatime) {
                needUpdate |= timingUpdate();
            }
        }
    }

    // update the view, if necessary
    if (needUpdate) {
        {
            auto t = m_logger.timing("renderer->clearscreen");
            m_renderer->clearscreen();
            m_displayTime = std::chrono::steady_clock::now();
            m_startTimer = false;
        }
        {
            auto t = m_logger.timing("renderer->renderDocument");
            for (auto& doc : shownDocuments) {
                m_renderer->renderDocument(doc);
                m_startTimer = true;
                m_displayTime = std::chrono::steady_clock::now();
            }
            if (newDocumentAdded) {
                auto start = startOfTheNewDoc.toMilliseconds();
                auto now = getCurrentMediatime().toMilliseconds();
                m_logger.ostrace(__LOGGER_FUNC__, " display real diff: ", (now - start).count(), " shown count: ", shownDocuments.size());
            }
        }

        if (m_showMediatime) {
            auto t = m_logger.timing("renderer->renderDocument timing");
            m_renderer->renderDocument(*m_timingDoc.get());
        }
        {
            auto t = m_logger.timing("renderer->update");
            m_renderer->update();
        }
    }
}

std::chrono::milliseconds TtmlEngineImpl::getWaitTime() const
{
    auto waitTime = std::chrono::milliseconds::zero();

    auto anythingToDraw = !m_timeline.empty();
    auto anythingToHide = !m_shownDocuments.empty() || m_startTimer;

    if ((anythingToDraw || anythingToHide)) {
        // TimePoint const currentMediaTime = getCurrentMediatime();

        // if (anythingToDraw) {
        //     TimePoint const& start = m_timeline.front().m_timing.getStartTimeRef();

        //     if (currentMediaTime < start) {
        //         waitTime = start.toMilliseconds() - currentMediaTime.toMilliseconds();
        //     }
        // }

        // for_each(m_shownDocuments.begin(), m_shownDocuments.end(), [&waitTime,
        //                                                             currentMediaTime](const IntermediateDocument &doc) {
        //     TimePoint const& end = doc.m_timing.getEndTimeRef();
        //     auto hideTime = end.toMilliseconds() - currentMediaTime.toMilliseconds();
        //     if (hideTime < waitTime) {
        //         waitTime = hideTime;
        //     }
        // });

        auto static constexpr MIN_WAIT_TIME = 25ms;
        // if (waitTime < MIN_WAIT_TIME) {
            waitTime = MIN_WAIT_TIME;
        // }
    }

    m_logger.osdebug(__LOGGER_FUNC__, " waitTime: ", waitTime.count());
    return waitTime;
}

TimePoint TtmlEngineImpl::getCurrentMediatime() const
{
    using namespace std::chrono;

    std::uint64_t mediatimeDiffMs = duration_cast<milliseconds>(system_clock::now() - m_lastMediatimeTimestamp).count();
    return TimePoint(m_lastMediatimeMs + mediatimeDiffMs - m_pauseTimeMs);
}

void TtmlEngineImpl::createTimingDoc()
{
    assert(! m_timingDoc);
    m_timingDoc = std::make_unique<IntermediateDocument>();

    IntermediateDocument::TextChunk timingText;
    IntermediateDocument::TextLine timingLine;
    IntermediateDocument::Entity timingEntity;

    timingLine.push_back(timingText);
    timingEntity.m_textLines.push_back(timingLine);
    timingEntity.m_imageChunk.m_image = std::make_shared<ImageElement>();
    timingEntity.m_region = std::make_shared<RegionElement>(1000, 1000, 9000, 9000);

    m_timingDoc->m_entites.push_back(timingEntity);

    m_displayedText = &m_timingDoc->m_entites.front().m_textLines.front().front().m_text;
}

bool TtmlEngineImpl::timingUpdate()
{
    using namespace std::chrono;

    static auto lastUpdate = system_clock::now();
    static constexpr int UPDATE_PERIOD_MS = 200;

    auto clockNow = system_clock::now();
    if ((duration_cast<milliseconds>(clockNow - lastUpdate).count()) > UPDATE_PERIOD_MS)
    {
        lastUpdate = clockNow;
        assert(m_displayedText);
        *m_displayedText = getCurrentMediatime().toStr();
        m_logger.osinfo(__LOGGER_FUNC__, " timing str: ", *m_displayedText);
        return true;
    }
    else
    {
        return false;
    }
}

} // namespace ttmlengine
} // namespace subttxrend

