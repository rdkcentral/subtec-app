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
#include <map>
#include <string>

namespace
{

const std::string& levelToString(rdk_LogLevel level)
{
    static std::map<rdk_LogLevel, std::string> levelNames;
    static const std::string UNKNOWN_NAME("?????");

    if (levelNames.size() == 0)
    {
        levelNames.insert(std::make_pair(RDK_LOG_FATAL, "FATAL"));
        levelNames.insert(std::make_pair(RDK_LOG_ERROR, "ERROR"));
        levelNames.insert(std::make_pair(RDK_LOG_WARN, "WARN_"));
        levelNames.insert(std::make_pair(RDK_LOG_NOTICE, "NOTIC"));
        levelNames.insert(std::make_pair(RDK_LOG_INFO, "INFO_"));
        levelNames.insert(std::make_pair(RDK_LOG_DEBUG, "DEBUG"));
        levelNames.insert(std::make_pair(RDK_LOG_TRACE1, "TRACE"));
        levelNames.insert(std::make_pair(RDK_LOG_TRACE2, "TRACE"));
        levelNames.insert(std::make_pair(RDK_LOG_TRACE3, "TRACE"));
        levelNames.insert(std::make_pair(RDK_LOG_TRACE4, "TRACE"));
        levelNames.insert(std::make_pair(RDK_LOG_TRACE5, "TRACE"));
        levelNames.insert(std::make_pair(RDK_LOG_TRACE6, "TRACE"));
        levelNames.insert(std::make_pair(RDK_LOG_TRACE7, "TRACE"));
        levelNames.insert(std::make_pair(RDK_LOG_TRACE8, "TRACE"));
        levelNames.insert(std::make_pair(RDK_LOG_TRACE9, "TRACE"));
    }

    auto nameIter = levelNames.find(level);
    if (nameIter != levelNames.end())
    {
        return nameIter->second;
    }
    else
    {
        return UNKNOWN_NAME;
    }
}

}

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

    printf("%s:%s - %s\n", levelToString(level).c_str(), module, buffer);
}
