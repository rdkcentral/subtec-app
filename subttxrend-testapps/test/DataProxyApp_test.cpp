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
#include <memory>
#include <string>
#include <vector>
#include <cstdlib>

#include "DataProxyApp.hpp"

using namespace subttxrend::testapps;

class DataProxyAppTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( DataProxyAppTest );
    CPPUNIT_TEST(testConstructorWithValidInputs);
    CPPUNIT_TEST(testConstructorWithEmptyAppName);
    CPPUNIT_TEST(testConstructorWithEmptyArguments);
    CPPUNIT_TEST(testConstructorWithSingleArgument);
    CPPUNIT_TEST(testConstructorWithTwoArguments);
    CPPUNIT_TEST(testConstructorWithMultipleArguments);
    CPPUNIT_TEST(testConstructorWithLongAppName);
    CPPUNIT_TEST(testConstructorWithSpecialCharactersInAppName);
    CPPUNIT_TEST(testConstructorWithUnicodeInAppName);
    CPPUNIT_TEST(testConstructorWithArgumentsContainingSpaces);
    CPPUNIT_TEST(testConstructorWithArgumentsContainingSpecialChars);
    CPPUNIT_TEST(testRunWithNoArguments);
    CPPUNIT_TEST(testRunWithOneArgument);
    CPPUNIT_TEST(testRunWithThreeArguments);
    CPPUNIT_TEST(testRunWithFiveArguments);
    CPPUNIT_TEST(testRunWithTenArguments);
    CPPUNIT_TEST(testRunWithBothPathsEmpty);
    CPPUNIT_TEST(testRunWithEmptySourcePath);
    CPPUNIT_TEST(testRunWithEmptyTargetPath);
    CPPUNIT_TEST(testRunWithFirstArgumentEmpty);
    CPPUNIT_TEST(testRunWithSecondArgumentEmpty);
    CPPUNIT_TEST(testRunWithBothArgumentsEmptyStrings);
    CPPUNIT_TEST(testRunWithInvalidSourcePrefix);
    CPPUNIT_TEST(testRunWithInvalidTargetPrefix);
    CPPUNIT_TEST(testRunWithBothInvalidPrefixes);
    CPPUNIT_TEST(testRunWithNoColonInSourcePath);
    CPPUNIT_TEST(testRunWithNoColonInTargetPath);
    CPPUNIT_TEST(testRunWithOnlyColonInSourcePath);
    CPPUNIT_TEST(testRunWithOnlyColonInTargetPath);
    CPPUNIT_TEST(testRunWithUppercaseSourcePrefix);
    CPPUNIT_TEST(testRunWithUppercaseTargetPrefix);
    CPPUNIT_TEST(testRunWithSourcePrefixColonAtEnd);
    CPPUNIT_TEST(testRunWithTargetPrefixColonAtEnd);
    CPPUNIT_TEST(testRunWithInvalidFileSourcePath);
    CPPUNIT_TEST(testRunWithInvalidFileTargetPath);
    CPPUNIT_TEST(testRunWithValidRandomSourceAndConsoleTarget);
    CPPUNIT_TEST(testRunWithValidRandomSourceMultiplePackets);
    CPPUNIT_TEST(testRunWithVeryLongSourcePath);
    CPPUNIT_TEST(testRunWithVeryLongTargetPath);
    CPPUNIT_TEST(testRunWithPathsContainingMultipleColons);
    CPPUNIT_TEST(testRunWithPathsContainingSpaces);
    CPPUNIT_TEST(testRunWithPathsContainingSpecialCharacters);
    CPPUNIT_TEST(testMultipleAppInstancesWithDifferentArguments);

CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // Setup code here if needed
    }

    void tearDown()
    {
        // Cleanup code here if needed
    }

