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
#include "Options.hpp"
#include <memory>
#include <vector>
#include <string>
#include <sstream>

using namespace subttxrend::ctrl;

// Helper class to build Options objects from argument vectors
class OptionsBuilder
{
public:
    static std::unique_ptr<Options> createFromArgs(const std::vector<std::string>& args)
    {
        // Convert vector<string> to char* array
        // Note: We need to keep storage alive for the duration of Options construction
        std::vector<std::vector<char>> argStorage;
        std::vector<char*> argv;

        for (const auto& arg : args)
        {
            argStorage.emplace_back(arg.begin(), arg.end());
            argStorage.back().push_back('\0');
            argv.push_back(argStorage.back().data());
        }

        return std::make_unique<Options>(static_cast<int>(argv.size()), argv.data());
    }

    static std::unique_ptr<Options> createEmpty()
    {
        return createFromArgs({"subttxrend-app"});
    }

    static std::unique_ptr<Options> createWithHelp()
    {
        return createFromArgs({"subttxrend-app", "--help"});
    }

    static std::unique_ptr<Options> createWithHelpShort()
    {
        return createFromArgs({"subttxrend-app", "-h"});
    }

    static std::unique_ptr<Options> createWithMainSocketPath(const std::string& path)
    {
        return createFromArgs({"subttxrend-app", "--main-socket-path=" + path});
    }

    static std::unique_ptr<Options> createWithMainSocketPathShort(const std::string& path)
    {
        return createFromArgs({"subttxrend-app", "-msp=" + path});
    }

    static std::unique_ptr<Options> createWithConfigFilePath(const std::string& path)
    {
        return createFromArgs({"subttxrend-app", "--config-file-path=" + path});
    }

    static std::unique_ptr<Options> createWithConfigFilePathShort(const std::string& path)
    {
        return createFromArgs({"subttxrend-app", "-cfp=" + path});
    }

    static std::unique_ptr<Options> createWithBoth(const std::string& socketPath, const std::string& configPath)
    {
        return createFromArgs({"subttxrend-app", "--main-socket-path=" + socketPath, "--config-file-path=" + configPath});
    }
};

class OptionsTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(OptionsTest);

    CPPUNIT_TEST(testConstructorWithEmptyArguments);
    CPPUNIT_TEST(testConstructorValidatesOnConstruction);
    CPPUNIT_TEST(testIsValidReturnsTrueForValidInput);
    CPPUNIT_TEST(testIsValidReturnsFalseForInvalidInput);
    CPPUNIT_TEST(testParseHelpOptionLongName);
    CPPUNIT_TEST(testParseHelpOptionShortName);
    CPPUNIT_TEST(testHelpWithOtherOptionsFails);
    CPPUNIT_TEST(testHelpWithValueFails);
    CPPUNIT_TEST(testHasSeparateReturnsTrueForHelp);
    CPPUNIT_TEST(testHasSeparateReturnsFalseWhenNoSeparate);
    CPPUNIT_TEST(testGetSeparateReturnsHelpKey);
    CPPUNIT_TEST(testParseMainSocketPathLongName);
    CPPUNIT_TEST(testParseMainSocketPathShortName);
    CPPUNIT_TEST(testMainSocketPathWithoutValueFails);
    CPPUNIT_TEST(testMainSocketPathWithEmptyString);
    CPPUNIT_TEST(testMainSocketPathWithSpaces);
    CPPUNIT_TEST(testMainSocketPathWithSpecialCharacters);
    CPPUNIT_TEST(testMainSocketPathWithMultipleEquals);
    CPPUNIT_TEST(testGetMainSocketPathWhenNotSpecified);
    CPPUNIT_TEST(testParseConfigFilePathLongName);
    CPPUNIT_TEST(testParseConfigFilePathShortName);
    CPPUNIT_TEST(testConfigFilePathNotSpecifiedReturnsDefault);
    CPPUNIT_TEST(testConfigFilePathWithRelativePath);
    CPPUNIT_TEST(testConfigFilePathWithAbsolutePath);
    CPPUNIT_TEST(testParseBothMainSocketAndConfigFile);
    CPPUNIT_TEST(testParseSameOptionTwiceFails);
    CPPUNIT_TEST(testParseAllNonSeparateOptions);
    CPPUNIT_TEST(testUnknownOptionFails);
    CPPUNIT_TEST(testOptionWithOnlyEqualSign);
    CPPUNIT_TEST(testGetOptionValueForUnsetOptionalWithDefault);
    CPPUNIT_TEST(testGetOptionValueForUnsetOptionalWithoutDefault);
    CPPUNIT_TEST(testGetOptionValueForSetOption);
    CPPUNIT_TEST(testGetOptionValueForHelpOption);
    CPPUNIT_TEST(testPrintUsageDoesNotCrash);
    CPPUNIT_TEST(testVeryLongOptionValue);
    CPPUNIT_TEST(testOptionValueWithTrailingEquals);
    CPPUNIT_TEST(testMultipleOptionsInDifferentOrder);
    CPPUNIT_TEST(testRealWorldHelpScenario);
    CPPUNIT_TEST(testRealWorldFullConfigScenario);
    CPPUNIT_TEST(testRealWorldNoArgumentsScenario);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        // Setup code here
    }

    void tearDown() override
    {
        // Cleanup code here
    }

