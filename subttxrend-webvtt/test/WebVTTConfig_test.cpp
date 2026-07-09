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
    CPPUNIT_TEST(testDefaultFieldInitialization);
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
        config.fontHeightH = 1;
        config.lineHeightH = 2;
        config.screenPadding = 3;
        config.horizontalPaddingEm = 4.0f;
        config.verticalPaddingEm = 5.0f;
        config.fontFamily = "Before";
        config.textColour = "Before";
        config.bgColour = "Before";
        config.init(&stub);

        CPPUNIT_ASSERT_EQUAL(constants::kDefaultFontHeight, config.fontHeightH);
        CPPUNIT_ASSERT_EQUAL(constants::kDefaultLineHeight, config.lineHeightH);
        CPPUNIT_ASSERT_EQUAL(constants::kScreenPaddingVmH, config.screenPadding);
        CPPUNIT_ASSERT_EQUAL(static_cast<float>(constants::kDefaultHorizPaddingEmTenths), config.horizontalPaddingEm);
        CPPUNIT_ASSERT_EQUAL(static_cast<float>(constants::kDefaultVerticalPaddingEmTenths), config.verticalPaddingEm);
        CPPUNIT_ASSERT_EQUAL(std::string("Cinecav Sans"), config.fontFamily);
        CPPUNIT_ASSERT_EQUAL(std::string("WHITE"), config.textColour);
        CPPUNIT_ASSERT_EQUAL(std::string("BLACK"), config.bgColour);
    }

    void testDefaultFieldInitialization() {
        WebVTTConfig config;

        CPPUNIT_ASSERT_EQUAL(constants::kDefaultFontHeight, config.fontHeightH);
        CPPUNIT_ASSERT_EQUAL(constants::kDefaultLineHeight, config.lineHeightH);
        CPPUNIT_ASSERT_EQUAL(constants::kScreenPaddingVmH, config.screenPadding);
        CPPUNIT_ASSERT_EQUAL(static_cast<float>(constants::kDefaultHorizPaddingEmTenths), config.horizontalPaddingEm);
        CPPUNIT_ASSERT_EQUAL(static_cast<float>(constants::kDefaultVerticalPaddingEmTenths), config.verticalPaddingEm);
        CPPUNIT_ASSERT_EQUAL(std::string("Cinecav Sans"), config.fontFamily);
        CPPUNIT_ASSERT_EQUAL(std::string("WHITE"), config.textColour);
        CPPUNIT_ASSERT_EQUAL(std::string("BLACK"), config.bgColour);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(WebVTTConfigTest);
