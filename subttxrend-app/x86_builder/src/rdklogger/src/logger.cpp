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


#include <rdk_debug.h>
#include <cstdarg>
#include <cstdio>
#include <string>

namespace
{

const std::string& levelToString(rdk_LogLevel level)
{
    static const std::string FATAL_NAME("FATAL");
    static const std::string ERROR_NAME("ERROR");
    static const std::string WARN_NAME("WARN_");
    static const std::string NOTICE_NAME("NOTIC");
    static const std::string INFO_NAME("INFO_");
    static const std::string DEBUG_NAME("DEBUG");
    static const std::string TRACE_NAME("TRACE");
    static const std::string UNKNOWN_NAME("?????");

    switch (level)
    {
        case RDK_LOG_FATAL:  return FATAL_NAME;
        case RDK_LOG_ERROR:  return ERROR_NAME;
        case RDK_LOG_WARN:   return WARN_NAME;
        case RDK_LOG_NOTICE: return NOTICE_NAME;
        case RDK_LOG_INFO:   return INFO_NAME;
        case RDK_LOG_DEBUG:  return DEBUG_NAME;
        default:
            break;
    }

    if (level == RDK_LOG_TRACE)
    {
        return TRACE_NAME;
    }

    return UNKNOWN_NAME;
}

} // namespace

rdk_Error rdk_logger_init(const char* debugConfigFile)
{
    return 0;
}

rdk_Error rdk_logger_deinit()
{
    return 0;
}

rdk_logger_Bool rdk_logger_is_logLevel_enabled(const char *module,
                                rdk_LogLevel level)
{
    return true;
}

void RDK_LOG(rdk_LogLevel level,
             const char *module,
             const char *format,
             ...)
{
    char buffer[32 * 1024];

    std::va_list arguments;

    va_start(arguments, format);
    vsnprintf(buffer, sizeof(buffer) - 1, format, arguments);
    buffer[sizeof(buffer) - 1] = 0;
    va_end(arguments);

    printf("%s\n", buffer);
}
