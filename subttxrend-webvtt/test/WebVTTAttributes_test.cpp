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

using namespace subttxrend::webvttengine;

class WebVTTAttributesTest : public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(WebVTTAttributesTest);
    CPPUNIT_TEST(testAttributes);
    CPPUNIT_TEST(testUpdateAttributes);
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
};

CPPUNIT_TEST_SUITE_REGISTRATION(WebVTTAttributesTest);
