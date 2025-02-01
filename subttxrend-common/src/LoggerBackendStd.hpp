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


#ifndef SUBTTXREND_COMMON_LOGGERBACKENDSTD_HPP_
#define SUBTTXREND_COMMON_LOGGERBACKENDSTD_HPP_

#include "NonCopyable.hpp"
#include "LoggerBackend.hpp"

namespace subttxrend
{
namespace common
{

/**
 * Utility class for logging messages.
 *
 * Uses stdout internally.
 */
class LoggerBackendStd : public LoggerBackend,
                         private NonCopyable
{
public:
    /** Constructor. */
    LoggerBackendStd() = default;

    /** Destructor. */
    virtual ~LoggerBackendStd() = default;

    /** @copydoc LoggerBackend::printMessage */
    virtual void printMessage(LoggerLevel level,
                              const char* groupName,
                              const std::string& component,
                              const std::string& element,
                              const std::string& message) override;

    /** @copydoc LoggerBackend::isEnabled */
    virtual bool isEnabled(LoggerLevel level, const char* groupName) const override;

    /** @copydoc LoggerBackend::getGroupName */
    virtual const char* getGroupName(LoggingGroup group) const override;

};

} // namespace common
} // namespace subttxrend

#endif /*SUBTTXREND_COMMON_LOGGERBACKENDRDK_HPP_*/
