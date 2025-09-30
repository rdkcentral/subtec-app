/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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
#include <cppunit/TestCase.h>
#include <WebVTTAttributes.hpp>

#include <thread>
#include <atomic>

using namespace subttxrend::webvttengine;

class WebVTTAttributesTest : public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(WebVTTAttributesTest);
    CPPUNIT_TEST(testAttributes);
    CPPUNIT_TEST(testUpdateAttributes);
    CPPUNIT_TEST(testInvalidAttributeType);
    CPPUNIT_TEST(testMultipleAttributes);
    CPPUNIT_TEST(testAllAttributeTypes);
    CPPUNIT_TEST(testDefaultValueForUnset);
    CPPUNIT_TEST(testEnhancedThreadSafety);
    CPPUNIT_TEST(testConcurrentDifferentAttributes);
    CPPUNIT_TEST(testBoundaryValues);
    CPPUNIT_TEST(testSystematicAttributeTypes);
    CPPUNIT_TEST(testInvalidAttributeTypeExtended);
    CPPUNIT_TEST(testUpdateOverlappingAttributes);
    CPPUNIT_TEST(testUpdateNonOverlappingAttributes);
    CPPUNIT_TEST(testPerformanceLargeNumberOfAttributes);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void testAttributes()
    {
        WebVTTAttributes attributes;
        constexpr uint32_t integerValue = 1;

        // Empty attributes.
        CPPUNIT_ASSERT_EQUAL(false, attributes.isSet(WebVTTAttributes::AttributeType::FONT_SIZE));

        // Set attribute.
        attributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, integerValue);
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::FONT_SIZE));
        CPPUNIT_ASSERT_EQUAL(integerValue, attributes.getInteger(WebVTTAttributes::AttributeType::FONT_SIZE));

        // Reset.
        attributes.reset();
        CPPUNIT_ASSERT_EQUAL(false, attributes.isSet(WebVTTAttributes::AttributeType::FONT_SIZE));
    }

    void testUpdateAttributes()
    {
        WebVTTAttributes oldAttributes;
        WebVTTAttributes newAttributes;
        constexpr uint32_t integerValue1 = 1;
        constexpr uint32_t integerValue2 = 2;

        // Update sets one attribute.
        newAttributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, integerValue1);
        oldAttributes.update(newAttributes);
        CPPUNIT_ASSERT_EQUAL(true, oldAttributes.isSet(WebVTTAttributes::AttributeType::FONT_SIZE));
        CPPUNIT_ASSERT_EQUAL(integerValue1, oldAttributes.getInteger(WebVTTAttributes::AttributeType::FONT_SIZE));

        // Empty update.
        newAttributes.reset();
        oldAttributes.update(newAttributes);
        CPPUNIT_ASSERT_EQUAL(true, oldAttributes.isSet(WebVTTAttributes::AttributeType::FONT_SIZE));
        CPPUNIT_ASSERT_EQUAL(integerValue1, oldAttributes.getInteger(WebVTTAttributes::AttributeType::FONT_SIZE));

        // Update sets an attribute.
        newAttributes.reset();
        newAttributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, integerValue2);
        oldAttributes.update(newAttributes);
        CPPUNIT_ASSERT_EQUAL(true, oldAttributes.isSet(WebVTTAttributes::AttributeType::FONT_SIZE));
        CPPUNIT_ASSERT_EQUAL(integerValue2, oldAttributes.getInteger(WebVTTAttributes::AttributeType::FONT_SIZE));
    }

    void testInvalidAttributeType()
    {
        WebVTTAttributes attributes;
        // Should not set or get LAST_VALUE
        attributes.setInteger(WebVTTAttributes::AttributeType::LAST_VALUE, 1234);
        CPPUNIT_ASSERT_EQUAL(false, attributes.isSet(WebVTTAttributes::AttributeType::LAST_VALUE));
        CPPUNIT_ASSERT_EQUAL(uint32_t(0), attributes.getInteger(WebVTTAttributes::AttributeType::LAST_VALUE));
    }

    void testMultipleAttributes()
    {
        WebVTTAttributes attributes;
        attributes.setInteger(WebVTTAttributes::AttributeType::FONT_COLOR, 0xFF0000);
        attributes.setInteger(WebVTTAttributes::AttributeType::BACKGROUND_COLOR, 0x00FF00);
        attributes.setInteger(WebVTTAttributes::AttributeType::EDGE_COLOR, 0x0000FF);

        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::FONT_COLOR));
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::EDGE_COLOR));
        CPPUNIT_ASSERT_EQUAL(uint32_t(0xFF0000), attributes.getInteger(WebVTTAttributes::AttributeType::FONT_COLOR));
        CPPUNIT_ASSERT_EQUAL(uint32_t(0x00FF00), attributes.getInteger(WebVTTAttributes::AttributeType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT_EQUAL(uint32_t(0x0000FF), attributes.getInteger(WebVTTAttributes::AttributeType::EDGE_COLOR));

        attributes.reset();
        CPPUNIT_ASSERT_EQUAL(false, attributes.isSet(WebVTTAttributes::AttributeType::FONT_COLOR));
        CPPUNIT_ASSERT_EQUAL(false, attributes.isSet(WebVTTAttributes::AttributeType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT_EQUAL(false, attributes.isSet(WebVTTAttributes::AttributeType::EDGE_COLOR));
    }

    void testAllAttributeTypes()
    {
        WebVTTAttributes attributes;
        for (int i = 0; i < static_cast<int>(WebVTTAttributes::AttributeType::LAST_VALUE); ++i) {
            auto type = static_cast<WebVTTAttributes::AttributeType>(i);
            attributes.setInteger(type, i * 10);
            CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(type));
            CPPUNIT_ASSERT_EQUAL(uint32_t(i * 10), attributes.getInteger(type));
        }
        attributes.reset();
        for (int i = 0; i < static_cast<int>(WebVTTAttributes::AttributeType::LAST_VALUE); ++i) {
            auto type = static_cast<WebVTTAttributes::AttributeType>(i);
            CPPUNIT_ASSERT_EQUAL(false, attributes.isSet(type));
        }
    }

    void testDefaultValueForUnset()
    {
        WebVTTAttributes attributes;
        CPPUNIT_ASSERT_EQUAL(uint32_t(0), attributes.getInteger(WebVTTAttributes::AttributeType::EDGE_COLOR));
    }


    void testEnhancedThreadSafety()
    {
        WebVTTAttributes attributes;
        std::atomic<bool> testComplete{false};
        std::atomic<uint32_t> lastReadValue{0};
        std::atomic<bool> inconsistentRead{false};

        auto writer = [&attributes, &testComplete]() {
            for (int i = 0; i < 1000; ++i) {
                attributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, i);
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
            testComplete = true;
        };

        auto reader = [&attributes, &testComplete, &lastReadValue, &inconsistentRead]() {
            uint32_t previousValue = 0;
            while (!testComplete) {
                uint32_t currentValue = attributes.getInteger(WebVTTAttributes::AttributeType::FONT_SIZE);
                lastReadValue = currentValue;
                // Value should either stay the same or increase (writer increments)
                if (currentValue < previousValue) {
                    inconsistentRead = true;
                }
                previousValue = currentValue;
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        };

        std::thread t1(writer), t2(reader);
        t1.join();
        t2.join();

        // Verify no inconsistent reads occurred
        CPPUNIT_ASSERT_EQUAL(false, inconsistentRead.load());
        // Verify final value is reasonable (should be close to 999)
        CPPUNIT_ASSERT(lastReadValue >= 990);
    }

    void testConcurrentDifferentAttributes()
    {
        WebVTTAttributes attributes;
        std::atomic<bool> testComplete{false};

        auto fontSizeWriter = [&attributes, &testComplete]() {
            for (int i = 0; i < 500; ++i) {
                attributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, i);
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
            testComplete = true;
        };

        auto fontColorWriter = [&attributes, &testComplete]() {
            for (int i = 0; i < 500; ++i) {
                attributes.setInteger(WebVTTAttributes::AttributeType::FONT_COLOR, 0xFF0000 + i);
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        };

        auto reader = [&attributes, &testComplete]() {
            while (!testComplete) {
                attributes.getInteger(WebVTTAttributes::AttributeType::FONT_SIZE);
                attributes.getInteger(WebVTTAttributes::AttributeType::FONT_COLOR);
                attributes.isSet(WebVTTAttributes::AttributeType::FONT_SIZE);
                attributes.isSet(WebVTTAttributes::AttributeType::FONT_COLOR);
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        };

        std::thread t1(fontSizeWriter), t2(fontColorWriter), t3(reader);
        t1.join();
        t2.join();
        t3.join();

        // Both attributes should be set at the end
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::FONT_SIZE));
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::FONT_COLOR));
        CPPUNIT_ASSERT(true); // Test completed without deadlock
    }

    void testBoundaryValues()
    {
        WebVTTAttributes attributes;

        // Test minimum value
        attributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, 0);
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::FONT_SIZE));
        CPPUNIT_ASSERT_EQUAL(uint32_t(0), attributes.getInteger(WebVTTAttributes::AttributeType::FONT_SIZE));

        // Test maximum uint32_t value
        constexpr uint32_t maxValue = std::numeric_limits<uint32_t>::max();
        attributes.setInteger(WebVTTAttributes::AttributeType::FONT_COLOR, maxValue);
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::FONT_COLOR));
        CPPUNIT_ASSERT_EQUAL(maxValue, attributes.getInteger(WebVTTAttributes::AttributeType::FONT_COLOR));

        // Test large values for different attributes
        attributes.setInteger(WebVTTAttributes::AttributeType::BACKGROUND_COLOR, 0xFFFFFFFF);
        attributes.setInteger(WebVTTAttributes::AttributeType::EDGE_COLOR, 0x12345678);
        
        CPPUNIT_ASSERT_EQUAL(uint32_t(0xFFFFFFFF), attributes.getInteger(WebVTTAttributes::AttributeType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT_EQUAL(uint32_t(0x12345678), attributes.getInteger(WebVTTAttributes::AttributeType::EDGE_COLOR));
    }

    void testSystematicAttributeTypes()
    {
        WebVTTAttributes attributes;

        // Test all specific attribute types individually
        attributes.setInteger(WebVTTAttributes::AttributeType::FONT_OPACITY, 128);
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::FONT_OPACITY));
        CPPUNIT_ASSERT_EQUAL(uint32_t(128), attributes.getInteger(WebVTTAttributes::AttributeType::FONT_OPACITY));

        attributes.setInteger(WebVTTAttributes::AttributeType::BACKGROUND_OPACITY, 64);
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::BACKGROUND_OPACITY));
        CPPUNIT_ASSERT_EQUAL(uint32_t(64), attributes.getInteger(WebVTTAttributes::AttributeType::BACKGROUND_OPACITY));

        attributes.setInteger(WebVTTAttributes::AttributeType::FONT_STYLE, 2);
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::FONT_STYLE));
        CPPUNIT_ASSERT_EQUAL(uint32_t(2), attributes.getInteger(WebVTTAttributes::AttributeType::FONT_STYLE));

        attributes.setInteger(WebVTTAttributes::AttributeType::WINDOW_COLOR, 0x800080);
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::WINDOW_COLOR));
        CPPUNIT_ASSERT_EQUAL(uint32_t(0x800080), attributes.getInteger(WebVTTAttributes::AttributeType::WINDOW_COLOR));

        attributes.setInteger(WebVTTAttributes::AttributeType::WINDOW_OPACITY, 192);
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::WINDOW_OPACITY));
        CPPUNIT_ASSERT_EQUAL(uint32_t(192), attributes.getInteger(WebVTTAttributes::AttributeType::WINDOW_OPACITY));

        attributes.setInteger(WebVTTAttributes::AttributeType::EDGE_STYLE, 3);
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::EDGE_STYLE));
        CPPUNIT_ASSERT_EQUAL(uint32_t(3), attributes.getInteger(WebVTTAttributes::AttributeType::EDGE_STYLE));

        // Verify all are still set
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::FONT_OPACITY));
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::BACKGROUND_OPACITY));
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::FONT_STYLE));
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::WINDOW_COLOR));
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::WINDOW_OPACITY));
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::EDGE_STYLE));
    }

    void testInvalidAttributeTypeExtended()
    {
        WebVTTAttributes attributes;

        // Test LAST_VALUE (already covered, but being thorough)
        attributes.setInteger(WebVTTAttributes::AttributeType::LAST_VALUE, 1234);
        CPPUNIT_ASSERT_EQUAL(false, attributes.isSet(WebVTTAttributes::AttributeType::LAST_VALUE));
        CPPUNIT_ASSERT_EQUAL(uint32_t(0), attributes.getInteger(WebVTTAttributes::AttributeType::LAST_VALUE));

        // Test casting larger enum values (potential future enum additions)
        auto invalidType = static_cast<WebVTTAttributes::AttributeType>(static_cast<int>(WebVTTAttributes::AttributeType::LAST_VALUE) + 1);
        attributes.setInteger(invalidType, 5678);
        CPPUNIT_ASSERT_EQUAL(false, attributes.isSet(invalidType));
        CPPUNIT_ASSERT_EQUAL(uint32_t(0), attributes.getInteger(invalidType));

        // Note: UNDEF (value 0) is actually a valid attribute type that can be set
        // since it's less than LAST_VALUE, so we test it as a valid type
        attributes.setInteger(WebVTTAttributes::AttributeType::UNDEF, 42);
        CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(WebVTTAttributes::AttributeType::UNDEF));
        CPPUNIT_ASSERT_EQUAL(uint32_t(42), attributes.getInteger(WebVTTAttributes::AttributeType::UNDEF));
    }

    void testUpdateOverlappingAttributes()
    {
        WebVTTAttributes oldAttributes;
        WebVTTAttributes newAttributes;

        // Set initial attributes
        oldAttributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, 10);
        oldAttributes.setInteger(WebVTTAttributes::AttributeType::FONT_COLOR, 0xFF0000);
        oldAttributes.setInteger(WebVTTAttributes::AttributeType::BACKGROUND_COLOR, 0x00FF00);

        // Set overlapping and new attributes in newAttributes
        newAttributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, 20); // Overlap - should update
        newAttributes.setInteger(WebVTTAttributes::AttributeType::FONT_COLOR, 0x0000FF); // Overlap - should update
        newAttributes.setInteger(WebVTTAttributes::AttributeType::EDGE_COLOR, 0xFFFF00); // New - should add

        // Update
        oldAttributes.update(newAttributes);

        // Verify overlapping attributes were updated
        CPPUNIT_ASSERT_EQUAL(uint32_t(20), oldAttributes.getInteger(WebVTTAttributes::AttributeType::FONT_SIZE));
        CPPUNIT_ASSERT_EQUAL(uint32_t(0x0000FF), oldAttributes.getInteger(WebVTTAttributes::AttributeType::FONT_COLOR));

        // Verify non-overlapping old attribute remains
        CPPUNIT_ASSERT_EQUAL(uint32_t(0x00FF00), oldAttributes.getInteger(WebVTTAttributes::AttributeType::BACKGROUND_COLOR));

        // Verify new attribute was added
        CPPUNIT_ASSERT_EQUAL(uint32_t(0xFFFF00), oldAttributes.getInteger(WebVTTAttributes::AttributeType::EDGE_COLOR));
        CPPUNIT_ASSERT_EQUAL(true, oldAttributes.isSet(WebVTTAttributes::AttributeType::EDGE_COLOR));
    }

    void testUpdateNonOverlappingAttributes()
    {
        WebVTTAttributes oldAttributes;
        WebVTTAttributes newAttributes;

        // Set completely different attributes
        oldAttributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, 15);
        oldAttributes.setInteger(WebVTTAttributes::AttributeType::FONT_COLOR, 0x123456);

        newAttributes.setInteger(WebVTTAttributes::AttributeType::BACKGROUND_COLOR, 0x654321);
        newAttributes.setInteger(WebVTTAttributes::AttributeType::EDGE_STYLE, 2);

        // Update
        oldAttributes.update(newAttributes);

        // Verify all attributes are preserved/added
        CPPUNIT_ASSERT_EQUAL(true, oldAttributes.isSet(WebVTTAttributes::AttributeType::FONT_SIZE));
        CPPUNIT_ASSERT_EQUAL(true, oldAttributes.isSet(WebVTTAttributes::AttributeType::FONT_COLOR));
        CPPUNIT_ASSERT_EQUAL(true, oldAttributes.isSet(WebVTTAttributes::AttributeType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT_EQUAL(true, oldAttributes.isSet(WebVTTAttributes::AttributeType::EDGE_STYLE));

        CPPUNIT_ASSERT_EQUAL(uint32_t(15), oldAttributes.getInteger(WebVTTAttributes::AttributeType::FONT_SIZE));
        CPPUNIT_ASSERT_EQUAL(uint32_t(0x123456), oldAttributes.getInteger(WebVTTAttributes::AttributeType::FONT_COLOR));
        CPPUNIT_ASSERT_EQUAL(uint32_t(0x654321), oldAttributes.getInteger(WebVTTAttributes::AttributeType::BACKGROUND_COLOR));
        CPPUNIT_ASSERT_EQUAL(uint32_t(2), oldAttributes.getInteger(WebVTTAttributes::AttributeType::EDGE_STYLE));
    }

    void testPerformanceLargeNumberOfAttributes()
    {
        WebVTTAttributes attributes;
        
        // Performance test: set many attributes rapidly
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int iteration = 0; iteration < 1000; ++iteration) {
            for (int i = 0; i < static_cast<int>(WebVTTAttributes::AttributeType::LAST_VALUE); ++i) {
                auto type = static_cast<WebVTTAttributes::AttributeType>(i);
                attributes.setInteger(type, iteration * 10 + i);
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Verify all attributes are set correctly after performance test
        for (int i = 0; i < static_cast<int>(WebVTTAttributes::AttributeType::LAST_VALUE); ++i) {
            auto type = static_cast<WebVTTAttributes::AttributeType>(i);
            CPPUNIT_ASSERT_EQUAL(true, attributes.isSet(type));
            CPPUNIT_ASSERT_EQUAL(uint32_t(999 * 10 + i), attributes.getInteger(type));
        }
        
        // Performance should complete in reasonable time (less than 5 seconds for this test)
        CPPUNIT_ASSERT(duration.count() < 5000);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(WebVTTAttributesTest);
