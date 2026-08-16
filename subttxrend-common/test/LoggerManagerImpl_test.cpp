/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
*/

#include <cppunit/extensions/HelperMacros.h>

#include <atomic>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "LoggerManagerImpl.hpp"
#include "ConfigProvider.hpp"
#include "LoggerExecutor.hpp"
#include "LoggerLevel.hpp"

using subttxrend::common::LoggerManagerImpl;
using subttxrend::common::ConfigProvider;
using subttxrend::common::LoggerExecutor;
using subttxrend::common::LoggerLevel;

/**
 * Test ConfigProvider implementation for LoggerManagerImpl testing
 */
class TestConfigProvider : public ConfigProvider
{
public:
    void setValue(const std::string& key, const std::string& value)
    {
        m_values[key] = value;
    }

    void clear()
    {
        m_values.clear();
    }

protected:
    const char* getValue(const std::string& key) const override
    {
        auto it = m_values.find(key);
        if (it != m_values.end())
        {
            return it->second.c_str();
        }
        return nullptr;
    }

private:
    std::map<std::string, std::string> m_values;
};

class LoggerManagerImplTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(LoggerManagerImplTest);
    CPPUNIT_TEST(testGetInstanceReturnsSingleton);
    CPPUNIT_TEST(testRegisterElementIdempotent);
    CPPUNIT_TEST(testRegisterDifferentElementsDistinct);
    CPPUNIT_TEST(testRegisterEmptyElementName);
    CPPUNIT_TEST(testRegisterLongNames);
    CPPUNIT_TEST(testRegisterSpecialCharacters);
    CPPUNIT_TEST(testInitNullConfigProvider);
    CPPUNIT_TEST(testInitStdBackendDefault);
    CPPUNIT_TEST(testInitExplicitStdBackend);
    CPPUNIT_TEST(testInitUnknownBackendFallsBack);
    CPPUNIT_TEST(testInitRdkBackendMissingConfigFile);
    CPPUNIT_TEST(testInitRdkBackendEmptyConfigFile);
    CPPUNIT_TEST(testInitRdkBackendWithConfig);
    CPPUNIT_TEST(testDeinitResetsToStdBackend);
    CPPUNIT_TEST(testDeinitWithNullConfigProvider);
    CPPUNIT_TEST(testLevelResolutionElementOverridesComponent);
    CPPUNIT_TEST(testLevelResolutionComponentFallbackToDefault);
    CPPUNIT_TEST(testLevelResolutionDefaultOnly);
    CPPUNIT_TEST(testLevelResolutionNoConfig);
    CPPUNIT_TEST(testLevelTokenFatal);
    CPPUNIT_TEST(testLevelTokenError);
    CPPUNIT_TEST(testLevelTokenWarning);
    CPPUNIT_TEST(testLevelTokenInfo);
    CPPUNIT_TEST(testLevelTokenDebug);
    CPPUNIT_TEST(testLevelTokenTrace);
    CPPUNIT_TEST(testLevelTokenAll);
    CPPUNIT_TEST(testLevelTokenWithPlus);
    CPPUNIT_TEST(testLevelTokenErrorPlusIncludesLower);
    CPPUNIT_TEST(testLevelTokenMultipleTokens);
    CPPUNIT_TEST(testLevelTokenInvalidIgnored);
    CPPUNIT_TEST(testLevelTokenEmptyString);
    CPPUNIT_TEST(testLevelTokenWhitespaceOnly);
    CPPUNIT_TEST(testComponentToGroupMappingKnownComponents);
    CPPUNIT_TEST(testIsEnabledDelegatesToBackend);
    CPPUNIT_TEST(testPrintMessageDelegatesToBackend);
    CPPUNIT_TEST(testExecutorReconfigurationAfterInit);
    CPPUNIT_TEST(testReinitChangesLevels);
    CPPUNIT_TEST(testConcurrentRegisterElement);
    CPPUNIT_TEST(testConcurrentDuplicateRegister);
    CPPUNIT_TEST(testInitDuringRegisterElement);
    CPPUNIT_TEST(testExecutorsRemainValid);
    CPPUNIT_TEST(testUnregisterElementIsNoop);
    CPPUNIT_TEST(testConfigurationChangePropagatesToExecutors);
    CPPUNIT_TEST(testLevelConfigurationWithNoConfigProvider);
