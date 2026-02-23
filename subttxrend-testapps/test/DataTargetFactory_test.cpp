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

#include "DataTargetFactory.hpp"

using namespace subttxrend::testapps;

class DataTargetFactoryTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( DataTargetFactoryTest );
    CPPUNIT_TEST(testFactoryConstruction);
    CPPUNIT_TEST(testFactoryRegistersFiveTargetTypes);
    CPPUNIT_TEST(testAllRegisteredTypesHaveCorrectPrefixes);
    CPPUNIT_TEST(testAllRegisteredTypesHaveNonEmptyDescriptions);
    CPPUNIT_TEST(testGetTypeCountReturnsCorrectValue);
    CPPUNIT_TEST(testGetTypeCountIsConsistentAcrossCalls);
    CPPUNIT_TEST(testGetTypeInfoWithIndexZero);
    CPPUNIT_TEST(testGetTypeInfoWithIndexFour);
    CPPUNIT_TEST(testGetTypeInfoWithMiddleIndex);
    CPPUNIT_TEST(testGetTypeInfoReturnsCorrectPrefix);
    CPPUNIT_TEST(testGetTypeInfoReturnsCorrectDescription);
    CPPUNIT_TEST(testGetTypeInfoReturnsValidReference);
    CPPUNIT_TEST(testGetTypeInfoWithIndexEqualToCount);
    CPPUNIT_TEST(testGetTypeInfoWithIndexGreaterThanCount);
    CPPUNIT_TEST(testGetTypeInfoWithMaxSizeT);
    CPPUNIT_TEST(testGetTypeInfoExceptionTypeIsLengthError);
    CPPUNIT_TEST(testCreateTargetWithIpv4Path);
    CPPUNIT_TEST(testCreateTargetWithUnixPath);
    CPPUNIT_TEST(testCreateTargetWithFilePath);
    CPPUNIT_TEST(testCreateTargetWithSfilePath);
    CPPUNIT_TEST(testCreateTargetWithConsolePath);
    CPPUNIT_TEST(testCreateTargetWithMinimalPathAfterColon);
    CPPUNIT_TEST(testCreateTargetWithMultipleColons);
    CPPUNIT_TEST(testCreateTargetWithPathContainingSpaces);
    CPPUNIT_TEST(testCreateTargetWithVeryLongPath);
    CPPUNIT_TEST(testCreateTargetWithEmptyString);
    CPPUNIT_TEST(testCreateTargetWithNoColon);
    CPPUNIT_TEST(testCreateTargetWithUnknownPrefix);
    CPPUNIT_TEST(testCreateTargetWithOnlyColon);
    CPPUNIT_TEST(testCreateTargetWithColonAtEnd);
    CPPUNIT_TEST(testCreateTargetWithUppercasePrefix);
    CPPUNIT_TEST(testCreateTargetWithMixedCasePrefix);
    CPPUNIT_TEST(testCreateTargetWithLeadingWhitespace);
    CPPUNIT_TEST(testCreateTargetWithTrailingWhitespaceInPrefix);
    CPPUNIT_TEST(testCreateTargetWithColonAsFirstCharacter);
    CPPUNIT_TEST(testCreateTargetWithSpecialCharacters);
    CPPUNIT_TEST(testFactoryCanCreateMultipleDifferentTargetsSequentially);
    CPPUNIT_TEST(testFactoryCanCreateSameTargetTypeMultipleTimes);
    CPPUNIT_TEST(testAllFiveTargetTypesCanBeCreated);
    CPPUNIT_TEST(testCreatedTargetsAreDifferentInstances);
    CPPUNIT_TEST(testFactoryIsReusableAfterTargetCreation);
    CPPUNIT_TEST(testCreateManyTargetsRapidly);
    CPPUNIT_TEST(testMultipleFactoriesCanExistSimultaneously);
    CPPUNIT_TEST(testFactoryEntryPrefixIsImmutable);
    CPPUNIT_TEST(testFactoryEntryDescriptionIsImmutable);
    CPPUNIT_TEST(testCreateTargetWithPathContainingNewline);
    CPPUNIT_TEST(testCreateTargetWithPathContainingTabs);

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
        DataTargetFactory factory;
        CPPUNIT_ASSERT(factory.getTypeCount() > 0);
    }

    void testFactoryRegistersFiveTargetTypes()
    {
        DataTargetFactory factory;
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(5), factory.getTypeCount());
    }

    void testAllRegisteredTypesHaveCorrectPrefixes()
    {
        DataTargetFactory factory;
        std::vector<std::string> expectedPrefixes = {
            "ipv4", "unix", "file", "sfile", "console"
        };

        CPPUNIT_ASSERT_EQUAL(expectedPrefixes.size(), factory.getTypeCount());

        for (std::size_t i = 0; i < factory.getTypeCount(); ++i)
        {
            const DataTargetFactoryEntry& entry = factory.getTypeInfo(i);
            CPPUNIT_ASSERT_EQUAL(expectedPrefixes[i], entry.getPrefix());
        }
    }

    void testAllRegisteredTypesHaveNonEmptyDescriptions()
    {
        DataTargetFactory factory;

        for (std::size_t i = 0; i < factory.getTypeCount(); ++i)
        {
            const DataTargetFactoryEntry& entry = factory.getTypeInfo(i);
            CPPUNIT_ASSERT(!entry.getDescription().empty());
            // Verify description contains the prefix for consistency
            CPPUNIT_ASSERT(entry.getDescription().find(entry.getPrefix()) != std::string::npos);
        }
    }

    void testGetTypeCountReturnsCorrectValue()
    {
        DataTargetFactory factory;
        std::size_t count = factory.getTypeCount();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(5), count);
    }

    void testGetTypeCountIsConsistentAcrossCalls()
    {
        DataTargetFactory factory;
        std::size_t count1 = factory.getTypeCount();
        std::size_t count2 = factory.getTypeCount();
        std::size_t count3 = factory.getTypeCount();

        CPPUNIT_ASSERT_EQUAL(count1, count2);
        CPPUNIT_ASSERT_EQUAL(count2, count3);
    }

    void testGetTypeInfoWithIndexZero()
    {
        DataTargetFactory factory;
        const DataTargetFactoryEntry& entry = factory.getTypeInfo(0);
        CPPUNIT_ASSERT_EQUAL(std::string("ipv4"), entry.getPrefix());
        CPPUNIT_ASSERT(!entry.getDescription().empty());
    }

    void testGetTypeInfoWithIndexFour()
    {
        DataTargetFactory factory;
        const DataTargetFactoryEntry& entry = factory.getTypeInfo(4);
        CPPUNIT_ASSERT_EQUAL(std::string("console"), entry.getPrefix());
        CPPUNIT_ASSERT(!entry.getDescription().empty());
    }

    void testGetTypeInfoWithMiddleIndex()
    {
        DataTargetFactory factory;
        const DataTargetFactoryEntry& entry = factory.getTypeInfo(2);
        CPPUNIT_ASSERT_EQUAL(std::string("file"), entry.getPrefix());
        CPPUNIT_ASSERT(!entry.getDescription().empty());
    }

    void testGetTypeInfoReturnsCorrectPrefix()
    {
        DataTargetFactory factory;
        const DataTargetFactoryEntry& entry = factory.getTypeInfo(1);
        CPPUNIT_ASSERT_EQUAL(std::string("unix"), entry.getPrefix());
    }

    void testGetTypeInfoReturnsCorrectDescription()
    {
        DataTargetFactory factory;
        const DataTargetFactoryEntry& entry = factory.getTypeInfo(3);
        std::string description = entry.getDescription();
        CPPUNIT_ASSERT(!description.empty());
        CPPUNIT_ASSERT(description.find("sfile") != std::string::npos);
    }

    void testGetTypeInfoReturnsValidReference()
    {
        DataTargetFactory factory;
        const DataTargetFactoryEntry& entry1 = factory.getTypeInfo(0);
        const DataTargetFactoryEntry& entry2 = factory.getTypeInfo(0);
        // Both references should point to the same underlying object
        CPPUNIT_ASSERT_EQUAL(&entry1, &entry2);
    }

    void testGetTypeInfoWithIndexEqualToCount()
    {
        DataTargetFactory factory;
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
        DataTargetFactory factory;

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
        DataTargetFactory factory;

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
        DataTargetFactory factory;
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

    void testCreateTargetWithIpv4Path()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("ipv4:192.168.1.1:8080");
        CPPUNIT_ASSERT(target != nullptr);
    }

    void testCreateTargetWithUnixPath()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("unix:/tmp/socket");
        CPPUNIT_ASSERT(target != nullptr);
    }

    void testCreateTargetWithFilePath()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("file:/path/to/file.txt");
        CPPUNIT_ASSERT(target != nullptr);
    }

    void testCreateTargetWithSfilePath()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("sfile:/path/to/file");
        CPPUNIT_ASSERT(target != nullptr);
    }

    void testCreateTargetWithConsolePath()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("console:");
        CPPUNIT_ASSERT(target != nullptr);
    }

    void testCreateTargetWithMinimalPathAfterColon()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("file:");
        CPPUNIT_ASSERT(target != nullptr);
    }

    void testCreateTargetWithMultipleColons()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("file:path:with:colons");
        CPPUNIT_ASSERT(target != nullptr);
    }

    void testCreateTargetWithPathContainingSpaces()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("file:/path/with spaces/file.txt");
        CPPUNIT_ASSERT(target != nullptr);
    }

    void testCreateTargetWithVeryLongPath()
    {
        DataTargetFactory factory;
        std::string longPath = "file:";
        longPath.append(2000, 'x');
        std::unique_ptr<DataTarget> target = factory.createTarget(longPath);
        CPPUNIT_ASSERT(target != nullptr);
    }

    void testCreateTargetWithEmptyString()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("");
        CPPUNIT_ASSERT(target == nullptr);
    }

    void testCreateTargetWithNoColon()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("justtext");
        CPPUNIT_ASSERT(target == nullptr);
    }

    void testCreateTargetWithUnknownPrefix()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("unknown:path");
        CPPUNIT_ASSERT(target == nullptr);
    }

    void testCreateTargetWithOnlyColon()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget(":");
        CPPUNIT_ASSERT(target == nullptr);
    }

    void testCreateTargetWithColonAtEnd()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("ipv4");
        CPPUNIT_ASSERT(target == nullptr);
    }

    void testCreateTargetWithUppercasePrefix()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("IPV4:address");
        CPPUNIT_ASSERT(target == nullptr);
    }

    void testCreateTargetWithMixedCasePrefix()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("IPv4:address");
        CPPUNIT_ASSERT(target == nullptr);
    }

    void testCreateTargetWithLeadingWhitespace()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget(" file:path");
        CPPUNIT_ASSERT(target == nullptr);
    }

    void testCreateTargetWithTrailingWhitespaceInPrefix()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("file :path");
        CPPUNIT_ASSERT(target == nullptr);
    }

    void testCreateTargetWithColonAsFirstCharacter()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget(":path");
        CPPUNIT_ASSERT(target == nullptr);
    }

    void testCreateTargetWithSpecialCharacters()
    {
        DataTargetFactory factory;
        std::unique_ptr<DataTarget> target = factory.createTarget("file:/path/file@#$.txt");
        CPPUNIT_ASSERT(target != nullptr);
    }

    void testFactoryCanCreateMultipleDifferentTargetsSequentially()
    {
        DataTargetFactory factory;

        std::unique_ptr<DataTarget> target1 = factory.createTarget("file:/path1");
        CPPUNIT_ASSERT(target1 != nullptr);

        std::unique_ptr<DataTarget> target2 = factory.createTarget("unix:/tmp/socket");
        CPPUNIT_ASSERT(target2 != nullptr);

        std::unique_ptr<DataTarget> target3 = factory.createTarget("ipv4:127.0.0.1:9000");
        CPPUNIT_ASSERT(target3 != nullptr);

        std::unique_ptr<DataTarget> target4 = factory.createTarget("console:");
        CPPUNIT_ASSERT(target4 != nullptr);
    }

    void testFactoryCanCreateSameTargetTypeMultipleTimes()
    {
        DataTargetFactory factory;

        std::unique_ptr<DataTarget> target1 = factory.createTarget("file:/path1");
        CPPUNIT_ASSERT(target1 != nullptr);

        std::unique_ptr<DataTarget> target2 = factory.createTarget("file:/path2");
        CPPUNIT_ASSERT(target2 != nullptr);

        std::unique_ptr<DataTarget> target3 = factory.createTarget("file:/path3");
        CPPUNIT_ASSERT(target3 != nullptr);

        // Verify they are different instances
        CPPUNIT_ASSERT(target1.get() != target2.get());
        CPPUNIT_ASSERT(target2.get() != target3.get());
        CPPUNIT_ASSERT(target1.get() != target3.get());
    }

    void testAllFiveTargetTypesCanBeCreated()
    {
        DataTargetFactory factory;

        std::unique_ptr<DataTarget> ipv4 = factory.createTarget("ipv4:127.0.0.1:8080");
        std::unique_ptr<DataTarget> unixSock = factory.createTarget("unix:/tmp/sock");
        std::unique_ptr<DataTarget> file = factory.createTarget("file:/file");
        std::unique_ptr<DataTarget> sfile = factory.createTarget("sfile:/sfile");
        std::unique_ptr<DataTarget> console = factory.createTarget("console:");

        CPPUNIT_ASSERT(ipv4 != nullptr);
        CPPUNIT_ASSERT(unixSock != nullptr);
        CPPUNIT_ASSERT(file != nullptr);
        CPPUNIT_ASSERT(sfile != nullptr);
        CPPUNIT_ASSERT(console != nullptr);

        // Verify all are different instances
        CPPUNIT_ASSERT(ipv4.get() != unixSock.get());
        CPPUNIT_ASSERT(ipv4.get() != file.get());
        CPPUNIT_ASSERT(ipv4.get() != sfile.get());
        CPPUNIT_ASSERT(ipv4.get() != console.get());
    }

    void testCreatedTargetsAreDifferentInstances()
    {
        DataTargetFactory factory;

        std::unique_ptr<DataTarget> target1 = factory.createTarget("file:/path1");
        std::unique_ptr<DataTarget> target2 = factory.createTarget("file:/path2");

        CPPUNIT_ASSERT(target1 != nullptr);
        CPPUNIT_ASSERT(target2 != nullptr);
        CPPUNIT_ASSERT(target1.get() != target2.get());

        // Get raw pointers to verify they are different
        DataTarget* ptr1 = target1.get();
        DataTarget* ptr2 = target2.get();
        CPPUNIT_ASSERT(ptr1 != ptr2);
    }

    void testFactoryIsReusableAfterTargetCreation()
    {
        DataTargetFactory factory;

        // Create a target
        std::unique_ptr<DataTarget> target1 = factory.createTarget("file:/first");
        CPPUNIT_ASSERT(target1 != nullptr);

        // Destroy it by resetting
        target1.reset();

        // Factory should still work
        std::unique_ptr<DataTarget> target2 = factory.createTarget("file:/second");
        CPPUNIT_ASSERT(target2 != nullptr);

        // Factory state should be unchanged
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(5), factory.getTypeCount());
    }

    void testCreateManyTargetsRapidly()
    {
        DataTargetFactory factory;
        std::vector<std::unique_ptr<DataTarget>> targets;

        // Create 100 targets rapidly
        for (int i = 0; i < 100; ++i)
        {
            std::string path = "file:/path" + std::to_string(i);
            std::unique_ptr<DataTarget> target = factory.createTarget(path);
            CPPUNIT_ASSERT(target != nullptr);
            targets.push_back(std::move(target));
        }

        // Verify all were created
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(100), targets.size());

        // Verify they are all different instances (sample check)
        for (std::size_t i = 0; i < 10 && i < targets.size(); ++i)
        {
            for (std::size_t j = i + 1; j < 10 && j < targets.size(); ++j)
            {
                CPPUNIT_ASSERT(targets[i].get() != targets[j].get());
            }
        }
    }

    void testMultipleFactoriesCanExistSimultaneously()
    {
        DataTargetFactory factory1;
        DataTargetFactory factory2;
        DataTargetFactory factory3;

        // All factories should work independently
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(5), factory1.getTypeCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(5), factory2.getTypeCount());
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(5), factory3.getTypeCount());

        // Each factory should be able to create targets
        std::unique_ptr<DataTarget> target1 = factory1.createTarget("file:/path1");
        std::unique_ptr<DataTarget> target2 = factory2.createTarget("file:/path2");
        std::unique_ptr<DataTarget> target3 = factory3.createTarget("file:/path3");

        CPPUNIT_ASSERT(target1 != nullptr);
        CPPUNIT_ASSERT(target2 != nullptr);
        CPPUNIT_ASSERT(target3 != nullptr);

        // All targets should be different instances
        CPPUNIT_ASSERT(target1.get() != target2.get());
        CPPUNIT_ASSERT(target2.get() != target3.get());
        CPPUNIT_ASSERT(target1.get() != target3.get());
    }

    void testFactoryEntryPrefixIsImmutable()
    {
        DataTargetFactory factory;
        const DataTargetFactoryEntry& entry = factory.getTypeInfo(0);

        std::string prefix1 = entry.getPrefix();
        std::string prefix2 = entry.getPrefix();
        std::string prefix3 = entry.getPrefix();

        CPPUNIT_ASSERT_EQUAL(prefix1, prefix2);
        CPPUNIT_ASSERT_EQUAL(prefix2, prefix3);
        CPPUNIT_ASSERT_EQUAL(std::string("ipv4"), prefix1);
    }

    void testFactoryEntryDescriptionIsImmutable()
    {
        DataTargetFactory factory;
        const DataTargetFactoryEntry& entry = factory.getTypeInfo(1);

        std::string desc1 = entry.getDescription();
        std::string desc2 = entry.getDescription();

        CPPUNIT_ASSERT_EQUAL(desc1, desc2);
        CPPUNIT_ASSERT(!desc1.empty());
    }

    void testCreateTargetWithPathContainingNewline()
    {
        DataTargetFactory factory;
        std::string pathWithNewline = "file:/path\nwith\nnewlines";
        std::unique_ptr<DataTarget> target = factory.createTarget(pathWithNewline);
        // Should still create target as newlines are in the path data portion
        CPPUNIT_ASSERT(target != nullptr);
    }

    void testCreateTargetWithPathContainingTabs()
    {
        DataTargetFactory factory;
        std::string pathWithTabs = "file:/path\twith\ttabs";
        std::unique_ptr<DataTarget> target = factory.createTarget(pathWithTabs);
        // Should still create target as tabs are in the path data portion
        CPPUNIT_ASSERT(target != nullptr);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION( DataTargetFactoryTest );
