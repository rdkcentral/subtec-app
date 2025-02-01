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


#pragma once

#include <iostream>
#include <sstream>

#define __LOGGER_FUNC__     __func__

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

class Logger
{
public:
    Logger(const std::string& component, const std::string& element, void* ctx = nullptr) 
    { 
        //std::cout << "Logger constr for " << component << " " << element << "\n"; 
    }
    
    template <typename T>
    void sendMessage(std::ostream& o, T t)
    {
        o << t;
    }
    template <typename T, typename ... Args>
    void sendMessage(std::ostream& o, T t, Args&&... args)
    {
        sendMessage(o, t);
        sendMessage(o, args...);
            
    }
    template<typename... Args>
    void sendRecursiveLog(Args... args)
    {
        std::ostringstream oss;
        sendMessage(oss, args...);
        std::cout << oss.str() << "\n";
    }
    
    template <typename... Args>
    void osfatal(Args&&... args)
    {
        sendRecursiveLog(std::forward<Args>(args)...);
    }
    template <typename... Args>
    void oserror(Args&&... args)
    {
        sendRecursiveLog(std::forward<Args>(args)...);
    }
    template <typename... Args>
    void osdebug(Args&&... args)
    {
        sendRecursiveLog(std::forward<Args>(args)...);
    }
    template <typename... Args>
    void oswarning(Args&&... args)
    {
        sendRecursiveLog(std::forward<Args>(args)...);
    }
    template <typename... Args>
    void osinfo(Args&&... args)
    {
        sendRecursiveLog(std::forward<Args>(args)...);
    }
    template <typename... Args>
    void ostrace(Args&&... args)
    {
        sendRecursiveLog(std::forward<Args>(args)...);
    }

    /**
     * Logs FATAL level message.
     *
     * @param format
     *      Printf-like format string.
     */
    void fatal(const char* format, ...) {  }
    /**
     * Logs ERROR level message.
     *
     * @param format
     *      Printf-like format string.
     */
    void error(const char* format, ...)  {  };

    /**
     * Logs WARNING level message.
     *
     * @param format
     *      Printf-like format string.
     */
    void warning(const char* format, ...)  {  };

    /**
     * Logs INFO level message.
     *
     * @param format
     *      Printf-like format string.
     */
    void info(const char* format, ...)  {  };

    /**
     * Logs INFO level message.
     *
     * @param format
     *      Printf-like format string.
     */
    void debug(const char* format, ...)  {  };

    /**
     * Logs TRACE level message.
     *
     * @param format
     *      Printf-like format string.
     */
    void trace(const char* format, ...)  {  };


};

}
}