protected:
    void testConstructorWithValidInputs()
    {
        std::vector<std::string> args = {"source:path", "target:path"};
        DataProxyApp app("testApp", args);
        // If constructor completes without exception, test passes
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithEmptyAppName()
    {
        std::vector<std::string> args = {"source:path", "target:path"};
        DataProxyApp app("", args);
        // Should not throw - empty app name is technically valid
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithEmptyArguments()
    {
        std::vector<std::string> args;
        DataProxyApp app("testApp", args);
        // Constructor should succeed even with empty arguments
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithSingleArgument()
    {
        std::vector<std::string> args = {"single:argument"};
        DataProxyApp app("testApp", args);
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithTwoArguments()
    {
        std::vector<std::string> args = {"first:arg", "second:arg"};
        DataProxyApp app("testApp", args);
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithMultipleArguments()
    {
        std::vector<std::string> args = {"arg1", "arg2", "arg3", "arg4", "arg5"};
        DataProxyApp app("testApp", args);
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithLongAppName()
    {
        std::string longName(1000, 'x');
        std::vector<std::string> args = {"source", "target"};
        DataProxyApp app(longName, args);
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithSpecialCharactersInAppName()
    {
        std::vector<std::string> args = {"source", "target"};
        DataProxyApp app("test@app#123!$%", args);
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithUnicodeInAppName()
    {
        std::vector<std::string> args = {"source", "target"};
        DataProxyApp app("测试应用", args);
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithArgumentsContainingSpaces()
    {
        std::vector<std::string> args = {"file:/path with spaces/file.txt", "target: also has spaces"};
        DataProxyApp app("testApp", args);
        CPPUNIT_ASSERT(true);
    }

    void testConstructorWithArgumentsContainingSpecialChars()
    {
        std::vector<std::string> args = {"file:/tmp/@#$%/file", "console:!@#"};
        DataProxyApp app("testApp", args);
        CPPUNIT_ASSERT(true);
    }

    void testRunWithNoArguments()
    {
        std::vector<std::string> args;
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithOneArgument()
    {
        std::vector<std::string> args = {"single:arg"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithThreeArguments()
    {
        std::vector<std::string> args = {"arg1", "arg2", "arg3"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithFiveArguments()
    {
        std::vector<std::string> args = {"arg1", "arg2", "arg3", "arg4", "arg5"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithTenArguments()
    {
        std::vector<std::string> args = {"a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithBothPathsEmpty()
    {
        std::vector<std::string> args = {"", ""};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithEmptySourcePath()
    {
        std::vector<std::string> args = {"", "console:"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithEmptyTargetPath()
    {
        std::vector<std::string> args = {"rand:1:0", ""};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithFirstArgumentEmpty()
    {
        std::vector<std::string> args = {"", "file:/tmp/test"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithSecondArgumentEmpty()
    {
        std::vector<std::string> args = {"file:/tmp/test", ""};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithBothArgumentsEmptyStrings()
    {
        std::vector<std::string> args = {"", ""};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithInvalidSourcePrefix()
    {
        std::vector<std::string> args = {"invalid:path", "console:"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithInvalidTargetPrefix()
    {
        std::vector<std::string> args = {"rand:1:0", "invalid:path"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithBothInvalidPrefixes()
    {
        std::vector<std::string> args = {"badsrc:path", "badtgt:path"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithNoColonInSourcePath()
    {
        std::vector<std::string> args = {"nosourcepath", "console:"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithNoColonInTargetPath()
    {
        std::vector<std::string> args = {"rand:1:0", "notargetpath"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithOnlyColonInSourcePath()
    {
        std::vector<std::string> args = {":", "console:"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithOnlyColonInTargetPath()
    {
        std::vector<std::string> args = {"rand:1:0", ":"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithUppercaseSourcePrefix()
    {
        std::vector<std::string> args = {"RAND:1:0", "console:"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithUppercaseTargetPrefix()
    {
        std::vector<std::string> args = {"rand:1:0", "CONSOLE:"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithSourcePrefixColonAtEnd()
    {
        // Prefix with ':' but no path data.
        // Using file: here ensures the app fails fast ("rand:" would run indefinitely
        // with ConsoleLogTarget because RandomPacketSource defaults to unlimited packets).
        std::vector<std::string> args = {"file:", "console:"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithTargetPrefixColonAtEnd()
    {
        // Prefix with ':' but no path data.
        std::vector<std::string> args = {"rand:1:0", "file:"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithInvalidFileSourcePath()
    {
        // Non-existent file should cause open() to fail
        std::vector<std::string> args = {"file:/nonexistent/path/file12345.txt", "console:"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithInvalidFileTargetPath()
    {
        // Invalid target path (directory doesn't exist)
        std::vector<std::string> args = {"rand:1:0", "file:/nonexistent/dir12345/output.txt"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithValidRandomSourceAndConsoleTarget()
    {
        // rand:1:0 creates 1 packet with 0ms sleep, console: outputs to console
        std::vector<std::string> args = {"rand:1:0", "console:"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_SUCCESS, result);
    }

    void testRunWithValidRandomSourceMultiplePackets()
    {
        // rand:5:0 creates 5 packets with 0ms sleep
        std::vector<std::string> args = {"rand:5:0", "console:"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        CPPUNIT_ASSERT_EQUAL(EXIT_SUCCESS, result);
    }

    void testRunWithVeryLongSourcePath()
    {
        std::string longPath = "rand:";
        longPath.append(1000, '1');
        std::vector<std::string> args = {longPath, "console:"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        // RandomPacketSource expects either empty params or "<count>:<sleepMs>".
        // A long digit-only params string is invalid and should fail during open().
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithVeryLongTargetPath()
    {
        std::string longPath = "console:";
        longPath.append(1000, 'x');
        std::vector<std::string> args = {"rand:1:0", longPath};
        DataProxyApp app("testApp", args);
        int result = app.run();
        // ConsoleLogTarget::open() always succeeds regardless of extra path data.
        CPPUNIT_ASSERT_EQUAL(EXIT_SUCCESS, result);
    }

    void testRunWithPathsContainingMultipleColons()
    {
        std::vector<std::string> args = {"rand:1:0:extra:colons", "console:extra:data"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        // Factories split only on the first ':', but RandomPacketSource parsing rejects
        // additional trailing characters (it expects exactly "<count>:<sleepMs>").
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithPathsContainingSpaces()
    {
        std::vector<std::string> args = {"rand:1 0", "console: "};
        DataProxyApp app("testApp", args);
        int result = app.run();
        // RandomPacketSource parsing requires ':' separator.
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testRunWithPathsContainingSpecialCharacters()
    {
        std::vector<std::string> args = {"rand:1:0!@#", "console:$%^&*()"};
        DataProxyApp app("testApp", args);
        int result = app.run();
        // RandomPacketSource rejects trailing non-numeric characters after sleepMs.
        CPPUNIT_ASSERT_EQUAL(EXIT_FAILURE, result);
    }

    void testMultipleAppInstancesWithDifferentArguments()
    {
        std::vector<std::string> args1 = {"rand:1:0", "console:"};
        std::vector<std::string> args2 = {"rand:2:0", "console:"};
        std::vector<std::string> args3 = {"rand:3:0", "console:"};

        DataProxyApp app1("testApp1", args1);
        DataProxyApp app2("testApp2", args2);
        DataProxyApp app3("testApp3", args3);

        int result1 = app1.run();
        int result2 = app2.run();
        int result3 = app3.run();

        CPPUNIT_ASSERT_EQUAL(EXIT_SUCCESS, result1);
        CPPUNIT_ASSERT_EQUAL(EXIT_SUCCESS, result2);
        CPPUNIT_ASSERT_EQUAL(EXIT_SUCCESS, result3);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( DataProxyAppTest );
