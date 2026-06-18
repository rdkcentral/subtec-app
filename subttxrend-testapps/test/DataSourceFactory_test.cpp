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
#include <type_traits>
#include <vector>
#include <limits>

#include "DataSourceFactory.hpp"
#include "Ipv4SocketSource.hpp"
#include "PlainFileSource.hpp"
#include "RandomPacketSource.hpp"
#include "SmartFileSource.hpp"
#include "TtmlFileSource.hpp"
#include "UnixSocketSource.hpp"
#include "WebvttFileNoHeaderSource.hpp"
#include "WebvttFileSource.hpp"

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
    template<typename SourceType>
    std::unique_ptr<DataSource> assertCreatedSource(const std::string& factoryPath,
                                                    const std::string& expectedPath)
    {
        static_assert(std::is_base_of<DataSource, SourceType>::value,
                      "SourceType must derive from DataSource");

        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource(factoryPath);

        CPPUNIT_ASSERT(source != nullptr);
        CPPUNIT_ASSERT(dynamic_cast<SourceType*>(source.get()) != nullptr);
        CPPUNIT_ASSERT_EQUAL(expectedPath, source->getPath());

        return source;
    }

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
        assertCreatedSource<Ipv4SocketSource>("ipv4:192.168.1.1:8080", "192.168.1.1:8080");
    }

    void testCreateSourceWithUnixPath()
    {
        assertCreatedSource<UnixSocketSource>("unix:/tmp/socket", "/tmp/socket");
    }

    void testCreateSourceWithFilePath()
    {
        assertCreatedSource<PlainFileSource>("file:/path/to/file.txt", "/path/to/file.txt");
    }

    void testCreateSourceWithTtmlPath()
    {
        assertCreatedSource<TtmlFileSource>("ttml:/path/to/file.ttml", "/path/to/file.ttml");
    }

    void testCreateSourceWithWvttPath()
    {
        assertCreatedSource<WebvttFileSource>("wvtt:/path/to/file.vtt", "/path/to/file.vtt");
    }

    void testCreateSourceWithWvttnhPath()
    {
        assertCreatedSource<WebvttFileNoHeaderSource>("wvttnh:/path/to/file.vtt", "/path/to/file.vtt");
    }

    void testCreateSourceWithSfilePath()
    {
        assertCreatedSource<SmartFileSource>("sfile:/path/to/file", "/path/to/file");
    }

    void testCreateSourceWithRandPath()
    {
        std::unique_ptr<DataSource> source = assertCreatedSource<RandomPacketSource>("rand:1:0", "1:0");

        CPPUNIT_ASSERT(source->open());

        DataPacket packet(1024);
        CPPUNIT_ASSERT(source->readPacket(packet));
        CPPUNIT_ASSERT(packet.getSize() > 0);

        source->close();
    }

    void testCreateSourceWithMinimalPathAfterColon()
    {
        assertCreatedSource<PlainFileSource>("file:", "");
    }

    void testCreateSourceWithMultipleColons()
    {
        assertCreatedSource<PlainFileSource>("file:path:with:colons", "path:with:colons");
    }

    void testCreateSourceWithPathContainingSpaces()
    {
        assertCreatedSource<PlainFileSource>("file:/path/with spaces/file.txt", "/path/with spaces/file.txt");
    }

    void testCreateSourceWithVeryLongPath()
    {
        std::string longPath = "file:";
        longPath.append(2000, 'x');
        assertCreatedSource<PlainFileSource>(longPath, std::string(2000, 'x'));
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
        assertCreatedSource<Ipv4SocketSource>("ipv4:", "");
    }

    void testCreateSourceWithUppercasePrefix()
    {
        DataSourceFactory factory;
        std::unique_ptr<DataSource> source = factory.createSource("IPV4:address");
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
        assertCreatedSource<PlainFileSource>("file:/path/file@#$.txt", "/path/file@#$.txt");
    }

    void testFactoryCanCreateMultipleDifferentSourcesSequentially()
    {
        std::unique_ptr<DataSource> source1 = assertCreatedSource<PlainFileSource>("file:/path1", "/path1");
        std::unique_ptr<DataSource> source2 = assertCreatedSource<UnixSocketSource>("unix:/tmp/socket", "/tmp/socket");
        std::unique_ptr<DataSource> source3 = assertCreatedSource<Ipv4SocketSource>("ipv4:127.0.0.1:9000", "127.0.0.1:9000");
        std::unique_ptr<DataSource> source4 = assertCreatedSource<TtmlFileSource>("ttml:/ttml/file", "/ttml/file");
    }

    void testFactoryCanCreateSameSourceTypeMultipleTimes()
    {
        std::unique_ptr<DataSource> source1 = assertCreatedSource<PlainFileSource>("file:/path1", "/path1");
        std::unique_ptr<DataSource> source2 = assertCreatedSource<PlainFileSource>("file:/path2", "/path2");
        std::unique_ptr<DataSource> source3 = assertCreatedSource<PlainFileSource>("file:/path3", "/path3");

        // Verify they are different instances
        CPPUNIT_ASSERT(source1.get() != source2.get());
        CPPUNIT_ASSERT(source2.get() != source3.get());
    }

    void testAllEightSourceTypesCanBeCreated()
    {
        assertCreatedSource<Ipv4SocketSource>("ipv4:127.0.0.1", "127.0.0.1");
        assertCreatedSource<UnixSocketSource>("unix:/tmp/sock", "/tmp/sock");
        assertCreatedSource<PlainFileSource>("file:/file", "/file");
        assertCreatedSource<TtmlFileSource>("ttml:/ttml", "/ttml");
        assertCreatedSource<WebvttFileSource>("wvtt:/wvtt", "/wvtt");
        assertCreatedSource<WebvttFileNoHeaderSource>("wvttnh:/wvttnh", "/wvttnh");
        assertCreatedSource<SmartFileSource>("sfile:/sfile", "/sfile");
        assertCreatedSource<RandomPacketSource>("rand:10:5", "10:5");
    }

    void testCreateManySourcesRapidly()
    {
        std::vector<std::unique_ptr<DataSource>> sources;

        // Create 100 sources rapidly
        for (int i = 0; i < 100; ++i)
        {
            std::string path = "file:/path" + std::to_string(i);
            std::unique_ptr<DataSource> source = assertCreatedSource<PlainFileSource>(path,
                                                                                       "/path" + std::to_string(i));
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