CPPUNIT_TEST_SUITE_END();

private:
    TestConfigProvider m_configProvider;

public:
    void setUp()
    {
        m_configProvider.clear();
        // Reset manager to clean state
        LoggerManagerImpl::getInstance()->deinit();
    }

    void tearDown()
    {
        m_configProvider.clear();
        LoggerManagerImpl::getInstance()->deinit();
    }

    void testGetInstanceReturnsSingleton()
    {
        LoggerManagerImpl* instance1 = LoggerManagerImpl::getInstance();
        LoggerManagerImpl* instance2 = LoggerManagerImpl::getInstance();

        CPPUNIT_ASSERT(instance1 != nullptr);
        CPPUNIT_ASSERT_EQUAL(instance1, instance2);
    }

    void testRegisterElementIdempotent()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        const LoggerExecutor* executor1 = manager->registerElement("App", "test");
        const LoggerExecutor* executor2 = manager->registerElement("App", "test");

        CPPUNIT_ASSERT(executor1 != nullptr);
        CPPUNIT_ASSERT_EQUAL(executor1, executor2);
    }

    void testRegisterDifferentElementsDistinct()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        const LoggerExecutor* executor1 = manager->registerElement("App", "test1");
        const LoggerExecutor* executor2 = manager->registerElement("App", "test2");
        const LoggerExecutor* executor3 = manager->registerElement("Common", "test1");

        CPPUNIT_ASSERT(executor1 != nullptr);
        CPPUNIT_ASSERT(executor2 != nullptr);
        CPPUNIT_ASSERT(executor3 != nullptr);
        CPPUNIT_ASSERT(executor1 != executor2);
        CPPUNIT_ASSERT(executor1 != executor3);
        CPPUNIT_ASSERT(executor2 != executor3);
    }

    void testRegisterEmptyElementName()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        const LoggerExecutor* executor = manager->registerElement("App", "");
        CPPUNIT_ASSERT(executor != nullptr);
        CPPUNIT_ASSERT_EQUAL(std::string(""), executor->getElement());
    }

    void testRegisterLongNames()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        // Component must be one of the known mapped names; stress only element length
        std::string longElement(1000, 'E');
        const LoggerExecutor* executor = manager->registerElement("App", longElement);
        CPPUNIT_ASSERT(executor != nullptr);
        CPPUNIT_ASSERT_EQUAL(std::string("App"), executor->getComponent());
        CPPUNIT_ASSERT_EQUAL(longElement, executor->getElement());
    }

    void testRegisterSpecialCharacters()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        // Only element may contain arbitrary characters; component must be mapped
        const LoggerExecutor* executor = manager->registerElement("App", "tëst!");
        CPPUNIT_ASSERT(executor != nullptr);
        CPPUNIT_ASSERT_EQUAL(std::string("App"), executor->getComponent());
        CPPUNIT_ASSERT_EQUAL(std::string("tëst!"), executor->getElement());
    }

    void testInitNullConfigProvider()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        // Should not crash and should use std backend
        CPPUNIT_ASSERT_NO_THROW(manager->init(nullptr));

        // Register an element to verify it works
        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);
    }

    void testInitStdBackendDefault()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        // No BACKEND key should default to std
        manager->init(&m_configProvider);

        // Should work normally
        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);
    }

    void testInitExplicitStdBackend()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("BACKEND", "std");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);
    }

    void testInitUnknownBackendFallsBack()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("BACKEND", "unknown_backend");

        // Should not crash and should fall back to std
        CPPUNIT_ASSERT_NO_THROW(manager->init(&m_configProvider));

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);
    }

    void testInitRdkBackendMissingConfigFile()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("BACKEND", "rdk");
        // No BACKEND_RDK_CONFIG_FILE key

        // Should fall back to std backend
        CPPUNIT_ASSERT_NO_THROW(manager->init(&m_configProvider));

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);
        CPPUNIT_ASSERT(executor->getGroupName() == nullptr);
    }

    void testInitRdkBackendEmptyConfigFile()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("BACKEND", "rdk");
        m_configProvider.setValue("BACKEND_RDK_CONFIG_FILE", "");

        // Should fall back to std backend
        CPPUNIT_ASSERT_NO_THROW(manager->init(&m_configProvider));

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);
        CPPUNIT_ASSERT(executor->getGroupName() == nullptr);
    }

    void testInitRdkBackendWithConfig()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("BACKEND", "rdk");
        m_configProvider.setValue("BACKEND_RDK_CONFIG_FILE", "/test/config.ini");

        // The test RDK stub accepts the path and selects the RDK backend.
        CPPUNIT_ASSERT_NO_THROW(manager->init(&m_configProvider));

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);
        CPPUNIT_ASSERT_EQUAL(std::string("LOG.RDK.SUBS.CORE"),
            std::string(executor->getGroupName()));
    }

    void testDeinitResetsToStdBackend()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("BACKEND", "rdk");
        m_configProvider.setValue("BACKEND_RDK_CONFIG_FILE", "/test/config.ini");
        manager->init(&m_configProvider);
        const LoggerExecutor* rdkExecutor = manager->registerElement("App", "deinit-rdk");
        CPPUNIT_ASSERT_EQUAL(std::string("LOG.RDK.SUBS.CORE"),
                std::string(rdkExecutor->getGroupName()));
        manager->deinit();

        // Should still work after deinit
        const LoggerExecutor* executor = manager->registerElement("App", "deinit-std");
        CPPUNIT_ASSERT(executor != nullptr);
        CPPUNIT_ASSERT(executor->getGroupName() == nullptr);
    }

    void testDeinitWithNullConfigProvider()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        manager->init(nullptr);

        // Should not crash
        CPPUNIT_ASSERT_NO_THROW(manager->deinit());
    }

    void testLevelResolutionElementOverridesComponent()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS.App", "ERROR");
        m_configProvider.setValue("LEVELS.App.test", "DEBUG");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        // Should use element-specific level (DEBUG) not component level (ERROR)
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::DEBUG));
    }

    void testLevelResolutionComponentFallbackToDefault()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS.App", "WARNING");
        m_configProvider.setValue("LEVELS_DEFAULT", "DEBUG");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        // Should use component level (WARNING)
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::WARNING));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::DEBUG));
    }

    void testLevelResolutionDefaultOnly()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS_DEFAULT", "INFO");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        // Should use default level
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::INFO));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::DEBUG));
    }

    void testLevelResolutionNoConfig()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        // Should have all levels enabled when no config
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::FATAL));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::ERROR));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::WARNING));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::INFO));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::DEBUG));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::TRACE));
    }

    void testLevelTokenFatal()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS_DEFAULT", "FATAL");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::FATAL));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::ERROR));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::WARNING));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::INFO));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::DEBUG));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::TRACE));
    }

    void testLevelTokenError()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS_DEFAULT", "ERROR");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::FATAL));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::ERROR));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::WARNING));
    }

    void testLevelTokenWarning()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS_DEFAULT", "WARNING");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::ERROR));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::WARNING));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::INFO));
    }

    void testLevelTokenInfo()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS_DEFAULT", "INFO");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::WARNING));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::INFO));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::DEBUG));
    }

    void testLevelTokenDebug()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS_DEFAULT", "DEBUG");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::INFO));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::DEBUG));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::TRACE));
    }

    void testLevelTokenTrace()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS_DEFAULT", "TRACE");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::DEBUG));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::TRACE));
    }

    void testLevelTokenAll()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS_DEFAULT", "ALL");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::FATAL));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::ERROR));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::WARNING));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::INFO));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::DEBUG));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::TRACE));
    }

    // NOTE on "+" semantics:
    // The implementation interprets a token like "WARNING+" or "ERROR+" as the named
    // level plus all numerically LOWER-severity bits (i.e. those with less significant
    // bit positions: WARNING -> ERROR -> FATAL).
    void testLevelTokenWithPlus()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS_DEFAULT", "WARNING+");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        bool f = executor->isEnabled(LoggerLevel::FATAL);
        bool e = executor->isEnabled(LoggerLevel::ERROR);
        bool w = executor->isEnabled(LoggerLevel::WARNING);
        bool i = executor->isEnabled(LoggerLevel::INFO);
        bool d = executor->isEnabled(LoggerLevel::DEBUG);
        bool t = executor->isEnabled(LoggerLevel::TRACE);
        std::ostringstream oss;
        oss << "WARNING+ flags F=" << f << " E=" << e << " W=" << w
            << " I=" << i << " D=" << d << " T=" << t;
        CPPUNIT_ASSERT_MESSAGE(oss.str(), f);
        CPPUNIT_ASSERT_MESSAGE(oss.str(), e);
        CPPUNIT_ASSERT_MESSAGE(oss.str(), w);
        CPPUNIT_ASSERT_MESSAGE(oss.str(), !i);
        CPPUNIT_ASSERT_MESSAGE(oss.str(), !d);
        CPPUNIT_ASSERT_MESSAGE(oss.str(), !t);
    }

    void testLevelTokenErrorPlusIncludesLower()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS_DEFAULT", "ERROR+");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        bool f = executor->isEnabled(LoggerLevel::FATAL);
        bool e = executor->isEnabled(LoggerLevel::ERROR);
        bool w = executor->isEnabled(LoggerLevel::WARNING);
        bool i = executor->isEnabled(LoggerLevel::INFO);
        bool d = executor->isEnabled(LoggerLevel::DEBUG);
        bool t = executor->isEnabled(LoggerLevel::TRACE);
        std::ostringstream oss;
        oss << "ERROR+ flags F=" << f << " E=" << e << " W=" << w
            << " I=" << i << " D=" << d << " T=" << t;
        CPPUNIT_ASSERT_MESSAGE(oss.str(), f);
        CPPUNIT_ASSERT_MESSAGE(oss.str(), e);
        CPPUNIT_ASSERT_MESSAGE(oss.str(), !w);
        CPPUNIT_ASSERT_MESSAGE(oss.str(), !i);
        CPPUNIT_ASSERT_MESSAGE(oss.str(), !d);
        CPPUNIT_ASSERT_MESSAGE(oss.str(), !t);
    }

    void testLevelTokenMultipleTokens()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS_DEFAULT", "FATAL ERROR INFO");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::FATAL));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::ERROR));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::WARNING));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::INFO));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::DEBUG));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::TRACE));
    }

    void testLevelTokenInvalidIgnored()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS_DEFAULT", "INVALID_LEVEL ERROR");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        // Should only have ERROR enabled, invalid token ignored
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::FATAL));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::ERROR));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::WARNING));
    }

    void testLevelTokenEmptyString()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS_DEFAULT", "");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        // Empty config should result in no levels enabled
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::FATAL));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::ERROR));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::WARNING));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::INFO));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::DEBUG));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::TRACE));
    }

    void testLevelTokenWhitespaceOnly()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("LEVELS_DEFAULT", "   ");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        // Whitespace-only should result in no levels enabled
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::FATAL));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::ERROR));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::WARNING));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::INFO));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::DEBUG));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::TRACE));
    }

    void testComponentToGroupMappingKnownComponents()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        m_configProvider.setValue("BACKEND", "rdk");
        m_configProvider.setValue("BACKEND_RDK_CONFIG_FILE", "/test/config.ini");
        manager->init(&m_configProvider);

        std::vector<std::pair<std::string, std::string>> components = {
            {"App", "LOG.RDK.SUBS.CORE"}, {"Common", "LOG.RDK.SUBS.CORE"},
            {"Dbus", "LOG.RDK.SUBS.CORE"}, {"DvbSub", "LOG.RDK.SUBS.REND"},
            {"DvbSubDecoder", "LOG.RDK.SUBS.REND"}, {"Gfx", "LOG.RDK.SUBS.GFX"},
            {"GfxEngine", "LOG.RDK.SUBS.GFX"}, {"Protocol", "LOG.RDK.SUBS.CORE"},
            {"SockSrc", "LOG.RDK.SUBS.CORE"}, {"TtmlEngine", "LOG.RDK.SUBS.REND"},
            {"WebvttEngine", "LOG.RDK.SUBS.REND"}, {"TtxDecoder", "LOG.RDK.SUBS.REND"},
            {"Ttxt", "LOG.RDK.SUBS.REND"}, {"Scte", "LOG.RDK.SUBS.REND"},
            {"ClosedCaptions", "LOG.RDK.SUBS.REND"}
        };

        for (const auto& component : components)
        {
            const LoggerExecutor* executor = manager->registerElement(component.first, "mapping");
            CPPUNIT_ASSERT(executor != nullptr);
            CPPUNIT_ASSERT_EQUAL(component.second, std::string(executor->getGroupName()));
        }
    }

    void testIsEnabledDelegatesToBackend()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        manager->init(&m_configProvider);

        CPPUNIT_ASSERT(manager->isEnabled(LoggerLevel::INFO, "CORE"));
        CPPUNIT_ASSERT(manager->isEnabled(LoggerLevel::ERROR, "REND"));
    }

    void testPrintMessageDelegatesToBackend()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        manager->init(&m_configProvider);

        CPPUNIT_ASSERT_NO_THROW(manager->printMessage(LoggerLevel::INFO, "CORE", "App", "test", "Test message"));

        const LoggerExecutor* executor = manager->registerElement("App", "print");
        CPPUNIT_ASSERT_NO_THROW(executor->printMessage(LoggerLevel::INFO, "Test message"));
    }

    void testExecutorReconfigurationAfterInit()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        // Register executor before config
        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        // Should have all levels enabled initially
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::DEBUG));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::TRACE));

        // Configure to only allow ERROR
        m_configProvider.setValue("LEVELS_DEFAULT", "ERROR");
        manager->init(&m_configProvider);

        // Executor should be reconfigured
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::ERROR));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::DEBUG));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::TRACE));
    }

    void testReinitChangesLevels()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        // First init with ERROR level
        m_configProvider.setValue("LEVELS_DEFAULT", "ERROR");
        manager->init(&m_configProvider);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::ERROR));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::DEBUG));

        // Change config and reinit
        m_configProvider.setValue("LEVELS_DEFAULT", "DEBUG");
        manager->init(&m_configProvider);

        // Should now have DEBUG enabled
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::DEBUG));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::ERROR));
    }

    void testConcurrentRegisterElement()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        std::vector<const LoggerExecutor*> executors(10);
        std::vector<std::thread> threads;

        // Launch multiple threads registering elements
        for (int i = 0; i < 10; ++i)
        {
            threads.emplace_back([&, i]() {
                executors[i] = manager->registerElement("App", "test" + std::to_string(i));
            });
        }

        // Wait for all threads
        for (auto& thread : threads)
        {
            thread.join();
        }

        // Verify all executors were created
        for (int i = 0; i < 10; ++i)
        {
            CPPUNIT_ASSERT(executors[i] != nullptr);
        }
    }

    void testConcurrentDuplicateRegister()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();
        std::vector<const LoggerExecutor*> executors(10);
        std::vector<std::thread> threads;

        for (std::size_t index = 0; index < executors.size(); ++index)
        {
            threads.emplace_back([&, index, manager]() {
                executors[index] = manager->registerElement("App", "duplicate");
            });
        }

        for (auto& thread : threads)
        {
            thread.join();
        }

        CPPUNIT_ASSERT(executors.front() != nullptr);
        for (const auto* executor : executors)
        {
            CPPUNIT_ASSERT_EQUAL(executors.front(), executor);
        }
    }

    void testInitDuringRegisterElement()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        // Use atomic for defined cross-thread visibility instead of volatile.
        std::atomic<bool> initStarted{false};
        std::atomic<bool> initComplete{false};
        const LoggerExecutor* executor = nullptr;

        // Thread 1: init
        std::thread initThread([&]() {
            m_configProvider.setValue("LEVELS_DEFAULT", "ERROR");
            initStarted.store(true, std::memory_order_release);
            manager->init(&m_configProvider);
            initComplete.store(true, std::memory_order_release);
        });

        // Thread 2: register element
        std::thread registerThread([&]() {
            while (!initStarted.load(std::memory_order_acquire))
            {
                std::this_thread::yield();
            }
            executor = manager->registerElement("App", "test");
        });

        initThread.join();
        registerThread.join();

        CPPUNIT_ASSERT(initComplete.load(std::memory_order_acquire));
        CPPUNIT_ASSERT(executor != nullptr);
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::ERROR));
        CPPUNIT_ASSERT(!executor->isEnabled(LoggerLevel::DEBUG));
    }

    void testExecutorsRemainValid()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        // Store component name to verify executor remains valid
        std::string component = executor->getComponent();
        CPPUNIT_ASSERT_EQUAL(std::string("App"), component);

        // Register more executors
        for (int i = 0; i < 100; ++i)
        {
            manager->registerElement("Common", "test" + std::to_string(i));
        }

        // Original executor should still be valid
        CPPUNIT_ASSERT_EQUAL(std::string("App"), executor->getComponent());
    }

    void testUnregisterElementIsNoop()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        // unregisterElement should not crash
        CPPUNIT_ASSERT_NO_THROW(manager->unregisterElement(executor));

        // Executor should still be valid (since unregister is noop)
        CPPUNIT_ASSERT_EQUAL(std::string("App"), executor->getComponent());
    }

    void testConfigurationChangePropagatesToExecutors()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        // Register multiple executors
        const LoggerExecutor* executor1 = manager->registerElement("App", "test1");
        const LoggerExecutor* executor2 = manager->registerElement("Common", "test2");

        // Set specific config
        m_configProvider.setValue("LEVELS_DEFAULT", "WARNING");
        manager->init(&m_configProvider);

        // Both should have WARNING enabled
        CPPUNIT_ASSERT(executor1->isEnabled(LoggerLevel::WARNING));
        CPPUNIT_ASSERT(executor2->isEnabled(LoggerLevel::WARNING));
        CPPUNIT_ASSERT(!executor1->isEnabled(LoggerLevel::DEBUG));
        CPPUNIT_ASSERT(!executor2->isEnabled(LoggerLevel::DEBUG));

        // Change config
        m_configProvider.setValue("LEVELS_DEFAULT", "DEBUG");
        manager->init(&m_configProvider);

        // Both should now have DEBUG enabled
        CPPUNIT_ASSERT(executor1->isEnabled(LoggerLevel::DEBUG));
        CPPUNIT_ASSERT(executor2->isEnabled(LoggerLevel::DEBUG));
    }

    void testLevelConfigurationWithNoConfigProvider()
    {
        LoggerManagerImpl* manager = LoggerManagerImpl::getInstance();

        manager->init(nullptr);

        const LoggerExecutor* executor = manager->registerElement("App", "test");
        CPPUNIT_ASSERT(executor != nullptr);

        // Without config provider, should have all levels enabled
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::FATAL));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::ERROR));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::WARNING));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::INFO));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::DEBUG));
        CPPUNIT_ASSERT(executor->isEnabled(LoggerLevel::TRACE));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(LoggerManagerImplTest);
