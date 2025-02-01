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


#ifndef SUBTTXREND_COMMON_LOGGEREXECUTOR_HPP_
#define SUBTTXREND_COMMON_LOGGEREXECUTOR_HPP_

#include "LoggerLevel.hpp"
#include "LoggerManagerImpl.hpp"

#include <atomic>

namespace subttxrend
{
namespace common
{

/**
 * Logger executor.
 */
class LoggerExecutor
{
public:
    /** Constructor. */
    LoggerExecutor(const LoggerManagerImpl* manager,
                   const char* groupName,
                   const std::string& component,
                   const std::string& element) :
            m_manager(manager),
            m_groupName(groupName),
            m_component(component),
            m_element(element),
            m_levelFlags(0)
    {
        // noop
    }

    /** Destructor. */
    ~LoggerExecutor() = default;

    /**
     * Returns group name.
     *
     * @return
     *      Group name.
     */
    const char* getGroupName() const
    {
        return m_groupName;
    }

    /**
     * Returns group name.
     *
     * @return
     *      Group name.
     */
    void setGroupName(const char* groupName)
    {
        m_groupName = groupName;
    }

    /**
     * Returns component name.
     *
     * @return
     *      Component name.
     */
    const std::string getComponent() const
    {
        return m_component;
    }

    /**
     * Returns element name.
     *
     * @return
     *      Element name.
     */
    const std::string& getElement() const
    {
        return m_element;
    }

    /**
     * Checks if logging is enabled for given level.
     *
     * @param level
     *      Message level.
     *
     * @retval true
     *      Logging is enabled.
     * @retval false
     *      Logging is disabled.
     */
    bool isEnabled(LoggerLevel level) const
    {
        auto levelBit = static_cast<std::uint32_t>(level);
        return ((m_levelFlags & levelBit) != 0);
    }

    /**
     * Prints message to log.
     *
     * @param level
     *      Message level.
     * @param message
     *      Message to log.
     */
    void printMessage(LoggerLevel level,
                      const std::string& message) const
    {
        m_manager->printMessage(level, getGroupName(), getComponent(), getElement(), message);
    }

    /**
     * Sets the value of enabled message level flags.
     *
     * @param levelFlags
     *      New value of level flags.
     */
    void setLevelFlags(std::uint32_t levelFlags)
    {
        m_levelFlags = levelFlags;
    }

private:
    /** Manager owning this executor. */
    const LoggerManagerImpl* const m_manager;

    /** Logging group. */
    const char* m_groupName;

    /** Component name. */
    const std::string m_component;

    /** Element name. */
    const std::string m_element;

    /** Current level flags. */
    std::atomic<uint32_t> m_levelFlags;
};

} // namespace common
} // namespace subttxrend

#endif /*SUBTTXREND_COMMON_LOGGEREXECUTOR_HPP_*/
