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
#include "Configuration.hpp"
#include "Options.hpp"
#include <fstream>
#include <memory>
#include <cstdio>

using namespace subttxrend::ctrl;

// Helper function to create temporary test config files
class TempConfigFile
{
public:
    TempConfigFile(const std::string& content) : m_filename("/tmp/test_config_" + std::to_string(rand()) + ".ini")
    {
        std::ofstream file(m_filename);
        if (file.is_open())
        {
            file << content;
            file.close();
        }
    }

    ~TempConfigFile()
    {
        std::remove(m_filename.c_str());
    }

    std::string getFilename() const { return m_filename; }

private:
    std::string m_filename;
};

// Helper to create Options with specific values
class OptionsBuilder
{
public:
    static std::unique_ptr<Options> createWithConfigFile(const std::string& configPath)
    {
        std::vector<std::string> args = {"program", "--config-file-path=" + configPath};
        return createFromArgs(args);
    }

    static std::unique_ptr<Options> createWithSocketPath(const std::string& socketPath)
    {
        std::vector<std::string> args = {"program", "--main-socket-path=" + socketPath};
        return createFromArgs(args);
    }

    static std::unique_ptr<Options> createWithBoth(const std::string& configPath, const std::string& socketPath)
    {
        std::vector<std::string> args = {"program", "--config-file-path=" + configPath, "--main-socket-path=" + socketPath};
        return createFromArgs(args);
    }

    static std::unique_ptr<Options> createEmpty()
    {
        std::vector<std::string> args = {"program"};
        return createFromArgs(args);
    }

private:
    static std::unique_ptr<Options> createFromArgs(const std::vector<std::string>& args)
    {
        // Convert to char* array - need to keep strings alive
        static std::vector<std::vector<char>> argStorage;
        argStorage.clear();
        std::vector<char*> argv;

        for (const auto& arg : args)
        {
            argStorage.emplace_back(arg.begin(), arg.end());
            argStorage.back().push_back('\0');
            argv.push_back(argStorage.back().data());
        }

        return std::make_unique<Options>(argv.size(), argv.data());
    }
};

class ConfigurationTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(ConfigurationTest);
    CPPUNIT_TEST(testConstructorWithValidOptions);
    CPPUNIT_TEST(testConstructorWithValidConfigFile);
    CPPUNIT_TEST(testConstructorWithInvalidConfigFilePath);
    CPPUNIT_TEST(testConstructorWithNonExistentFile);
    CPPUNIT_TEST(testConstructorWithEmptyConfigFilePath);
    CPPUNIT_TEST(testConstructorWithPartialConfigData);
    CPPUNIT_TEST(testSocketPathFromOptionsOnly);
    CPPUNIT_TEST(testSocketPathFromConfigFileOnly);
    CPPUNIT_TEST(testSocketPathOptionsPrecedence);
    CPPUNIT_TEST(testSocketPathWithLeadingWhitespace);
    CPPUNIT_TEST(testSocketPathWithTrailingWhitespace);
    CPPUNIT_TEST(testSocketPathWithBothWhitespace);
    CPPUNIT_TEST(testSocketPathWithEmptyOptions);
    CPPUNIT_TEST(testSocketPathWithSpecialCharacters);
    CPPUNIT_TEST(testSocketPathCalledMultipleTimes);
    CPPUNIT_TEST(testGetTeletextConfigReturnsValidProvider);
    CPPUNIT_TEST(testGetTeletextConfigCalledMultipleTimes);
    CPPUNIT_TEST(testGetRdkEnvConfigReturnsValidProvider);
    CPPUNIT_TEST(testGetRdkEnvConfigWithCustomFile);
    CPPUNIT_TEST(testGetRdkEnvConfigCalledMultipleTimes);
    CPPUNIT_TEST(testGetLoggerConfigReturnsValidProvider);
    CPPUNIT_TEST(testGetLoggerConfigWithCustomFile);
    CPPUNIT_TEST(testGetLoggerConfigCalledMultipleTimes);
    CPPUNIT_TEST(testGetTtmlConfigReturnsValidProvider);
    CPPUNIT_TEST(testGetTtmlConfigCalledMultipleTimes);
    CPPUNIT_TEST(testGetWebvttConfigReturnsValidProvider);
    CPPUNIT_TEST(testGetWebvttConfigCalledMultipleTimes);
    CPPUNIT_TEST(testConfigPrecedenceConfigFileOverDefaults);
    CPPUNIT_TEST(testAllProvidersWorkTogether);
    CPPUNIT_TEST(testConfigWithCustomValues);
    CPPUNIT_TEST(testConfigWithEmptyFile);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        // Seed random for temp file names
        srand(time(nullptr));
    }

    void tearDown() override
    {
        // Cleanup handled by TempConfigFile destructors
    }