protected:
    void testConstructorWithEmptyArguments()
    {
        auto options = OptionsBuilder::createEmpty();
        CPPUNIT_ASSERT(options != nullptr);
        CPPUNIT_ASSERT(options->isValid());
    }

    void testConstructorValidatesOnConstruction()
    {
        // Constructor should parse and validate during construction
        auto options = OptionsBuilder::createEmpty();
        CPPUNIT_ASSERT(options->isValid());
    }

    void testIsValidReturnsTrueForValidInput()
    {
        auto options = OptionsBuilder::createWithMainSocketPath("/tmp/socket");
        CPPUNIT_ASSERT(options->isValid());
    }

    void testIsValidReturnsFalseForInvalidInput()
    {
        auto options = OptionsBuilder::createFromArgs({"subttxrend-app", "--invalid-option"});
        CPPUNIT_ASSERT(!options->isValid());
    }

    void testParseHelpOptionLongName()
    {
        auto options = OptionsBuilder::createWithHelp();
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT(options->hasSeparate());
        CPPUNIT_ASSERT_EQUAL(Options::Key::HELP, options->getSeparate());
    }

    void testParseHelpOptionShortName()
    {
        auto options = OptionsBuilder::createWithHelpShort();
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT(options->hasSeparate());
        CPPUNIT_ASSERT_EQUAL(Options::Key::HELP, options->getSeparate());
    }

    void testHelpWithOtherOptionsFails()
    {
        auto options = OptionsBuilder::createFromArgs({"subttxrend-app", "--help", "--main-socket-path=/tmp/socket"});
        CPPUNIT_ASSERT(!options->isValid());
    }

    void testHelpWithValueFails()
    {
        auto options = OptionsBuilder::createFromArgs({"subttxrend-app", "--help=value"});
        CPPUNIT_ASSERT(!options->isValid());
    }

    void testHasSeparateReturnsTrueForHelp()
    {
        auto options = OptionsBuilder::createWithHelp();
        CPPUNIT_ASSERT(options->hasSeparate());
    }

    void testHasSeparateReturnsFalseWhenNoSeparate()
    {
        auto options = OptionsBuilder::createEmpty();
        CPPUNIT_ASSERT(!options->hasSeparate());
    }

    void testGetSeparateReturnsHelpKey()
    {
        auto options = OptionsBuilder::createWithHelp();
        CPPUNIT_ASSERT(options->hasSeparate());
        CPPUNIT_ASSERT_EQUAL(Options::Key::HELP, options->getSeparate());
    }

    void testParseMainSocketPathLongName()
    {
        std::string testPath = "/tmp/test_socket";
        auto options = OptionsBuilder::createWithMainSocketPath(testPath);
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT_EQUAL(testPath, options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
    }

    void testParseMainSocketPathShortName()
    {
        std::string testPath = "/tmp/test_socket";
        auto options = OptionsBuilder::createWithMainSocketPathShort(testPath);
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT_EQUAL(testPath, options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
    }

    void testMainSocketPathWithoutValueFails()
    {
        auto options = OptionsBuilder::createFromArgs({"subttxrend-app", "--main-socket-path"});
        CPPUNIT_ASSERT(!options->isValid());
    }

    void testMainSocketPathWithEmptyString()
    {
        auto options = OptionsBuilder::createWithMainSocketPath("");
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT_EQUAL(std::string(""), options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
    }

    void testMainSocketPathWithSpaces()
    {
        std::string pathWithSpaces = "/tmp/path with spaces/socket";
        auto options = OptionsBuilder::createWithMainSocketPath(pathWithSpaces);
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT_EQUAL(pathWithSpaces, options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
    }

    void testMainSocketPathWithSpecialCharacters()
    {
        std::string pathWithSpecial = "/tmp/socket-123_test.sock";
        auto options = OptionsBuilder::createWithMainSocketPath(pathWithSpecial);
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT_EQUAL(pathWithSpecial, options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
    }

    void testMainSocketPathWithMultipleEquals()
    {
        // Value after first '=' should be parsed correctly
        auto options = OptionsBuilder::createFromArgs({"subttxrend-app", "--main-socket-path=value=with=equals"});
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT_EQUAL(std::string("value=with=equals"), options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
    }

    void testGetMainSocketPathWhenNotSpecified()
    {
        auto options = OptionsBuilder::createEmpty();
        CPPUNIT_ASSERT(options->isValid());
        // Should return empty string as default for MAIN_SOCKET_PATH
        CPPUNIT_ASSERT_EQUAL(std::string(""), options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
    }

    void testParseConfigFilePathLongName()
    {
        std::string testPath = "/etc/custom/config.ini";
        auto options = OptionsBuilder::createWithConfigFilePath(testPath);
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT_EQUAL(testPath, options->getOptionValue(Options::Key::CONFIG_FILE_PATH));
    }

    void testParseConfigFilePathShortName()
    {
        std::string testPath = "/etc/custom/config.ini";
        auto options = OptionsBuilder::createWithConfigFilePathShort(testPath);
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT_EQUAL(testPath, options->getOptionValue(Options::Key::CONFIG_FILE_PATH));
    }

    void testConfigFilePathNotSpecifiedReturnsDefault()
    {
        auto options = OptionsBuilder::createEmpty();
        CPPUNIT_ASSERT(options->isValid());
        // Should return default value
        CPPUNIT_ASSERT_EQUAL(std::string("/etc/subttxrend/config.ini"),
                           options->getOptionValue(Options::Key::CONFIG_FILE_PATH));
    }

    void testConfigFilePathWithRelativePath()
    {
        std::string relativePath = "../config/test.ini";
        auto options = OptionsBuilder::createWithConfigFilePath(relativePath);
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT_EQUAL(relativePath, options->getOptionValue(Options::Key::CONFIG_FILE_PATH));
    }

    void testConfigFilePathWithAbsolutePath()
    {
        std::string absolutePath = "/usr/local/etc/config.ini";
        auto options = OptionsBuilder::createWithConfigFilePath(absolutePath);
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT_EQUAL(absolutePath, options->getOptionValue(Options::Key::CONFIG_FILE_PATH));
    }

    void testParseBothMainSocketAndConfigFile()
    {
        std::string socketPath = "/tmp/socket";
        std::string configPath = "/etc/config.ini";
        auto options = OptionsBuilder::createWithBoth(socketPath, configPath);

        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT_EQUAL(socketPath, options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
        CPPUNIT_ASSERT_EQUAL(configPath, options->getOptionValue(Options::Key::CONFIG_FILE_PATH));
    }

    void testParseSameOptionTwiceFails()
    {
        auto options = OptionsBuilder::createFromArgs({
            "subttxrend-app",
            "--main-socket-path=/tmp/socket1",
            "--main-socket-path=/tmp/socket2"
        });
        CPPUNIT_ASSERT(!options->isValid());
    }

    void testParseAllNonSeparateOptions()
    {
        std::string socketPath = "/tmp/socket";
        std::string configPath = "/etc/config.ini";
        auto options = OptionsBuilder::createFromArgs({
            "subttxrend-app",
            "--main-socket-path=" + socketPath,
            "--config-file-path=" + configPath
        });

        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT(!options->hasSeparate());
        CPPUNIT_ASSERT_EQUAL(socketPath, options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
        CPPUNIT_ASSERT_EQUAL(configPath, options->getOptionValue(Options::Key::CONFIG_FILE_PATH));
    }

    void testUnknownOptionFails()
    {
        auto options = OptionsBuilder::createFromArgs({"subttxrend-app", "--unknown-option=value"});
        CPPUNIT_ASSERT(!options->isValid());
    }

    void testOptionWithOnlyEqualSign()
    {
        // --main-socket-path= should accept empty value
        auto options = OptionsBuilder::createFromArgs({"subttxrend-app", "--main-socket-path="});
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT_EQUAL(std::string(""), options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
    }

    void testGetOptionValueForUnsetOptionalWithDefault()
    {
        auto options = OptionsBuilder::createEmpty();
        CPPUNIT_ASSERT(options->isValid());
        // CONFIG_FILE_PATH has default value
        CPPUNIT_ASSERT_EQUAL(std::string("/etc/subttxrend/config.ini"),
                           options->getOptionValue(Options::Key::CONFIG_FILE_PATH));
    }

    void testGetOptionValueForUnsetOptionalWithoutDefault()
    {
        auto options = OptionsBuilder::createEmpty();
        CPPUNIT_ASSERT(options->isValid());
        // MAIN_SOCKET_PATH has empty string as default
        CPPUNIT_ASSERT_EQUAL(std::string(""),
                           options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
    }

    void testGetOptionValueForSetOption()
    {
        std::string testPath = "/custom/path";
        auto options = OptionsBuilder::createWithMainSocketPath(testPath);
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT_EQUAL(testPath, options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
    }

    void testGetOptionValueForHelpOption()
    {
        auto options = OptionsBuilder::createWithHelp();
        CPPUNIT_ASSERT(options->isValid());
        // HELP is a SEPARATE option with no value, should return empty string
        CPPUNIT_ASSERT_EQUAL(std::string(""), options->getOptionValue(Options::Key::HELP));
    }

    void testPrintUsageDoesNotCrash()
    {
        auto options = OptionsBuilder::createEmpty();
        CPPUNIT_ASSERT(options->isValid());
        // Just verify it doesn't crash
        options->printUsage();
        CPPUNIT_ASSERT(true);
    }

    void testVeryLongOptionValue()
    {
        std::string longPath(10000, 'x');
        auto options = OptionsBuilder::createWithMainSocketPath(longPath);
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT_EQUAL(longPath, options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
    }

    void testOptionValueWithTrailingEquals()
    {
        auto options = OptionsBuilder::createFromArgs({"subttxrend-app", "--main-socket-path=/path/equals="});
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT_EQUAL(std::string("/path/equals="),
                           options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
    }

    void testMultipleOptionsInDifferentOrder()
    {
        std::string socketPath = "/tmp/socket";
        std::string configPath = "/etc/config.ini";

        // Order 1: socket then config
        auto options1 = OptionsBuilder::createFromArgs({
            "subttxrend-app",
            "--main-socket-path=" + socketPath,
            "--config-file-path=" + configPath
        });
        CPPUNIT_ASSERT(options1->isValid());
        CPPUNIT_ASSERT_EQUAL(socketPath, options1->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
        CPPUNIT_ASSERT_EQUAL(configPath, options1->getOptionValue(Options::Key::CONFIG_FILE_PATH));

        // Order 2: config then socket
        auto options2 = OptionsBuilder::createFromArgs({
            "subttxrend-app",
            "--config-file-path=" + configPath,
            "--main-socket-path=" + socketPath
        });
        CPPUNIT_ASSERT(options2->isValid());
        CPPUNIT_ASSERT_EQUAL(socketPath, options2->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
        CPPUNIT_ASSERT_EQUAL(configPath, options2->getOptionValue(Options::Key::CONFIG_FILE_PATH));
    }

    void testRealWorldHelpScenario()
    {
        // Simulates: ./subttxrend-app --help
        auto options = OptionsBuilder::createFromArgs({"subttxrend-app", "--help"});
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT(options->hasSeparate());
        CPPUNIT_ASSERT_EQUAL(Options::Key::HELP, options->getSeparate());
    }

    void testRealWorldFullConfigScenario()
    {
        // Simulates: ./subttxrend-app -msp=/tmp/socket -cfp=/etc/custom.ini
        auto options = OptionsBuilder::createFromArgs({
            "subttxrend-app",
            "-msp=/tmp/socket",
            "-cfp=/etc/custom.ini"
        });
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT(!options->hasSeparate());
        CPPUNIT_ASSERT_EQUAL(std::string("/tmp/socket"),
                           options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
        CPPUNIT_ASSERT_EQUAL(std::string("/etc/custom.ini"),
                           options->getOptionValue(Options::Key::CONFIG_FILE_PATH));
    }

    void testRealWorldNoArgumentsScenario()
    {
        // Simulates: ./subttxrend-app (no arguments)
        auto options = OptionsBuilder::createFromArgs({"subttxrend-app"});
        CPPUNIT_ASSERT(options->isValid());
        CPPUNIT_ASSERT(!options->hasSeparate());
        // Should use defaults
        CPPUNIT_ASSERT_EQUAL(std::string(""),
                           options->getOptionValue(Options::Key::MAIN_SOCKET_PATH));
        CPPUNIT_ASSERT_EQUAL(std::string("/etc/subttxrend/config.ini"),
                           options->getOptionValue(Options::Key::CONFIG_FILE_PATH));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OptionsTest);
