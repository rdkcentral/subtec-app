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


#include "ScteController.hpp"

#include <stdexcept>
#include <cmath>
#include <subttxrend/common/Logger.hpp>

namespace subttxrend
{
namespace scte
{

namespace
{

common::Logger g_logger("Scte", "ScteController");

bool isAfter(std::uint32_t pts1, std::uint32_t pts2)
{
    return ((pts1 - pts2) < 0x80000000);
}

constexpr std::uint16_t SECTION_HEADER_SIZE = 3;

bool findNextSectionStart(const std::uint8_t*& data, std::uint32_t& size)
{
    constexpr std::uint8_t SECTION_START = 0xC6;

    while ((size >= SECTION_HEADER_SIZE) and (data[0] != SECTION_START))
    {
        data++;
        size--;
    }
    return ((size >= SECTION_HEADER_SIZE) and (data[0] == SECTION_START));
}

}

Controller::Controller() :
        m_isStarted(false),
        m_isMuted(false),
        m_stcProvider(),
        m_decoder(),
        m_database(),
        m_renderer(),
        m_presented()
{
    g_logger.trace("%s", __func__);
}

Controller::~Controller()
{
    g_logger.trace("%s", __func__);
}

bool Controller::init(gfx::Window* gfxWindow,
                      common::StcProvider* stcProvider)
{
    m_stcProvider = stcProvider;
    g_logger.trace("%s", __func__);

    m_renderer = std::make_unique<Renderer>(gfxWindow);

    g_logger.info("%s - done", __func__);

    return true;
}

void Controller::shutdown()
{
    g_logger.trace("%s", __func__);
    m_decoder.clear();
    clearDatabase();
    g_logger.info("%s - done", __func__);
}

bool Controller::addData(const protocol::PacketData& dataPacket)
{
    if (m_isStarted)
    {
        g_logger.trace("%s size: %zu", __func__, dataPacket.getDataSize());

        auto buffer = reinterpret_cast<const std::uint8_t*>(dataPacket.getData());
        auto bufferSize = static_cast<std::uint32_t>(dataPacket.getDataSize());

        while(findNextSectionStart(buffer, bufferSize))
        {
            try
            {
                auto section = Section{buffer, bufferSize};

                auto consumedBytes = SECTION_HEADER_SIZE + section.sectionLength();
                g_logger.debug("%s adding section, consumed %d of %d", __func__, consumedBytes, bufferSize);

                m_decoder.pushSection(section);

                buffer += consumedBytes;
                bufferSize -= consumedBytes;

            }
            catch(const std::exception& ex)
            {
                g_logger.warning("%s parsing section failed, reason: %s", __func__, ex.what());

                buffer++;
                bufferSize--;
            }
        }

        return true;
    }
    else
    {
        g_logger.warning("%s - skipping, not started", __func__);

        return false;
    }
}

bool Controller::start()
{

    if (m_isStarted)
    {
        stop();
    }

    if (!m_isMuted)
    {
        m_renderer->clearscreen();
        m_renderer->update();
        m_renderer->show();
    }
    m_decoder.clear();
    clearDatabase();

    m_isStarted = true;

    g_logger.info("%s - done", __func__);

    return true;
}

bool Controller::stop()
{
    g_logger.info("%s", __func__);

    if (m_isStarted)
    {
        m_renderer->hide();

        m_isStarted = false;
    }

    g_logger.info("%s - done", __func__);

    return true;
}

bool Controller::isStarted() const
{
    return m_isStarted;
}

void Controller::mute()
{
    g_logger.info("%s", __func__);

    if (!m_isMuted)
    {
        m_isMuted = true;

        if (m_isStarted)
        {
            m_renderer->hide();
        }
    }

    g_logger.trace("%s - done", __func__);
}

void Controller::unmute()
{
    g_logger.info("%s", __func__);

    if (m_isMuted)
    {
        m_isMuted = false;

        if (m_isStarted)
        {
            m_renderer->clearscreen();
            m_renderer->show();
        }
    }

    g_logger.trace("%s - done", __func__);
}

bool Controller::isMuted() const
{
    return m_isMuted;
}

bool Controller::isTimeToDisplay(const ScteTablePtr& table, std::uint32_t stc)
{
    return isAfter(stc + DISPLAY_TIME_OFFSET_PCR, table->getPTS());
}

void Controller::processPackets(std::uint32_t stc)
{
    while(auto table = m_decoder.provideData())
    {
        if (table->isImmediate())
        {
            g_logger.debug("%s immediate packet received, discarding database", __LOGGER_FUNC__);

            clearDatabase();
            m_database.emplace(std::move(table));
        }
        else
        {
            std::uint32_t pts = table->getPTS();
            if (isAfter(pts, stc))
            {
                if (! m_database.empty())
                {
                    auto lastStartPts = m_database.back().get()->getPTS();
                    if ((lastStartPts != 0) && isAfter(lastStartPts, pts))
                    {
                        g_logger.debug("%s newer packet received, last pts: 0x%08x new pts: 0x%08x, discarding database",
                            __LOGGER_FUNC__, lastStartPts, pts);
                        clearDatabase();
                    }
                }
                m_database.emplace(std::move(table));
            }
            else
            {
                g_logger.warning("%s overdue packet dropped (std:0x%08x pts:0x%08x)",
                    __LOGGER_FUNC__, stc, table->getPTS());
            }
        }
    }
}

void Controller::process()
{
    g_logger.trace("%s", __func__);

    if (!m_isStarted)
    {
        g_logger.trace("%s - not started", __func__);
        return;
    }

    // find bitmaps in our database then throw them into gfx here

    if (m_isMuted)
    {
        g_logger.trace("%s - muted", __func__);
        return;
    }

    // stc returns higher 32 of 33 pcr bits, pts are signaled as lower 32 bits -> adjust stc to match
    std::uint32_t stc = m_stcProvider->getStc() << 1;

    processPackets(stc);

    g_logger.trace("%s - drawing", __func__);

    while(! m_database.empty())
    {
        auto& table = m_database.front();

        if (table->isImmediate() || isTimeToDisplay(table, stc))
        {
            m_presented = std::move(table);
            m_database.pop();

            g_logger.debug("%s rendering %p, pts: 0x%08x endPts: 0x%08x stc: 0x%08x",
                __func__,
                m_presented.get(),
                m_presented->getPTS(),
                m_presented->getEndPts(),
                stc);

            if (m_presented->preClearDisplay())
            {
                m_renderer->clearscreen();
            }
            render(m_presented.get());
        }
        else
        {
            break;
        }
    }

    if (m_presented && isAfter(stc, m_presented->getEndPts()))
    {
        g_logger.debug("%s clearing %p, endPts: 0x%08x stc: 0x%08x",
            __func__,
            m_presented.get(),
            m_presented->getEndPts(),
            stc);

        m_renderer->clearscreen();
        m_renderer->update();
        m_presented = nullptr;
    }
}

void Controller::render(const ScteTable* table)
{
    auto& sbitmap = table->getSimpleBitmap();
    size_t width = 1920;
    size_t height = 1080;

    switch(table->getDisplayStandard())
    {
      case DisplayStandard::DS_720_480_30:
        width = 720;
        height = 480;
        break;
      case DisplayStandard::DS_720_576_25:
        width = 720;
        height = 576;
        break;
      case DisplayStandard::DS_1280_720_60:
        width = 1280;
        height = 720;
        break;
      case DisplayStandard::DS_1920_1080_60:
        width = 1920;
        height = 1080;
        break;
    }

    g_logger.debug("%s - rendering bitmap: %p (w:%zu, h:%zu) endPts: 0x%08x",
        __LOGGER_FUNC__,
        &sbitmap,
        width,
        height,
        table->getEndPts());

    m_renderer->render(sbitmap, width, height);
}

void Controller::clearDatabase()
{
    decltype(m_database) empty;
    std::swap(m_database, empty);
}

} // namespace scte
} // namespace subttxrend

