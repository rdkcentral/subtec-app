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


#ifndef SUBTTXREND_COMMON_LOGGERMANAGER_HPP_
#define SUBTTXREND_COMMON_LOGGERMANAGER_HPP_

#include <string>

#include "NonCopyable.hpp"
#include "ConfigProvider.hpp"

namespace subttxrend
{
namespace common
{

class LoggerExecutor;

/**
 * Manager of the loggers.
 *
 * By default the loggers would print messages on standard output and
 * all messages would be printed.
 *
 * This behavior could be altered by calling the init() method and providing
 * configuration. The configuration keys are as follow:
 * - Backend selection:
 *      - BACKEND = std | rdk
 * - RDK backend configuration:
 *      - BACKEND_RDK_CONFIG_FILE = path to RDK logger configuration file
 *      - BACKEND_RDK_MODULE_NAME = RDK module name to use
 * - Enabled message levels. The levels could be enabled by specifying words
 *   separated by spaces. The words are: FATAL, ERROR, WARNING, INFO, DEBUG,
 *   TRACE, ALL. For a logger the configuration first tries to take element
 *   configuration, if not found then it tries to use component configuration,
 *   and if not found the default configuration.
 *      - LEVELS_DEFAULT = level words (defaults for all loggers)
 *      - LEVELS.component = level words (defaults for loggers withing component)
 *      - LEVELS.component.element = level words (for specific logger)
 */
class LoggerManager : NonCopyable
{
public:
    /**
     * Returns manager singleton.
     *
     * @return
     *      Manager singleton.
     */
    static LoggerManager* getInstance();

    /**
     * (Re)initializes the logging subsystem.
     *
     * @param configProvider
     *      The config provider to use.
     *      The provider must remain valid until next call to configure()
     *      or deinit().
     */
    virtual void init(const ConfigProvider* configProvider) = 0;

    /**
     * Deinitializes the logging subsystem.
     */
    virtual void deinit() = 0;

protected:
    /**
     * Constructor.
     */
    LoggerManager() = default;

    /**
     * Destructor.
     */
    virtual ~LoggerManager() = default;
};

} // namespace common
} // namespace subttxrend

#endif /*SUBTTXREND_COMMON_LOGGERMANAGER_HPP_*/
