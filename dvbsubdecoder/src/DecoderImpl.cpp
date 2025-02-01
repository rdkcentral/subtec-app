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


#include "DecoderImpl.hpp"

#include <subttxrend/common/Logger.hpp>

#include "DecoderClient.hpp"
#include "TimeProvider.hpp"

namespace dvbsubdecoder
{

namespace
{

subttxrend::common::Logger g_logger("DvbSubDecoder", "DecoderImpl");

} // namespace <anonmymous>

DecoderImpl::DecoderImpl(Specification specVersion,
                         Allocator& allocator,
                         DecoderClient& client,
                         TimeProvider& timeProvider) :
        m_client(client),
        m_timeProvider(timeProvider),
        m_storage(specVersion, allocator, client),
        m_presenter(client, m_storage.getDatabase()),
        m_parser(client, m_storage.getDatabase(), m_storage.getPesBuffer()),
        m_started(false),
        m_redrawNeeded(false)
{
    g_logger.trace("%s - created: %p", __func__, this);
}

DecoderImpl::~DecoderImpl()
{
    g_logger.trace("%s - destroyed: %p", __func__, this);
}

void DecoderImpl::setPageIds(std::uint16_t compositionPageId,
                             std::uint16_t ancillaryPageId)
{
    g_logger.trace("%s - comp=%hu anc=%hu", __func__, compositionPageId,
            ancillaryPageId);

    auto& status = m_storage.getDatabase().getStatus();
    status.setPageIds(compositionPageId, ancillaryPageId);

    // forcing reset (including redraw)
    reset();
}

void DecoderImpl::start()
{
    g_logger.trace("%s", __func__);
    m_started = true;
}

void DecoderImpl::stop()
{
    g_logger.trace("%s", __func__);
    m_started = false;
}

void DecoderImpl::reset()
{
    g_logger.trace("%s", __func__);

    // clear buffer
    m_storage.getPesBuffer().clear();

    // invalidate parsing state
    m_storage.getDatabase().epochReset();
    m_storage.getDatabase().getCurrentDisplay().reset();
    m_storage.getDatabase().getParsedDisplay().reset();

    // invalidate rendering state
    invalidate();

    g_logger.trace("%s - complete", __func__);
}

bool DecoderImpl::addPesPacket(const std::uint8_t* buffer,
                               std::size_t size)
{
    g_logger.trace("%s - buffer=%p, size=%zu", __func__, buffer, size);

    auto& pesBuffer = m_storage.getPesBuffer();

    return pesBuffer.addPesPacket(buffer, size);
}

void DecoderImpl::invalidate()
{
    g_logger.trace("%s", __func__);

    m_presenter.invalidate();
    m_redrawNeeded = true;
}

void DecoderImpl::draw()
{
    g_logger.trace("%s", __func__);

    m_presenter.draw();
    m_redrawNeeded = false;
}

bool DecoderImpl::process()
{
    g_logger.trace("%s", __func__);

    if (!m_started)
    {
        g_logger.info("%s - not started, skipping", __func__);
        return false;
    }

    auto stcTime = m_timeProvider.getStc();

    bool anyDataProcessed = m_parser.process(stcTime);

    g_logger.trace("%s -> parsed=%d | redraw=%d", __func__, anyDataProcessed,
            m_redrawNeeded);

    return anyDataProcessed | m_redrawNeeded;
}

} // namespace dvbsubdecoder
