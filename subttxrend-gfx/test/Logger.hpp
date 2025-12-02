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


#ifndef SUBTTXREND_COMMON_LOGGER_HPP_
#define SUBTTXREND_COMMON_LOGGER_HPP_

#include <string>
#include <chrono>
#include <sstream>

#include "NonCopyable.hpp"

#define __LOGGER_FUNC__     __func__

#ifdef __GNUC__
#define LOGGER_CHECK_PRINTF_LIKE_ARGUMENTS      __attribute__ ((format (printf, 2, 3)))
#endif

namespace subttxrend
{
namespace common
{

enum class LoggerLevel : std::uint32_t
{
    /** Logging level - FATAL. */
    FATAL = (1 << 0),

    /** Logging level - ERROR. */
    ERROR = (1 << 1),

    /** Logging level - WARNING. */
    WARNING = (1 << 2),

    /** Logging level - INFO. */
    INFO = (1 << 3),

    /** Logging level - DEBUG. */
    DEBUG = (1 << 4),

    /** Logging level - TRACE. */
    TRACE = (1 << 5),
};


class LoggerExecutor;

struct Timing
{
    Timing(const LoggerExecutor* exe, std::string s, void* ctx);
    Timing(const LoggerExecutor* exe, std::string s, LoggerLevel level, void* ctx);
    ~Timing();
};

/**
 * Utility class for logging messages.
 *
 * May use RDK logger internally.
 */
class Logger : NonCopyable
{
  public:
    /**
     * Constructs logger for given module.
     *
     * @param component
     *      Component name.
     * @param element
     *      Element name.
     */
    Logger(const std::string& component, const std::string& element, void* ctx = nullptr);

    /**
     * Destructor.
     */
    virtual ~Logger();

    Timing timing(std::string s) const;
    Timing timing(std::string s, LoggerLevel level) const;
    bool isEnabled(LoggerLevel level) const;
    void sendMessage(LoggerLevel level, std::string const& s);

    template <class... Args>
    void makeMessage(LoggerLevel level, Args&&... args)
    {

    }
    template <class... Args>
    void osfatal(Args&&... args)
    {
        this->template makeMessage(LoggerLevel::FATAL, std::forward<Args>(args)...);
    }
    template <class... Args>
    void oserror(Args&&... args)
    {
        this->template makeMessage(LoggerLevel::ERROR, std::forward<Args>(args)...);
    }
    template <class... Args>
    void osdebug(Args&&... args)
    {
        this->template makeMessage(LoggerLevel::DEBUG, std::forward<Args>(args)...);
    }
    template <class... Args>
    void oswarning(Args&&... args)
    {
        this->template makeMessage(LoggerLevel::WARNING, std::forward<Args>(args)...);
    }
    template <class... Args>
    void osinfo(Args&&... args)
    {
        this->template makeMessage(LoggerLevel::INFO, std::forward<Args>(args)...);
    }
    template <class... Args>
    void ostrace(Args&&... args)
    {
        this->template makeMessage(LoggerLevel::TRACE, std::forward<Args>(args)...);
    }

    /**
     * Logs FATAL level message.
     *
     * @param format
     *      Printf-like format string.
     */
    void fatal(const char* format, ...) LOGGER_CHECK_PRINTF_LIKE_ARGUMENTS;
    /**
     * Logs ERROR level message.
     *
     * @param format
     *      Printf-like format string.
     */
    void error(const char* format, ...) LOGGER_CHECK_PRINTF_LIKE_ARGUMENTS;

    /**
     * Logs WARNING level message.
     *
     * @param format
     *      Printf-like format string.
     */
    void warning(const char* format, ...) LOGGER_CHECK_PRINTF_LIKE_ARGUMENTS;

    /**
     * Logs INFO level message.
     *
     * @param format
     *      Printf-like format string.
     */
    void info(const char* format, ...) LOGGER_CHECK_PRINTF_LIKE_ARGUMENTS;

    /**
     * Logs INFO level message.
     *
     * @param format
     *      Printf-like format string.
     */
    void debug(const char* format, ...) LOGGER_CHECK_PRINTF_LIKE_ARGUMENTS;

    /**
     * Logs TRACE level message.
     *
     * @param format
     *      Printf-like format string.
     */
    void trace(const char* format, ...) LOGGER_CHECK_PRINTF_LIKE_ARGUMENTS;

  private:
    void makeMessage(std::ostream&) {}

    template <class A, class... Args>
    void makeMessage(std::ostream& os, A&& a, Args&&... args)
    {
        makeMessage(os << a, std::forward<Args>(args)...);
    }

  private:
    /** Logger executor object. */
    const LoggerExecutor* const m_executor;
    void * const context;
};

} // namespace common
} // namespace subttxrend

#endif /*SUBTTXREND_COMMON_LOGGER_HPP_*/