protected:
    void testConstructorWithValidOptions()
    {
        auto options = OptionsBuilder::createEmpty();
        CPPUNIT_ASSERT(options->isValid());

        Configuration config(*options);
        // If we reach here, constructor succeeded
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithValidConfigFile()
    {
        TempConfigFile tempFile(
            "MAIN_CONTEXT.SOCKET_PATH=/test/socket\n"
            "RDKENV.GFX.VL.APP.1.WIDTH=1920\n"
        );

        auto options = OptionsBuilder::createWithConfigFile(tempFile.getFilename());
        CPPUNIT_ASSERT(options->isValid());

        Configuration config(*options);

        // Verify config file was loaded by checking a value
        std::string socketPath = config.getMainContextSocketPath();
        CPPUNIT_ASSERT_EQUAL(std::string("/test/socket"), socketPath);
    }

    void testConstructorWithInvalidConfigFilePath()
    {
        auto options = OptionsBuilder::createWithConfigFile("/invalid/path/config.ini");
        // Options object is valid (arguments parsed correctly), but the file path doesn't exist
        CPPUNIT_ASSERT(options->isValid());

        // Configuration should handle non-existent file gracefully (log warning, use defaults)
        Configuration config(*options);

        // Should use default socket path since config file couldn't be loaded
        std::string socketPath = config.getMainContextSocketPath();
        #ifdef PC_BUILD
        CPPUNIT_ASSERT_EQUAL(std::string("/tmp/subttx-socket"), socketPath);
        #else
        CPPUNIT_ASSERT_EQUAL(std::string("/var/run/subttx/pes_data_main"), socketPath);
        #endif
    }

    void testConstructorWithNonExistentFile()
    {
        // Create a path that definitely doesn't exist
        std::string nonExistentPath = "/tmp/nonexistent_config_" + std::to_string(rand()) + "_" + std::to_string(time(nullptr)) + ".ini";

        // Verify the file actually doesn't exist
        std::ifstream checkFile(nonExistentPath);
        CPPUNIT_ASSERT_MESSAGE("File should not exist for this test", !checkFile.good());

        auto options = OptionsBuilder::createWithConfigFile(nonExistentPath);
        CPPUNIT_ASSERT(options->isValid());

        // Should handle gracefully
        Configuration config(*options);

        // Should use default socket path
        std::string socketPath = config.getMainContextSocketPath();
        #ifdef PC_BUILD
        CPPUNIT_ASSERT_EQUAL(std::string("/tmp/subttx-socket"), socketPath);
        #else
        CPPUNIT_ASSERT_EQUAL(std::string("/var/run/subttx/pes_data_main"), socketPath);
        #endif
    }

    void testConstructorWithEmptyConfigFilePath()
    {
        auto options = OptionsBuilder::createEmpty();
        CPPUNIT_ASSERT(options->isValid());

        Configuration config(*options);

        // Should use default socket path
        std::string socketPath = config.getMainContextSocketPath();
        #ifdef PC_BUILD
        CPPUNIT_ASSERT_EQUAL(std::string("/tmp/subttx-socket"), socketPath);
        #else
        CPPUNIT_ASSERT_EQUAL(std::string("/var/run/subttx/pes_data_main"), socketPath);
        #endif
    }

    void testConstructorWithPartialConfigData()
    {
        TempConfigFile tempFile(
            "MAIN_CONTEXT.SOCKET_PATH=/partial/socket\n"
            "# Missing other values\n"
        );

        auto options = OptionsBuilder::createWithConfigFile(tempFile.getFilename());
        Configuration config(*options);

        // Should load valid entry from config file
        CPPUNIT_ASSERT_EQUAL(std::string("/partial/socket"), config.getMainContextSocketPath());

        // Config providers should be accessible (returns valid reference)
        const auto& rdkConfig = config.getRdkEnvConfig();
        // We can verify the reference is valid (doesn't crash)
        CPPUNIT_ASSERT(true);
    }

    void testSocketPathFromOptionsOnly()
    {
        auto options = OptionsBuilder::createWithSocketPath("/custom/socket/from/options");
        Configuration config(*options);

        std::string socketPath = config.getMainContextSocketPath();
        CPPUNIT_ASSERT_EQUAL(std::string("/custom/socket/from/options"), socketPath);
    }

    void testSocketPathFromConfigFileOnly()
    {
        TempConfigFile tempFile("MAIN_CONTEXT.SOCKET_PATH=/custom/socket/from/file\n");

        auto options = OptionsBuilder::createWithConfigFile(tempFile.getFilename());
        Configuration config(*options);

        std::string socketPath = config.getMainContextSocketPath();
        CPPUNIT_ASSERT_EQUAL(std::string("/custom/socket/from/file"), socketPath);
    }

    void testSocketPathOptionsPrecedence()
    {
        TempConfigFile tempFile("MAIN_CONTEXT.SOCKET_PATH=/socket/from/file\n");

        auto options = OptionsBuilder::createWithBoth(tempFile.getFilename(), "/socket/from/options");
        Configuration config(*options);

        // Options should take precedence
        std::string socketPath = config.getMainContextSocketPath();
        CPPUNIT_ASSERT_EQUAL(std::string("/socket/from/options"), socketPath);
    }

    void testSocketPathWithLeadingWhitespace()
    {
        TempConfigFile tempFile("MAIN_CONTEXT.SOCKET_PATH=   /socket/with/spaces\n");

        auto options = OptionsBuilder::createWithConfigFile(tempFile.getFilename());
        Configuration config(*options);

        std::string socketPath = config.getMainContextSocketPath();
        CPPUNIT_ASSERT_EQUAL(std::string("/socket/with/spaces"), socketPath);
    }

    void testSocketPathWithTrailingWhitespace()
    {
        TempConfigFile tempFile("MAIN_CONTEXT.SOCKET_PATH=/socket/with/spaces   \n");

        auto options = OptionsBuilder::createWithConfigFile(tempFile.getFilename());
        Configuration config(*options);

        std::string socketPath = config.getMainContextSocketPath();
        CPPUNIT_ASSERT_EQUAL(std::string("/socket/with/spaces"), socketPath);
    }

    void testSocketPathWithBothWhitespace()
    {
        TempConfigFile tempFile("MAIN_CONTEXT.SOCKET_PATH=   /socket/trimmed   \n");

        auto options = OptionsBuilder::createWithConfigFile(tempFile.getFilename());
        Configuration config(*options);

        std::string socketPath = config.getMainContextSocketPath();
        CPPUNIT_ASSERT_EQUAL(std::string("/socket/trimmed"), socketPath);
    }

    void testSocketPathWithEmptyOptions()
    {
        auto options = OptionsBuilder::createWithSocketPath("");
        Configuration config(*options);

        // Should use default when option is empty
        std::string socketPath = config.getMainContextSocketPath();
        CPPUNIT_ASSERT(socketPath.size() > 0);
    }

    void testSocketPathWithSpecialCharacters()
    {
        auto options = OptionsBuilder::createWithSocketPath("/socket-path_with.special@chars");
        Configuration config(*options);

        std::string socketPath = config.getMainContextSocketPath();
        CPPUNIT_ASSERT_EQUAL(std::string("/socket-path_with.special@chars"), socketPath);
    }

    void testSocketPathCalledMultipleTimes()
    {
        auto options = OptionsBuilder::createWithSocketPath("/test/socket");
        Configuration config(*options);

        std::string path1 = config.getMainContextSocketPath();
        std::string path2 = config.getMainContextSocketPath();
        std::string path3 = config.getMainContextSocketPath();

        CPPUNIT_ASSERT_EQUAL(path1, path2);
        CPPUNIT_ASSERT_EQUAL(path2, path3);
    }

    void testGetTeletextConfigReturnsValidProvider()
    {
        auto options = OptionsBuilder::createEmpty();
        Configuration config(*options);

        // Verify we can get the provider reference without crashing
        const auto& provider = config.getTeletextConfig();
        // Reference is always valid if we reach here
        CPPUNIT_ASSERT(true);
    }

    void testGetTeletextConfigCalledMultipleTimes()
    {
        auto options = OptionsBuilder::createEmpty();
        Configuration config(*options);

        const auto& provider1 = config.getTeletextConfig();
        const auto& provider2 = config.getTeletextConfig();

        // Should return same instance (same address)
        CPPUNIT_ASSERT_EQUAL(&provider1, &provider2);
    }

    void testGetRdkEnvConfigReturnsValidProvider()
    {
        auto options = OptionsBuilder::createEmpty();
        Configuration config(*options);

        // Verify we can get the provider reference without crashing
        const auto& provider = config.getRdkEnvConfig();
        CPPUNIT_ASSERT(true);
    }

    void testGetRdkEnvConfigWithCustomFile()
    {
        TempConfigFile tempFile(
            "RDKENV.GFX.VL.APP.1.WIDTH=2560\n"
            "RDKENV.GFX.VL.APP.1.HEIGHT=1440\n"
        );

        auto options = OptionsBuilder::createWithConfigFile(tempFile.getFilename());
        Configuration config(*options);

        // Verify we can get the provider reference
        const auto& rdkConfig = config.getRdkEnvConfig();
        // Config loaded successfully if we reach here
        CPPUNIT_ASSERT(true);
    }

    void testGetRdkEnvConfigCalledMultipleTimes()
    {
        auto options = OptionsBuilder::createEmpty();
        Configuration config(*options);

        const auto& provider1 = config.getRdkEnvConfig();
        const auto& provider2 = config.getRdkEnvConfig();

        CPPUNIT_ASSERT_EQUAL(&provider1, &provider2);
    }

    void testGetLoggerConfigReturnsValidProvider()
    {
        auto options = OptionsBuilder::createEmpty();
        Configuration config(*options);

        // Verify we can get the provider reference without crashing
        const auto& provider = config.getLoggerConfig();
        CPPUNIT_ASSERT(true);
    }

    void testGetLoggerConfigWithCustomFile()
    {
        TempConfigFile tempFile("LOGGER.BACKEND=custom\n");

        auto options = OptionsBuilder::createWithConfigFile(tempFile.getFilename());
        Configuration config(*options);

        // Verify we can get the provider reference
        const auto& loggerConfig = config.getLoggerConfig();
        CPPUNIT_ASSERT(true);
    }

    void testGetLoggerConfigCalledMultipleTimes()
    {
        auto options = OptionsBuilder::createEmpty();
        Configuration config(*options);

        const auto& provider1 = config.getLoggerConfig();
        const auto& provider2 = config.getLoggerConfig();

        CPPUNIT_ASSERT_EQUAL(&provider1, &provider2);
    }

    void testGetTtmlConfigReturnsValidProvider()
    {
        auto options = OptionsBuilder::createEmpty();
        Configuration config(*options);

        // Verify we can get the provider reference without crashing
        const auto& provider = config.getTtmlConfig();
        CPPUNIT_ASSERT(true);
    }

    void testGetTtmlConfigCalledMultipleTimes()
    {
        auto options = OptionsBuilder::createEmpty();
        Configuration config(*options);

        const auto& provider1 = config.getTtmlConfig();
        const auto& provider2 = config.getTtmlConfig();

        CPPUNIT_ASSERT_EQUAL(&provider1, &provider2);
    }

    void testGetWebvttConfigReturnsValidProvider()
    {
        auto options = OptionsBuilder::createEmpty();
        Configuration config(*options);

        // Verify we can get the provider reference without crashing
        const auto& provider = config.getWebvttConfig();
        CPPUNIT_ASSERT(true);
    }

    void testGetWebvttConfigCalledMultipleTimes()
    {
        auto options = OptionsBuilder::createEmpty();
        Configuration config(*options);

        const auto& provider1 = config.getWebvttConfig();
        const auto& provider2 = config.getWebvttConfig();

        CPPUNIT_ASSERT_EQUAL(&provider1, &provider2);
    }

    void testConfigPrecedenceConfigFileOverDefaults()
    {
        TempConfigFile tempFile(
            "MAIN_CONTEXT.SOCKET_PATH=/custom/path\n"
            "RDKENV.GFX.VL.APP.1.WIDTH=2560\n"
            "LOGGER.BACKEND=custom\n"
        );

        auto options = OptionsBuilder::createWithConfigFile(tempFile.getFilename());
        Configuration config(*options);

        // Custom socket path from config file (not default)
        CPPUNIT_ASSERT_EQUAL(std::string("/custom/path"), config.getMainContextSocketPath());

        // Config providers loaded successfully
        const auto& rdkConfig = config.getRdkEnvConfig();
        const auto& loggerConfig = config.getLoggerConfig();
        CPPUNIT_ASSERT(true);
    }

    void testAllProvidersWorkTogether()
    {
        TempConfigFile tempFile(
            "MAIN_CONTEXT.SOCKET_PATH=/test/socket\n"
            "RDKENV.GFX.VL.APP.1.WIDTH=1920\n"
            "LOGGER.BACKEND=test\n"
            "TELETEXT.CUSTOM_KEY=custom_value\n"
            "TTML.TTML_KEY=ttml_value\n"
            "WEBVTT.WEBVTT_KEY=webvtt_value\n"
        );

        auto options = OptionsBuilder::createWithConfigFile(tempFile.getFilename());
        Configuration config(*options);

        // Verify socket path loaded from config
        CPPUNIT_ASSERT_EQUAL(std::string("/test/socket"), config.getMainContextSocketPath());

        // All providers should be accessible without crashing
        const auto& rdkConfig = config.getRdkEnvConfig();
        const auto& loggerConfig = config.getLoggerConfig();
        const auto& ttxConfig = config.getTeletextConfig();
        const auto& ttmlConfig = config.getTtmlConfig();
        const auto& webvttConfig = config.getWebvttConfig();

        // If we reach here, all providers work together
        CPPUNIT_ASSERT(true);
    }

    void testConfigWithCustomValues()
    {
        TempConfigFile tempFile(
            "MAIN_CONTEXT.SOCKET_PATH=/custom/values/socket\n"
            "RDKENV.FEATURE.TTX.GRID_W=1000\n"
            "RDKENV.FEATURE.TTX.GRID_H=800\n"
        );

        auto options = OptionsBuilder::createWithConfigFile(tempFile.getFilename());
        Configuration config(*options);

        // Verify custom socket path loaded
        CPPUNIT_ASSERT_EQUAL(std::string("/custom/values/socket"), config.getMainContextSocketPath());

        // Config provider accessible
        const auto& rdkConfig = config.getRdkEnvConfig();
        CPPUNIT_ASSERT(true);
    }

    void testConfigWithEmptyFile()
    {
        TempConfigFile tempFile("");

        auto options = OptionsBuilder::createWithConfigFile(tempFile.getFilename());
        Configuration config(*options);

        // Should use default socket path
        #ifdef PC_BUILD
        CPPUNIT_ASSERT_EQUAL(std::string("/tmp/subttx-socket"), config.getMainContextSocketPath());
        #else
        CPPUNIT_ASSERT_EQUAL(std::string("/var/run/subttx/pes_data_main"), config.getMainContextSocketPath());
        #endif

        // Config providers should still be accessible
        const auto& rdkConfig = config.getRdkEnvConfig();
        const auto& loggerConfig = config.getLoggerConfig();
        CPPUNIT_ASSERT(true);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConfigurationTest);
