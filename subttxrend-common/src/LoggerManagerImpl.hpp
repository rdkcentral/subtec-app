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


#ifndef SUBTTXREND_COMMON_LOGGERMANAGERIMPL_HPP_
#define SUBTTXREND_COMMON_LOGGERMANAGERIMPL_HPP_

#include <mutex>
#include <string>
#include <map>
#include <memory>

#include "LoggerManager.hpp"
#include "LoggerLevel.hpp"
#include "LoggerBackendRdk.hpp"
#include "LoggerBackendStd.hpp"

namespace subttxrend
{
namespace common
{

/**
 * Manager of the loggers (implementation).
 */
class LoggerManagerImpl : public LoggerManager
{
public:
    /**
     * Returns manager singleton.
     *
     * @return
     *      Manager singleton.
     */
    static LoggerManagerImpl* getInstance();

    /** @copydoc LoggerManager::init */
    virtual void init(const ConfigProvider* configProvider) override;

    /** @copydoc LoggerManager::deinit */
    virtual void deinit() override;

    /**
     * Registers logging element.
     *
     * @param component
     *      Component name.
     * @param element
     *      Element name.
     *
     * @return
     *      Executor for given element.
     */
    const LoggerExecutor* registerElement(const std::string& component,
                                          const std::string& element);

    /**
     * Registers logging element.
     *
     * @param executor
     *      Executor returned by registerElement.
     */
    void unregisterElement(const LoggerExecutor* executor);

    /**
     * Checks if logging is enabled for given level.
     *
     * @param level
     *      Message level.
     * @param group
     *      Logging group.
     *
     * @retval true
     *      Logging is enabled.
     * @retval false
     *      Logging is disabled.
     */
    bool isEnabled(LoggerLevel level, const char* groupName) const;

    /**
     * Prints message to log.
     *
     * @param level
     *      Message level.
     * @param group
     *      Logging group.
     * @param component
     *      Component name.
     * @param element
     *      Element name.
     * @param message
     *      Message to log.
     */
    void printMessage(LoggerLevel level,
                      const char* groupName,
                      const std::string& component,
                      const std::string& element,
                      const std::string& message) const;

private:
    /**
     * Constructor.
     */
    LoggerManagerImpl();

    /**
     * Destructor.
     */
    ~LoggerManagerImpl();

    /**
     * Configure executor.
     *
     * Sets the executor enabled levels.
     *
     * @param executor
     *      Executor to configure.
     */
    void configureExecutor(LoggerExecutor* executor);

    /**
     * Gets logging group name this component belongs to.
     *
     * @param component
     *      Component to find group name for.
     */
    const char* componentToGroupName(const std::string& component) const;

    /** Mutex type. */
    typedef std::recursive_mutex Mutex;

    /** Mutex guard type. */
    typedef std::lock_guard<Mutex> MutexGuard;

    /** Map key. */
    typedef std::pair<std::string, std::string> ExecutorsMapKey;

    /** Mutex for synchronization. */
    mutable Mutex m_mutex;

    /** Configuration provider. */
    const ConfigProvider* m_configProvider;

    /** Executors collecton. */
    std::map<ExecutorsMapKey, std::unique_ptr<LoggerExecutor>> m_executors;

    /** STD backend. */
    LoggerBackendStd m_stdBackend;

    /** RDK backend. */
    LoggerBackendRdk m_rdkBackend;

    /** Current backend. */
    LoggerBackend* m_currentBackend;
};

} // namespace common
} // namespace subttxrend

#endif /*SUBTTXREND_COMMON_LOGGERMANAGERIMPL_HPP_*/
