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

#include "LoggerManagerImpl.hpp"
#include "StringUtils.hpp"
#include "LoggerExecutor.hpp"

namespace subttxrend
{
namespace common
{

namespace {
thread_local int depth{0};
constexpr LoggerLevel TIMING_LEVEL = LoggerLevel::TRACE;
} /* namespace  */

Timing::Timing(const LoggerExecutor* exe, std::string s, void* ctx)
    : Timing{exe, std::move(s), TIMING_LEVEL, ctx}
{
}

Timing::Timing(const LoggerExecutor* exe, std::string s, LoggerLevel l, void* ctx)
    : executor{exe}
    , context{ctx}
    , str{std::move(s)}
    , level{l}
    , start{std::chrono::steady_clock::now()}
{
    if (executor->isEnabled(level)) {
        depth += 2;
        std::ostringstream os;
        if (context) {
            os << "[" << context << "] ";
        }
        os << std::setw(depth) << '+' << "[" << str << "]";
        executor->printMessage(level, os.str());
    }
}

Timing::Timing(Timing&& other)
    : executor{other.executor}
    , context{other.context}
    , level{other.level}
    , str{std::move(other.str)}
    , start{other.start}
{
}

Timing::~Timing()
{
    if (executor->isEnabled(level)) {
        auto now = std::chrono::steady_clock::now();
        auto diff = now - start;

        std::ostringstream os;
        if (context) {
            os << "[" << context << "] ";
        }
        os << std::setw(depth) << '-' << "[" << str << "] (" << std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(diff).count()) << "[us])";
        executor->printMessage(level, os.str());

        depth -= 2;
    }
}

Logger::Logger(const std::string& component, const std::string& element, void* ctx)
    : m_executor(LoggerManagerImpl::getInstance()->registerElement(component, element))
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
    LoggerManagerImpl::getInstance()->unregisterElement(m_executor);
}

bool Logger::isEnabled(LoggerLevel level) const
{
    return m_executor->isEnabled(level);
}

void Logger::sendMessage(LoggerLevel level, std::string const& s)
{
    m_executor->printMessage(level, s);
}

void Logger::fatal(const char* format,
                   ...)
{
    const auto level = LoggerLevel::FATAL;

    if (m_executor->isEnabled(level))
    {
        std::va_list arguments;
        va_start(arguments, format);

        std::string message = StringUtils::vformat(format, arguments);

        va_end(arguments);

        m_executor->printMessage(level, message);
    }
}

void Logger::error(const char* format,
                   ...)
{
    const auto level = LoggerLevel::ERROR;

    if (m_executor->isEnabled(level))
    {
        std::va_list arguments;
        va_start(arguments, format);

        std::string message = StringUtils::vformat(format, arguments);

        va_end(arguments);

        m_executor->printMessage(level, message);
    }
}

void Logger::warning(const char* format,
                     ...)
{
    const auto level = LoggerLevel::WARNING;

    if (m_executor->isEnabled(level))
    {
        std::va_list arguments;
        va_start(arguments, format);

        std::string message = StringUtils::vformat(format, arguments);

        va_end(arguments);

        m_executor->printMessage(level, message);
    }
}

void Logger::info(const char* format,
                  ...)
{
    const auto level = LoggerLevel::INFO;

    if (m_executor->isEnabled(level))
    {
        std::va_list arguments;
        va_start(arguments, format);

        std::string message = StringUtils::vformat(format, arguments);

        va_end(arguments);

        m_executor->printMessage(level, message);
    }
}

void Logger::debug(const char* format,
                   ...)
{
    const auto level = LoggerLevel::DEBUG;

    if (m_executor->isEnabled(level))
    {
        std::va_list arguments;
        va_start(arguments, format);

        std::string message = StringUtils::vformat(format, arguments);

        va_end(arguments);

        m_executor->printMessage(level, message);
    }
}

void Logger::trace(const char* format,
                   ...)
{
    const auto level = LoggerLevel::TRACE;

    if (m_executor->isEnabled(level))
    {
        std::va_list arguments;
        va_start(arguments, format);

        std::string message = StringUtils::vformat(format, arguments);

        va_end(arguments);

        m_executor->printMessage(level, message);
    }
}

} // namespace common
} // namespace subttxrend
