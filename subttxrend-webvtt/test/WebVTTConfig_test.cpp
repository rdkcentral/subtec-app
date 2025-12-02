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

/*
 * Unit tests for WebVTTConfig and getConfigFractional using stub ConfigProvider
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <WebVTTConfig.hpp>
#include <subttxrend/common/ConfigProvider.hpp>

using namespace subttxrend::webvttengine;

class WebVTTConfigTest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(WebVTTConfigTest);
    CPPUNIT_TEST(testGetConfigFractionalReturnsDefault);
    CPPUNIT_TEST(testInitUsesDefaults);
    CPPUNIT_TEST(testGetConfigFractionalWithDifferentFractions);
    CPPUNIT_TEST(testConfigFieldsInitialization);
    CPPUNIT_TEST_SUITE_END();
public:
    void testGetConfigFractionalReturnsDefault() {
        subttxrend::common::ConfigProvider stub;
        // Always returns default, so should get defaultValue
        double result = getConfigFractional(&stub, "ANY.KEY", 42, 10);
        CPPUNIT_ASSERT_EQUAL(42.0, result);
    }
    void testInitUsesDefaults() {
        subttxrend::common::ConfigProvider stub;
        WebVTTConfig config;
        config.init(&stub);

        // Verify all fields are set to their expected default values
        CPPUNIT_ASSERT_EQUAL(std::string("Cinecav Sans"), config.fontFamily);
        CPPUNIT_ASSERT_EQUAL(std::string("WHITE"), config.textColour);
        CPPUNIT_ASSERT_EQUAL(std::string("BLACK"), config.bgColour);

        // Verify numeric defaults are reasonable (non-zero positive values)
        CPPUNIT_ASSERT(config.fontHeightH > 0);
        CPPUNIT_ASSERT(config.lineHeightH > 0);
        CPPUNIT_ASSERT(config.screenPadding >= 0);
        CPPUNIT_ASSERT(config.horizontalPaddingEm > 0);
        CPPUNIT_ASSERT(config.verticalPaddingEm > 0);
    }

    void testGetConfigFractionalWithDifferentFractions() {
        subttxrend::common::ConfigProvider stub;

        // Test with different fraction values
        double result1 = getConfigFractional(&stub, "TEST.KEY", 50, 100);
        CPPUNIT_ASSERT_EQUAL(50.0, result1);

        double result2 = getConfigFractional(&stub, "TEST.KEY", 25, 10);
        CPPUNIT_ASSERT_EQUAL(25.0, result2);

        double result3 = getConfigFractional(&stub, "TEST.KEY", 0, 1000);
        CPPUNIT_ASSERT_EQUAL(0.0, result3);
    }

    void testConfigFieldsInitialization() {
        subttxrend::common::ConfigProvider stub;

        // Test that WebVTTConfig properly initializes all fields
        WebVTTConfig config1;
        WebVTTConfig config2;

        // Before init, should have default values
        CPPUNIT_ASSERT_EQUAL(std::string("Cinecav Sans"), config1.fontFamily);
        CPPUNIT_ASSERT_EQUAL(std::string("WHITE"), config1.textColour);
        CPPUNIT_ASSERT_EQUAL(std::string("BLACK"), config1.bgColour);

        // After init with stub provider, should still have default values
        config1.init(&stub);
        config2.init(&stub);

        // Both configs should be identical
        CPPUNIT_ASSERT_EQUAL(config1.fontHeightH, config2.fontHeightH);
        CPPUNIT_ASSERT_EQUAL(config1.lineHeightH, config2.lineHeightH);
        CPPUNIT_ASSERT_EQUAL(config1.fontFamily, config2.fontFamily);
        CPPUNIT_ASSERT_EQUAL(config1.textColour, config2.textColour);
        CPPUNIT_ASSERT_EQUAL(config1.bgColour, config2.bgColour);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(WebVTTConfigTest);
