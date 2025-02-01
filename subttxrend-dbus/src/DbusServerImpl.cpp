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


#include "DbusServerImpl.hpp"
#include "JsonHelper.hpp"

namespace subttxrend
{
namespace dbus
{

DbusServerImpl::DbusServerImpl(Executor &executor) :
        m_logger("Dbus", "DbusServerImpl"),
        m_executor(executor),
        m_dbusHandler(*this)
{
    m_logger.info("started");
}

bool DbusServerImpl::init()
{
    m_logger.info("initialization");

    return m_dbusHandler.init();
}

void DbusServerImpl::subtitleSetMuted(const std::string &request)
{
    bool muted = JsonHelper::DecodeSubtitleSetMuted(request);
    m_executor.subtitleSetMuted(muted);
}

std::string DbusServerImpl::subtitleGetStatus()
{
    SubtitleStatus subtitleStatus = m_executor.subtitleGetStatus();
    return JsonHelper::EncodeSubtitleStatusResponse(subtitleStatus);
}

void DbusServerImpl::teletextSetMuted(const std::string &request)
{
    bool muted = JsonHelper::DecodeTeletextSetMuted(request);
    m_executor.teletextSetMuted(muted);
}

std::string DbusServerImpl::teletextGetStatus()
{
    TeletextStatus ttxStatus = m_executor.teletextGetStatus();
    return JsonHelper::EncodeTeletextStatusResponse(ttxStatus);
}

} // namespace dbus
} // namespace subttxrend

