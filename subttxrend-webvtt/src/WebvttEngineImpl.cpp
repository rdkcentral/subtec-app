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
#include <cassert>
#include <iostream>
#include <algorithm>

#include <inttypes.h>

#include <WebvttEngineImpl.hpp>
#include <WebVTTDocument.hpp>
#include <WebVTTConfig.hpp>
#include <WebVTTExceptions.hpp>
#include <LineBuilder.hpp>


namespace subttxrend
{
namespace webvttengine
{

using namespace std::literals;
namespace { common::Logger g_logger("WebvttEngine", "WebvttEngineImpl"); }

WebvttEngineImpl::WebvttEngineImpl()
{
    g_logger.osinfo(__LOGGER_FUNC__, " created");
}

WebvttEngineImpl::~WebvttEngineImpl()
{
    // noop
}

void WebvttEngineImpl::clear()
{
    g_logger.osinfo(__LOGGER_FUNC__);
    if (m_renderer)
    {
        m_renderer->clearState();
        m_renderer->clearscreen();
        m_renderer->update();
    }
    
    m_timeline.clear();

    m_lastMediatimeMs = -1;
    m_lastMediatimeTimestamp = std::chrono::system_clock::time_point::min();

    m_paused = false;
    m_pauseEnteredTime = std::chrono::system_clock::time_point::min();
    m_pauseTimeMs = 0;
}

void WebvttEngineImpl::init(const common::ConfigProvider* configProvider,
                          std::weak_ptr<gfx::Window> gfxWindowPtr)
{
    g_logger.osinfo(__LOGGER_FUNC__);

    assert(configProvider);
    m_config.init(configProvider);

    m_renderer = std::make_unique<WebVTTRenderer>(gfxWindowPtr, m_config);

    clear();
}

void WebvttEngineImpl::setRelatedVideoSize(gfx::Size relatedVideoSize)
{
    m_surfaceSize = relatedVideoSize;
}

void WebvttEngineImpl::start()
{
    g_logger.osinfo(__LOGGER_FUNC__);

    clear();
    m_renderer->show();
}

void WebvttEngineImpl::stop()
{
    g_logger.osinfo(__LOGGER_FUNC__);

    clear();
    m_renderer->hide();
}

void WebvttEngineImpl::pause()
{
    g_logger.osdebug("pause received");

    std::lock_guard<std::mutex> lock{m_mutex};
    if (!m_paused)
    {
        m_pauseEnteredTime = std::chrono::system_clock::now();
    }
    m_paused = true;
}

void WebvttEngineImpl::resume()
{
    g_logger.osdebug("resume received");

    std::lock_guard<std::mutex> lock{m_mutex};
    if (m_paused)
    {
        using namespace std::chrono;
        m_pauseTimeMs += duration_cast<milliseconds>(system_clock::now() - m_pauseEnteredTime).count();
        g_logger.osdebug("pasueTimeMs: ", m_pauseTimeMs);
    }
    m_paused = false;
}

void WebvttEngineImpl::mute()
{
    g_logger.osdebug("mute received");
    m_renderer->hide();
}

void WebvttEngineImpl::unmute()
{
    g_logger.osdebug("unmute received");
    m_renderer->show();
}

void WebvttEngineImpl::setAttributes(const WebVTTAttributes &attributes)
{
    m_renderer->setAttributes(attributes);
}

/**
 * @brief Simple string helper method
 * 
 * @param str 
 * @param find 
 * @param replace 
 */
void ReplaceAll(std::string & str, const std::string &find, const std::string &replace) {
    std::size_t pos = 0;
    while ((pos = str.find(find, pos)) != std::string::npos) {
        str.replace(pos, find.length(), replace);
        pos += replace.length();
    };
}

/**
 * @brief Gets a map of Regions. The list returned from the current file might not have regions
 * for cues from previous files, so this checks those too.
 * 
 * @param cues 
 * @param latest 
 * @param last 
 * @return RegionMap 
 */
RegionMap GetUnifiedRegionMap(const CueList &cues, const RegionMap &latest_regions, const RegionMap &cached_regions) {
    RegionMap unified_map;
    std::set<std::string> region_ids_in_timeline;
    
    //Get list of region IDs from timeline
    std::for_each(cues.begin(), cues.end(), [&region_ids_in_timeline](const auto &cue) {
        if (!cue->regionId().empty()) region_ids_in_timeline.insert(cue->regionId());
    });
    
    //First check the list returned from the present document. If it's not there,
    //get it from the previous list
    for (const auto &region_id : region_ids_in_timeline) {
        if (latest_regions.count(region_id)) {
            unified_map.emplace(region_id, latest_regions.at(region_id));
        } else if (cached_regions.count(region_id)) {
            unified_map.emplace(region_id, cached_regions.at(region_id));
        } else {
            g_logger.oswarning(__LOGGER_FUNC__, " - ", region_id, "region not in list - will display using defaults");
        }
    }
    return unified_map;
}

/**
 * @brief Called from App with data from incoming cue file
 *        Will parse all the data into local data structures for rendering
 * 
 * @param buffer 
 * @param bufferSize 
 * @param displayOffsetMs 
 */
void WebvttEngineImpl::addData(const std::uint8_t* buffer,
                             std::size_t bufferSize,
                             std::int64_t displayOffsetMs)
{
    g_logger.ostrace(__LOGGER_FUNC__,
                   " buffer=",
                   static_cast<const void*>(buffer),
                   " size=",
                   bufferSize,
                   " displayOffsetMs=",
                   displayOffsetMs,
                   " mediatime=",
                   getCurrentMediatime());

    std::lock_guard<std::mutex> lock{m_mutex};
    {
        auto preParseTimeLineSize = m_timeline.size();

        try {
            std::string buffer_string = std::string(buffer, buffer + bufferSize);
            
            //Fix line endings
            if (buffer_string.find("\r") != std::string::npos) {
                ReplaceAll(buffer_string, "\r\n", "\n");
                ReplaceAll(buffer_string, "\r", "");
            }
            //Trim padding NULLs from end
            while (buffer_string.back() == '\0') 
                buffer_string.pop_back();

            std::istringstream ss(buffer_string);
            WebVTTDocument documentParser;
            CueList documentCueList;
            RegionMap regionMap;
            
            //Get list of cues and regions from the incoming WebVTT file
            std::tie(documentCueList, regionMap) = documentParser.parseCueList(ss, displayOffsetMs);
           
           //Merge incoming cues and regions with cache
            m_timeline.merge(documentCueList, [](const CuePtr &a, const CuePtr &b) -> bool { return *a < *b; });
            m_cachedRegionMap = GetUnifiedRegionMap(m_timeline, regionMap, m_cachedRegionMap);
        } catch (const WebVTTException& e) {
            g_logger.oswarning(__LOGGER_FUNC__, e.what());
        }
        
        if (preParseTimeLineSize == m_timeline.size())
        {
            g_logger.osinfo("no data added, empty document received?");
        }
    }
}

/**
 * @brief Stores incoming timestamp along with the current time
 * 
 * @param mediatimeMs 
 */
void WebvttEngineImpl::currentMediatime(const std::uint64_t mediatimeMs)
{
    m_lastMediatimeMs = mediatimeMs;
    m_lastMediatimeTimestamp = std::chrono::system_clock::now();
    m_pauseTimeMs = 0;

    g_logger.osinfo(__LOGGER_FUNC__, " mediatime=", getCurrentMediatime(), " (mediaTimeMs=", mediatimeMs, ")");
}

/**
 * @brief Called from App to check cues and display if necessary
 *        Creates a list of shown cues, prunes old cues, checks for duplicates,
 *        then sends any current cues to the renderer for display
 * 
 */
void WebvttEngineImpl::process()
{
    bool needUpdate = false;
    std::list<CueSharedPtr> shownCues;
    {
        if ((m_lastMediatimeMs != -1) && (!m_paused)) {

            assert(m_renderer);
            TimePoint currentMediaTime = getCurrentMediatime();

            // remove no-loger visible ones
            m_shownDocuments.remove_if([&needUpdate, currentMediaTime, this](const CueSharedPtr &doc) {
                TimePoint const& end = doc->endTime();
                bool ret = (end <= currentMediaTime);
                if (ret) {
                    needUpdate = true;
                }
                return ret;
            });
            // add new subtitles to show
            while (!m_timeline.empty()) {
                std::lock_guard<std::mutex> lock{m_mutex};
                {
                    auto& doc = m_timeline.front();

                    TimePoint const& start = doc->startTime();
                    TimePoint const& end = doc->endTime();

                    if (start <= currentMediaTime && end > currentMediaTime) {
                        g_logger.osinfo(__LOGGER_FUNC__,
                                    " mediaTime=",
                                    currentMediaTime,
                                    "; displaying: ",
                                    *doc);

                        needUpdate = true;
                        m_shownDocuments.emplace_back(static_cast<CueSharedPtr>(std::move(doc)));
                        m_shownDocuments.unique([](const CueSharedPtr &a, CueSharedPtr &b) -> bool 
                            { return *a == *b; });
                        m_timeline.pop_front();
                    } else if (start > currentMediaTime) {
                        break;
                    } else {
                        g_logger.oswarning(__LOGGER_FUNC__,
                                        " skipping outdated document ended=",
                                        end,
                                        ", mediaTime=",
                                        currentMediaTime);
                        m_timeline.pop_front();
                    }
                }
            }
            shownCues = m_shownDocuments;
        }
    }

    // update the view, if necessary
    if (needUpdate) {
        {
            auto t = g_logger.timing("renderer->clearscreen");
            m_renderer->clearscreen();
        }
        {
            g_logger.osinfo("renderer->renderDocument - cue number:", shownCues.size());
            auto t = g_logger.timing("renderer->renderDocument");
            m_renderer->renderDocument(shownCues, m_cachedRegionMap);
        }

        {
            auto t = g_logger.timing("renderer->update");
            m_renderer->update();
        }
    }
}

TimePoint WebvttEngineImpl::getCurrentMediatime() const
{
    using namespace std::chrono;

    std::uint64_t mediatimeDiffMs = duration_cast<milliseconds>(system_clock::now() - m_lastMediatimeTimestamp).count();
    return TimePoint(m_lastMediatimeMs + mediatimeDiffMs - m_pauseTimeMs);
}

std::chrono::milliseconds WebvttEngineImpl::getWaitTime() const
{
    auto waitTime = std::chrono::milliseconds::zero();

    auto anythingToDraw = !m_timeline.empty();
    auto anythingToHide = !m_shownDocuments.empty();

    if ((anythingToDraw || anythingToHide) && (m_lastMediatimeMs != -1) && (!m_paused)) {
        // TimePoint const currentMediaTime = getCurrentMediatime();

        // if (anythingToDraw) {
        //     TimePoint const& start = m_timeline.front()->startTime();

        //     if (currentMediaTime < start) {
        //         waitTime = start.toMilliseconds() - currentMediaTime.toMilliseconds();
        //     }
        // }

        // for_each(m_shownDocuments.begin(), m_shownDocuments.end(), [&waitTime,
        //                                                             currentMediaTime](const CueSharedPtr &doc) {
        //     TimePoint const& end = doc->endTime();
        //     auto hideTime = end.toMilliseconds() - currentMediaTime.toMilliseconds();
        //     if (hideTime < waitTime || waitTime == std::chrono::milliseconds::zero()) {
        //         waitTime = hideTime;
        //     }
        // });

        auto static constexpr MIN_WAIT_TIME = 25ms;
        // if (waitTime < MIN_WAIT_TIME) {
            waitTime = MIN_WAIT_TIME;
        // }
    }
    else if (m_paused)
    {
       auto static constexpr MIN_WAIT_TIME = 250ms;
       waitTime = MIN_WAIT_TIME;
    }

    g_logger.osdebug(__LOGGER_FUNC__, " waitTime: ", waitTime.count(), ", paused: ", m_paused);
    return waitTime;
}
} // namespace webvttengine
} // namespace subttxrend


