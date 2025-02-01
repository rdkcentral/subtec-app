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


#ifndef SUBTTXREND_COMMON_LOGGERBACKEND_HPP_
#define SUBTTXREND_COMMON_LOGGERBACKEND_HPP_

#include "LoggerLevel.hpp"
#include "LoggingGroup.hpp"

namespace subttxrend
{
namespace common
{

/**
 * Backend for Logger.
 */
class LoggerBackend
{
public:
    /** Constructor. */
    LoggerBackend() = default;

    /** Destructor. */
    virtual ~LoggerBackend() = default;

    /**
     * Prints message to log.
     *
     * @param level
     *      Message level.
     * @param groupName
     *      Logging group name.
     * @param component
     *      Component name.
     * @param element
     *      Element name.
     * @param message
     *      Message to log.
     */
    virtual void printMessage(LoggerLevel level,
                              const char* groupName,
                              const std::string& component,
                              const std::string& element,
                              const std::string& message) = 0;

    /**
     * Checks if logging is enabled for "level,module" pair.
     *
     * @param groupName
     *      Logging group name.
     * @param level
     *      Message level.
     *
     * @retval true
     *      Logging is enabled.
     * @retval false
     *      Logging is disabled.
     */
    virtual bool isEnabled(LoggerLevel level, const char* groupName) const = 0;

    /**
     * Group name getter.
     *
     * @param group
     *      Logging group.

     * @return
     *      Logging group name.
     */
    virtual const char* getGroupName(LoggingGroup group) const = 0;

};

} // namespace common
} // namespace subttxrend

#endif /*SUBTTXREND_COMMON_LOGGERBACKEND_HPP_*/
