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


#include "LoggerBackendRdk.hpp"

#include "NonCopyable.hpp"
#include "LoggerBackend.hpp"

#include <rdk_debug.h>

#include <array>
#include <cassert>

namespace subttxrend
{
namespace common
{

namespace
{

rdk_LogLevel convertLevel(LoggerLevel level)
{
    switch (level)
    {
    case LoggerLevel::FATAL:
        return RDK_LOG_FATAL;
    case LoggerLevel::ERROR:
        return RDK_LOG_ERROR;
    case LoggerLevel::WARNING:
        return RDK_LOG_WARN;
    case LoggerLevel::INFO:
        return RDK_LOG_INFO;
    case LoggerLevel::DEBUG:
        return RDK_LOG_DEBUG;
    case LoggerLevel::TRACE:
        return RDK_LOG_TRACE;
    default:
        return RDK_LOG_TRACE;
    }
}

std::array<const char *, 3> const moduleNames = {
        "LOG.RDK.SUBS.CORE",
        "LOG.RDK.SUBS.GFX",
        "LOG.RDK.SUBS.REND"
};

} // namespace anonymous

LoggerBackendRdk::LoggerBackendRdk() :
        m_initialized(false)
{
    // noop
}

LoggerBackendRdk::~LoggerBackendRdk()
{
    deinit();
}

bool LoggerBackendRdk::isInitialized() const
{
    return m_initialized;
}

bool LoggerBackendRdk::init(const std::string& rdkConfigFileName)
{
    if (m_initialized)
    {
        return true;
    }

    if (rdk_logger_init(rdkConfigFileName.c_str()) != 0)
    {
        return false;
    }

    m_initialized = true;

    return true;
}

void LoggerBackendRdk::deinit()
{
    if (m_initialized)
    {
        (void) rdk_logger_deinit();

        m_initialized = false;
    }
}

void LoggerBackendRdk::printMessage(LoggerLevel level,
                                    const char* groupName,
                                    const std::string& component,
                                    const std::string& element,
                                    const std::string& message)
{
    assert(groupName && "group name is null");

    RDK_LOG(convertLevel(level), groupName, "[%s::%s] %s\n",
            component.c_str(), element.c_str(), message.c_str());
}

bool LoggerBackendRdk::isEnabled(LoggerLevel level, const char* groupName) const
{
    return rdk_logger_is_logLevel_enabled(groupName, convertLevel(level));
}

const char* LoggerBackendRdk::getGroupName(LoggingGroup group) const
{
    assert((group>= 0) && (group < static_cast<int>(moduleNames.size())));
    return moduleNames[group];
}

} // namespace common
} // namespace subttxrend
