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


#include "LoggerManager.hpp"

#include <array>
#include <cassert>
#include <cstdarg>
#include <iostream>
#include <unordered_map>

#include "LoggerExecutor.hpp"
#include "LoggingGroup.hpp"
#include "StringUtils.hpp"
#include "Logger.hpp"

namespace subttxrend
{
namespace common
{

namespace
{

Logger g_logger("Common", "LoggerManager");

} // namespace

LoggerManager* LoggerManager::getInstance()
{
    return LoggerManagerImpl::getInstance();
}

LoggerManagerImpl* LoggerManagerImpl::getInstance()
{
    static LoggerManagerImpl* theInstance = nullptr;

    if (!theInstance)
    {
        theInstance = new LoggerManagerImpl();
    }

    return theInstance;
}

LoggerManagerImpl::LoggerManagerImpl() :
        m_configProvider(nullptr),
        m_currentBackend(&m_stdBackend)
{
    // noop
}

LoggerManagerImpl::~LoggerManagerImpl()
{
    // noop
}

void LoggerManagerImpl::init(const ConfigProvider* configProvider)
{
    MutexGuard guard(m_mutex);

    deinit();

    m_configProvider = configProvider;

    if (configProvider)
    {
        auto backendName = configProvider->get("BACKEND", "std");
        if (backendName == "rdk")
        {
            auto configFile = StringUtils::trim(
                    configProvider->get("BACKEND_RDK_CONFIG_FILE"));

            if (configFile.length() > 0)
            {
                if (m_rdkBackend.init(configFile))
                {
                    m_currentBackend = &m_rdkBackend;
                }
                else
                {
                    g_logger.warning("%s - Cannot initialize RDK backend.",
                            __func__);
                }
            }
            else
            {
                g_logger.warning(
                        "%s - Missing RDK backend params file=%s",
                        __func__, configFile.c_str());
            }
        }
        else if (backendName == "std")
        {
            // noop
        }
        else
        {
            g_logger.warning("%s - Unknown backend name: %s", __func__,
                    backendName.c_str());
        }
    }

    // reconfigure executors
    for (const auto& entry : m_executors)
    {
        configureExecutor(entry.second.get());
    }
}

void LoggerManagerImpl::deinit()
{
    MutexGuard guard(m_mutex);

    m_configProvider = nullptr;
    m_currentBackend = &m_stdBackend;
    m_rdkBackend.deinit();

}

bool LoggerManagerImpl::isEnabled(LoggerLevel level, const char* groupName) const
{
    MutexGuard guard(m_mutex);

    return m_currentBackend->isEnabled(level, groupName);
}

void LoggerManagerImpl::printMessage(LoggerLevel level,
                                     const char* groupName,
                                     const std::string& component,
                                     const std::string& element,
                                     const std::string& message) const
{
    MutexGuard guard(m_mutex);

    m_currentBackend->printMessage(level, groupName, component, element, message);
}

const LoggerExecutor* LoggerManagerImpl::registerElement(const std::string& component,
                                                         const std::string& element)
{
    MutexGuard guard(m_mutex);

    auto elementKey = std::make_pair(component, element);

    auto iter = m_executors.find(elementKey);
    if (iter != m_executors.end())
    {
        return iter->second.get();
    }

    std::unique_ptr<LoggerExecutor> executor(
            new LoggerExecutor(this, componentToGroupName(component), component, element));

    auto executorPtr = executor.get();

    m_executors.insert(std::make_pair(elementKey, std::move(executor)));

    configureExecutor(executorPtr);

    return executorPtr;
}

void LoggerManagerImpl::unregisterElement(const LoggerExecutor* /*executor*/)
{
    // noop
}

void LoggerManagerImpl::configureExecutor(LoggerExecutor* executor)
{
    std::uint32_t levelFlags = ~0;

    executor->setGroupName(componentToGroupName(executor->getComponent()));

    if (m_configProvider)
    {
        bool hasValue = false;
        std::string configKey;

        configKey = "LEVELS." + executor->getComponent() + "."
                + executor->getElement();
        hasValue = m_configProvider->hasValue(configKey);
        if (!hasValue)
        {
            configKey = "LEVELS." + executor->getComponent();
            hasValue = m_configProvider->hasValue(configKey);
            if (!hasValue)
            {
                configKey = "LEVELS_DEFAULT";
                hasValue = m_configProvider->hasValue(configKey);
            }
        }

        if (hasValue)
        {
            levelFlags = 0;

            auto levelWords = m_configProvider->getArray(configKey, " ");
            for (auto word : levelWords)
            {
                uint32_t flag = 0;
                bool setLowerFlags = false;

                if ((word.length() > 0) && (word.back() == '+'))
                {
                    word = word.substr(0, word.length() - 1);
                    setLowerFlags = true;
                }

                if (word == "FATAL")
                {
                    flag = static_cast<std::uint32_t>(LoggerLevel::FATAL);
                }
                else if (word == "ERROR")
                {
                    flag = static_cast<std::uint32_t>(LoggerLevel::ERROR);
                }
                else if (word == "WARNING")
                {
                    flag = static_cast<std::uint32_t>(LoggerLevel::WARNING);
                }
                else if (word == "INFO")
                {
                    flag = static_cast<std::uint32_t>(LoggerLevel::INFO);
                }
                else if (word == "DEBUG")
                {
                    flag = static_cast<std::uint32_t>(LoggerLevel::DEBUG);
                }
                else if (word == "TRACE")
                {
                    flag = static_cast<std::uint32_t>(LoggerLevel::TRACE);
                }
                else if (word == "ALL")
                {
                    flag = ~0;
                }
                else
                {
                    // noop (no logging as it may potentially deadlock)
                }

                if (setLowerFlags)
                {
                    while (flag > 0)
                    {
                        levelFlags |= flag;
                        flag >>= 1;
                    }
                }
                else
                {
                    levelFlags |= flag;
                }
            }

            LoggerLevelFlags backendLevel = 0;
            for (auto numLogLevel = static_cast<uint32_t>(LoggerLevel::TRACE); numLogLevel != 0; numLogLevel >>= 1)
            {
                auto logLevel = static_cast<LoggerLevel>(numLogLevel);
                if (m_currentBackend->isEnabled(logLevel, executor->getGroupName()))
                {
                    backendLevel |= numLogLevel;
                }
            }

            levelFlags &= backendLevel;

            m_currentBackend->printMessage(LoggerLevel::INFO,
                m_currentBackend->getGroupName(CORE),
                executor->getComponent(),
                executor->getElement(), std::string(" Setting logger level: ") + executor->getComponent()
                    + " " + std::to_string(levelFlags));
        }
    }

    executor->setLevelFlags(levelFlags);
}

const char* LoggerManagerImpl::componentToGroupName(const std::string& component) const
{
    static std::unordered_map<std::string, LoggingGroup> const componentToGroup = {
            {"App", CORE},
            {"Common", CORE},
            {"Dbus", CORE},
            {"DvbSub", REND},
            {"DvbSubDecoder", REND},
            {"Gfx", GFX},
            {"GfxEngine", GFX},
            {"Protocol", CORE},
            {"SockSrc", CORE},
            {"TtmlEngine", REND},
            {"WebvttEngine", REND},
            {"TtxDecoder", REND},
            {"Ttxt", REND},
            {"Scte", REND},
            {"ClosedCaptions", REND}
    };

    auto it = componentToGroup.find(component);
    assert((it != componentToGroup.end()) && "component group not found");

    return m_currentBackend->getGroupName(it->second);
}

} // namespace common
} // namespace subttxrend
