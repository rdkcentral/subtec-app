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
#include <stdexcept>
#include <memory>
#include <string>
#include <vector>
#include <limits>

#include "DataSourceFactory.hpp"

using namespace subttxrend::testapps;

class DataSourceFactoryTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( DataSourceFactoryTest );
    CPPUNIT_TEST(testFactoryConstruction);
    CPPUNIT_TEST(testFactoryRegistersEightSourceTypes);
    CPPUNIT_TEST(testAllRegisteredTypesHaveCorrectPrefixes);
    CPPUNIT_TEST(testAllRegisteredTypesHaveNonEmptyDescriptions);
    CPPUNIT_TEST(testGetTypeCountReturnsCorrectValue);
    CPPUNIT_TEST(testGetTypeCountIsConsistentAcrossCalls);
    CPPUNIT_TEST(testGetTypeInfoWithIndexZero);
    CPPUNIT_TEST(testGetTypeInfoWithIndexSeven);
    CPPUNIT_TEST(testGetTypeInfoWithMiddleIndex);
    CPPUNIT_TEST(testGetTypeInfoReturnsCorrectPrefix);
    CPPUNIT_TEST(testGetTypeInfoReturnsCorrectDescription);
    CPPUNIT_TEST(testGetTypeInfoWithIndexEqualToCount);
    CPPUNIT_TEST(testGetTypeInfoWithIndexGreaterThanCount);
    CPPUNIT_TEST(testGetTypeInfoWithMaxSizeT);
    CPPUNIT_TEST(testGetTypeInfoExceptionTypeIsLengthError);
    CPPUNIT_TEST(testCreateSourceWithIpv4Path);
    CPPUNIT_TEST(testCreateSourceWithUnixPath);
    CPPUNIT_TEST(testCreateSourceWithFilePath);
    CPPUNIT_TEST(testCreateSourceWithTtmlPath);
    CPPUNIT_TEST(testCreateSourceWithWvttPath);
    CPPUNIT_TEST(testCreateSourceWithWvttnhPath);
    CPPUNIT_TEST(testCreateSourceWithSfilePath);
    CPPUNIT_TEST(testCreateSourceWithRandPath);
    CPPUNIT_TEST(testCreateSourceWithMinimalPathAfterColon);
    CPPUNIT_TEST(testCreateSourceWithMultipleColons);
    CPPUNIT_TEST(testCreateSourceWithPathContainingSpaces);
    CPPUNIT_TEST(testCreateSourceWithVeryLongPath);
    CPPUNIT_TEST(testCreateSourceWithEmptyString);
    CPPUNIT_TEST(testCreateSourceWithNoColon);
    CPPUNIT_TEST(testCreateSourceWithUnknownPrefix);
    CPPUNIT_TEST(testCreateSourceWithOnlyColon);
    CPPUNIT_TEST(testCreateSourceWithColonAtEnd);
    CPPUNIT_TEST(testCreateSourceWithUppercasePrefix);
    CPPUNIT_TEST(testCreateSourceWithMixedCasePrefix);
    CPPUNIT_TEST(testCreateSourceWithLeadingWhitespace);
    CPPUNIT_TEST(testCreateSourceWithTrailingWhitespaceInPrefix);
    CPPUNIT_TEST(testCreateSourceWithColonAsFirstCharacter);
    CPPUNIT_TEST(testCreateSourceWithSpecialCharacters);
    CPPUNIT_TEST(testFactoryCanCreateMultipleDifferentSourcesSequentially);
    CPPUNIT_TEST(testFactoryCanCreateSameSourceTypeMultipleTimes);
    CPPUNIT_TEST(testAllEightSourceTypesCanBeCreated);
    CPPUNIT_TEST(testCreateManySourcesRapidly);

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
    void testFactoryConstruction()
    {
        // Verify factory can be constructed without exceptions
        DataSourceFactory factory;
        CPPUNIT_ASSERT(factory.getTypeCount() > 0);
    }

    void testFactoryRegistersEightSourceTypes()
    {
        DataSourceFactory factory;
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(8), factory.getTypeCount());
    }

    void testAllRegisteredTypesHaveCorrectPrefixes()
    {
        DataSourceFactory factory;
        std::vector<std::string> expectedPrefixes = {
            "ipv4", "unix", "file", "ttml", "wvtt", "wvttnh", "sfile", "rand"
        };

        CPPUNIT_ASSERT_EQUAL(expectedPrefixes.size(), factory.getTypeCount());

        for (std::size_t i = 0; i < factory.getTypeCount(); ++i)
        {
            const DataSourceFactoryEntry& entry = factory.getTypeInfo(i);
            CPPUNIT_ASSERT_EQUAL(expectedPrefixes[i], entry.getPrefix());
        }
    }

    void testAllRegisteredTypesHaveNonEmptyDescriptions()
    {
        DataSourceFactory factory;

        for (std::size_t i = 0; i < factory.getTypeCount(); ++i)
        {
            const DataSourceFactoryEntry& entry = factory.getTypeInfo(i);
            CPPUNIT_ASSERT(!entry.getDescription().empty());
        }
    }

    void testGetTypeCountReturnsCorrectValue()
    {
        DataSourceFactory factory;
        std::size_t count = factory.getTypeCount();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(8), count);
    }

    void testGetTypeCountIsConsistentAcrossCalls()
    {
        DataSourceFactory factory;
        std::size_t count1 = factory.getTypeCount();
        std::size_t count2 = factory.getTypeCount();
        std::size_t count3 = factory.getTypeCount();

        CPPUNIT_ASSERT_EQUAL(count1, count2);
        CPPUNIT_ASSERT_EQUAL(count2, count3);
    }

    void testGetTypeInfoWithIndexZero()
    {
        DataSourceFactory factory;
        const DataSourceFactoryEntry& entry = factory.getTypeInfo(0);
        CPPUNIT_ASSERT_EQUAL(std::string("ipv4"), entry.getPrefix());
    }

    void testGetTypeInfoWithIndexSeven()
    {
        DataSourceFactory factory;
        const DataSourceFactoryEntry& entry = factory.getTypeInfo(7);
        CPPUNIT_ASSERT_EQUAL(std::string("rand"), entry.getPrefix());
    }

    void testGetTypeInfoWithMiddleIndex()
    {
        DataSourceFactory factory;
        const DataSourceFactoryEntry& entry = factory.getTypeInfo(3);
        CPPUNIT_ASSERT_EQUAL(std::string("ttml"), entry.getPrefix());
    }

    void testGetTypeInfoReturnsCorrectPrefix()
    {
        DataSourceFactory factory;
        const DataSourceFactoryEntry& entry = factory.getTypeInfo(1);
        CPPUNIT_ASSERT_EQUAL(std::string("unix"), entry.getPrefix());
    }

    void testGetTypeInfoReturnsCorrectDescription()
    {
        DataSourceFactory factory;
        const DataSourceFactoryEntry& entry = factory.getTypeInfo(2);
        std::string description = entry.getDescription();
        CPPUNIT_ASSERT(!description.empty());
        CPPUNIT_ASSERT(description.find("file") != std::string::npos ||
                       description.find("Plain") != std::string::npos);
    }

    void testGetTypeInfoWithIndexEqualToCount()
    {
        DataSourceFactory factory;
        std::size_t count = factory.getTypeCount();

        try
        {
            factory.getTypeInfo(count);
            CPPUNIT_FAIL("Expected std::length_error exception");
        }
        catch (const std::length_error& e)
        {
            // Expected exception
            CPPUNIT_ASSERT(std::string(e.what()).find("out of range") != std::string::npos);
        }
        catch (...)
        {
            CPPUNIT_FAIL("Wrong exception type thrown");
        }
    }

    void testGetTypeInfoWithIndexGreaterThanCount()
    {
        DataSourceFactory factory;

        try
        {
            factory.getTypeInfo(100);
            CPPUNIT_FAIL("Expected std::length_error exception");
        }
        catch (const std::length_error&)
        {
            // Expected exception
        }
        catch (...)
        {
            CPPUNIT_FAIL("Wrong exception type thrown");
        }
    }

    void testGetTypeInfoWithMaxSizeT()
    {
        DataSourceFactory factory;

        try
        {
            factory.getTypeInfo(std::numeric_limits<std::size_t>::max());
            CPPUNIT_FAIL("Expected std::length_error exception");
        }
        catch (const std::length_error&)
        {
            // Expected exception
        }
        catch (...)
        {
            CPPUNIT_FAIL("Wrong exception type thrown");
        }
    }

    void testGetTypeInfoExceptionTypeIsLengthError()
    {
        DataSourceFactory factory;
        bool caughtCorrectException = false;

        try
        {
            factory.getTypeInfo(999);
        }
        catch (const std::length_error&)
        {
            caughtCorrectException = true;
        }
        catch (...)
        {
            // Wrong exception type
        }

        CPPUNIT_ASSERT(caughtCorrectException);
    }

    void testCreateSourceWithIpv4Path()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("ipv4:192.168.1.1:8080");
        CPPUNIT_ASSERT(source != nullptr);
    }

    void testCreateSourceWithUnixPath()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("unix:/tmp/socket");
        CPPUNIT_ASSERT(source != nullptr);
    }

    void testCreateSourceWithFilePath()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("file:/path/to/file.txt");
        CPPUNIT_ASSERT(source != nullptr);
    }

    void testCreateSourceWithTtmlPath()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("ttml:/path/to/file.ttml");
        CPPUNIT_ASSERT(source != nullptr);
    }

    void testCreateSourceWithWvttPath()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("wvtt:/path/to/file.vtt");
        CPPUNIT_ASSERT(source != nullptr);
    }

    void testCreateSourceWithWvttnhPath()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("wvttnh:/path/to/file.vtt");
        CPPUNIT_ASSERT(source != nullptr);
    }

    void testCreateSourceWithSfilePath()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("sfile:/path/to/file");
        CPPUNIT_ASSERT(source != nullptr);
    }

    void testCreateSourceWithRandPath()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("rand:100:50");
        CPPUNIT_ASSERT(source != nullptr);
    }

    void testCreateSourceWithMinimalPathAfterColon()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("file:");
        CPPUNIT_ASSERT(source != nullptr);
    }

    void testCreateSourceWithMultipleColons()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("file:path:with:colons");
        CPPUNIT_ASSERT(source != nullptr);
    }

    void testCreateSourceWithPathContainingSpaces()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("file:/path/with spaces/file.txt");
        CPPUNIT_ASSERT(source != nullptr);
    }

    void testCreateSourceWithVeryLongPath()
    {
        DataSourceFactory factory;
        std::string longPath = "file:";
        longPath.append(2000, 'x');
        std::unique_ptr<DataSource> source = factory.createSource(longPath);
        CPPUNIT_ASSERT(source != nullptr);
    }

    void testCreateSourceWithEmptyString()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("");
        CPPUNIT_ASSERT(source == nullptr);
    }

    void testCreateSourceWithNoColon()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("justtext");
        CPPUNIT_ASSERT(source == nullptr);
    }

    void testCreateSourceWithUnknownPrefix()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("unknown:path");
        CPPUNIT_ASSERT(source == nullptr);
    }

    void testCreateSourceWithOnlyColon()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource(":");
        CPPUNIT_ASSERT(source == nullptr);
    }

    void testCreateSourceWithColonAtEnd()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("ipv4");
        CPPUNIT_ASSERT(source == nullptr);
    }

    void testCreateSourceWithUppercasePrefix()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("IPIV4:address");
        CPPUNIT_ASSERT(source == nullptr);
    }

    void testCreateSourceWithMixedCasePrefix()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("IPv4:address");
        CPPUNIT_ASSERT(source == nullptr);
    }

    void testCreateSourceWithLeadingWhitespace()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource(" file:path");
        CPPUNIT_ASSERT(source == nullptr);
    }

    void testCreateSourceWithTrailingWhitespaceInPrefix()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("file :path");
        CPPUNIT_ASSERT(source == nullptr);
    }

    void testCreateSourceWithColonAsFirstCharacter()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource(":path");
        CPPUNIT_ASSERT(source == nullptr);
    }

    void testCreateSourceWithSpecialCharacters()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("file:/path/file@#$.txt");
        CPPUNIT_ASSERT(source != nullptr);
    }

    void testFactoryCanCreateMultipleDifferentSourcesSequentially()
    {
        DataSourceFactory factory;

        std::unique_ptr<DataSource> source1 = factory.createSource("file:/path1");
        CPPUNIT_ASSERT(source1 != nullptr);

        std::unique_ptr<DataSource> source2 = factory.createSource("unix:/tmp/socket");
        CPPUNIT_ASSERT(source2 != nullptr);

        std::unique_ptr<DataSource> source3 = factory.createSource("ipv4:127.0.0.1:9000");
        CPPUNIT_ASSERT(source3 != nullptr);

        std::unique_ptr<DataSource> source4 = factory.createSource("ttml:/ttml/file");
        CPPUNIT_ASSERT(source4 != nullptr);
    }

    void testFactoryCanCreateSameSourceTypeMultipleTimes()
    {
        DataSourceFactory factory;

        std::unique_ptr<DataSource> source1 = factory.createSource("file:/path1");
        CPPUNIT_ASSERT(source1 != nullptr);

        std::unique_ptr<DataSource> source2 = factory.createSource("file:/path2");
        CPPUNIT_ASSERT(source2 != nullptr);

        std::unique_ptr<DataSource> source3 = factory.createSource("file:/path3");
        CPPUNIT_ASSERT(source3 != nullptr);

        // Verify they are different instances
        CPPUNIT_ASSERT(source1.get() != source2.get());
        CPPUNIT_ASSERT(source2.get() != source3.get());
    }

    void testAllEightSourceTypesCanBeCreated()
    {
        DataSourceFactory factory;

        std::unique_ptr<DataSource> ipv4 = factory.createSource("ipv4:127.0.0.1");
        std::unique_ptr<DataSource> unixSock = factory.createSource("unix:/tmp/sock");
        std::unique_ptr<DataSource> file = factory.createSource("file:/file");
        std::unique_ptr<DataSource> ttml = factory.createSource("ttml:/ttml");
        std::unique_ptr<DataSource> wvtt = factory.createSource("wvtt:/wvtt");
        std::unique_ptr<DataSource> wvttnh = factory.createSource("wvttnh:/wvttnh");
        std::unique_ptr<DataSource> sfile = factory.createSource("sfile:/sfile");
        std::unique_ptr<DataSource> rand = factory.createSource("rand:10:5");

        CPPUNIT_ASSERT(ipv4 != nullptr);
        CPPUNIT_ASSERT(unixSock != nullptr);
        CPPUNIT_ASSERT(file != nullptr);
        CPPUNIT_ASSERT(ttml != nullptr);
        CPPUNIT_ASSERT(wvtt != nullptr);
        CPPUNIT_ASSERT(wvttnh != nullptr);
        CPPUNIT_ASSERT(sfile != nullptr);
        CPPUNIT_ASSERT(rand != nullptr);
    }

    void testCreateManySourcesRapidly()
    {
        DataSourceFactory factory;
        std::vector<std::unique_ptr<DataSource>> sources;

        // Create 100 sources rapidly
        for (int i = 0; i < 100; ++i)
        {
            std::string path = "file:/path" + std::to_string(i);
            std::unique_ptr<DataSource> source = factory.createSource(path);
            CPPUNIT_ASSERT(source != nullptr);
            sources.push_back(std::move(source));
        }

        // Verify all were created
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(100), sources.size());

        // Verify they are all different instances
        for (std::size_t i = 0; i < sources.size(); ++i)
        {
            for (std::size_t j = i + 1; j < sources.size(); ++j)
            {
                CPPUNIT_ASSERT(sources[i].get() != sources[j].get());
            }
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( DataSourceFactoryTest );
