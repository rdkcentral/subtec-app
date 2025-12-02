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


#include "Logger.hpp"

#include <cstdarg>
#include <iostream>
#include <iomanip>
#include <sstream>

//class LoggerExecutor {};

namespace subttxrend
{
namespace common
{

namespace {
thread_local int depth{0};
constexpr LoggerLevel TIMING_LEVEL = LoggerLevel::TRACE;
} /* namespace  */

Timing::Timing(const LoggerExecutor* exe, std::string s, void* ctx)

{
}

Timing::Timing(const LoggerExecutor* exe, std::string s, LoggerLevel l, void* ctx)
{

}

Timing::~Timing()
{
}

Logger::Logger(const std::string& component, const std::string& element, void* ctx)
    : m_executor()
    , context{ctx}
{
    // noop
}

Timing Logger::timing(std::string s) const
{
    return Timing(m_executor, std::move(s), context);
}

Timing Logger::timing(std::string s, LoggerLevel level) const
{
    return Timing(m_executor, std::move(s), level, context);
}

Logger::~Logger()
{
    // noop
}

bool Logger::isEnabled(LoggerLevel level) const
{
    return true;
}

void Logger::sendMessage(LoggerLevel level, std::string const& s)
{
    std::cout << s;
}

void Logger::fatal(const char* format,
                   ...)
{

}

void Logger::error(const char* format,
                   ...)
{
}

void Logger::warning(const char* format,
                     ...)
{
}

void Logger::info(const char* format,
                  ...)
{
}

void Logger::debug(const char* format,
                   ...)
{
}

void Logger::trace(const char* format,
                   ...)
{

}

} // namespace common
} // namespace subttxrend
