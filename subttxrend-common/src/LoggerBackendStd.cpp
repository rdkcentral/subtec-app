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


#include "LoggerBackendStd.hpp"

#include <string>
#include <iostream>
#include <cstdio>
#include <map>
#include <cstdint>

#include <sys/time.h>

namespace subttxrend
{
namespace common
{

namespace
{

std::uint64_t getCurrentTimestampMs()
{
    struct timeval tv = timeval();
    gettimeofday(&tv, NULL);

    const std::uint64_t clockSec = tv.tv_sec;
    const std::uint64_t clockUsec = tv.tv_usec;

    return (clockSec * 1000) + (clockUsec / 1000);
}

const char* levelToString(LoggerLevel level)
{
    switch (level) {
        case LoggerLevel::FATAL:
            return "FATAL";
        case LoggerLevel::ERROR:
            return "ERROR";
        case LoggerLevel::WARNING:
            return "WARNING";
        case LoggerLevel::INFO:
            return "INFO";
        case LoggerLevel::DEBUG:
            return "DEBUG";
        case LoggerLevel::TRACE:
        default:
            break;
    }
    return "?????";
}
} // namespace

void LoggerBackendStd::printMessage(LoggerLevel level,
                                    const char* groupName,
                                    const std::string& component,
                                    const std::string& element,
                                    const std::string& message)
{
    static auto firstTime = getCurrentTimestampMs();

    if (isEnabled(level, groupName))
    {
        auto currentTime = getCurrentTimestampMs();
        auto timeDiff = currentTime - firstTime;

        printf("%08u %s: [%s:%s] %s\n", static_cast<std::uint32_t>(timeDiff),
                levelToString(level), component.c_str(),
                element.c_str(), message.c_str());
    }
}

bool LoggerBackendStd::isEnabled(LoggerLevel /* unused */, const char* /* unused */) const
{
    return true;
}

const char* LoggerBackendStd::getGroupName(LoggingGroup group) const
{
    return nullptr;
}

} // namespace common
} // namespace subttxrend
