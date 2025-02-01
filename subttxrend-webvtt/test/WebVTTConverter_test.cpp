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
#include <sstream>
#include <iostream>
#include <cmath>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <WebVTTConverter.hpp>

using namespace subttxrend::webvttengine;

#define INT32(a) static_cast<std::int32_t>(a)

class WebVTTConverterTest : public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(WebVTTConverterTest);
CPPUNIT_TEST(testConverter);
CPPUNIT_TEST(testConverterAttributes);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {

    }

    void tearDown()
    {

    }

    /**
     * @brief Get an expected pixel height.
     *
     * The returned pixel height is scaled based on the actual height of the
     * display viewport.
     *
     * @param pixels
     *      Pixel height at a screen height of 1080 pixels.
     * @param height
     *      Actual display viewport height in pixels.
     * @return
     *      The expected height in pixels on the display viewport.
     */
    int pixelsToHeightPixels(int pixels, int height)
    {
        int vh = (int)(((((float)pixels)*10000.0f) / 1080.0f) + 0.5f);
        return (int)(((float)(vh * height)) / 10000.0f);
    }

    /**
     * @brief Get the expected line height in pixels.
     *
     * The line height is 9/8 times the font size. The returned height is scaled
     * based on the actual height of the display viewport.
     *
     * @param pixels
     *      Font size in pixels at a screen height of 1080 pixels.
     * @param height
     *      Actual display viewport height in pixels.
     * @return
     *      The expected line height in pixels on the display viewport.
     */
    int pixelsToLineHeightPixels(int pixels, int height)
    {
        int vh = pixelsToLineHeightVh(pixels);
        return (int)(((float)(vh * height)) / 10000.0f);
    }

    /**
     * @brief Get the expected line height in hundredths of one percent of the
     *        display viewport height.
     *
     * The line height is 9/8 times the font size.
     *
     * @param pixels
     *      Font size in pixels at a screen height of 1080 pixels.
     * @return
     *      The expected line height in hundredths of one percent of the display
     *      viewport height.
     */
    int pixelsToLineHeightVh(int pixels)
    {
        return (int)(((((float)pixels)*10000.0f*9.0f) / (1080.0f*8.0f)) + 0.5f);
    }

    void testConverter()
    {

        const int orig_width = 1920, orig_height = 1080;
        Converter converter {orig_width, orig_height};
        const int width = converter.width(), height = converter.height();

        CPPUNIT_ASSERT_EQUAL(width / 2, converter.vwToWidthPixels(5000));
        CPPUNIT_ASSERT_EQUAL(height / 5, converter.vhToHeightPixels(2000));

        // Center align, size 80%, position 50%, text length 20%
        //                  40                60
        //        10        |------------------|        90
        //        |-------------------------------------|
        // |--------------------------|-------------------------|
        //
        CPPUNIT_ASSERT_EQUAL((int)std::round(width * 0.4), converter.getXForTextBox(width * 0.2, WebVTTCue::AlignType::kCenter, width * 0.5));

        // Left align, size 50%, text length 20%
        //       20
        // |-----|
        // |--------------------------|
        // |--------------------------|-------------------------|
        //
        CPPUNIT_ASSERT_EQUAL(0, converter.getXForTextBox(width / 5, WebVTTCue::AlignType::kLeft, 0));

        // Right align, size 50%, text length 20%
        //                                                80
        //                                                |-----|
        //                            |-------------------------|
        // |--------------------------|-------------------------|
        //
        CPPUNIT_ASSERT_EQUAL((int)std::round(width * 0.8), converter.getXForTextBox(width / 5, WebVTTCue::AlignType::kRight, width));
    }

    /**
     * @brief Test the Converter when WebVTT attributes are set.
     */
    void testConverterAttributes()
    {
        WebVTTAttributes attributes;
        const int orig_width = 1920;
        const int orig_height = 1080;
        Converter converter(orig_width, orig_height);
        const int defaultFontSizePixels = converter.fontSizePixels();
        const int defaultLineHeightPixels = converter.lineHeightPixels();
        const int defaultLineHeightVh = converter.lineHeightVh();
        const int height = converter.height();

        // Small font size - 34px at 1080px height, top positioning 870px at 1080px height.
        attributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, static_cast<uint32_t>(WebVTTAttributes::FontSize::SMALL));
        converter.setAttributes(attributes);
        CPPUNIT_ASSERT_EQUAL(pixelsToLineHeightVh(34), converter.lineHeightVh());
        CPPUNIT_ASSERT_EQUAL(pixelsToLineHeightPixels(34, height), converter.lineHeightPixels());
        CPPUNIT_ASSERT_EQUAL(pixelsToHeightPixels(34, height), converter.fontSizePixels());
        CPPUNIT_ASSERT_EQUAL(true, converter.isTopPositioningSet());
        CPPUNIT_ASSERT_EQUAL(pixelsToHeightPixels(870, height), converter.topPositioningPixels());

        // Medium font size - 50px at 1080px height, top positioning 860px at 1080px height.
        attributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, static_cast<uint32_t>(WebVTTAttributes::FontSize::MEDIUM));
        converter.setAttributes(attributes);
        CPPUNIT_ASSERT_EQUAL(pixelsToLineHeightVh(50), converter.lineHeightVh());
        CPPUNIT_ASSERT_EQUAL(pixelsToLineHeightPixels(50, height), converter.lineHeightPixels());
        CPPUNIT_ASSERT_EQUAL(pixelsToHeightPixels(50, height), converter.fontSizePixels());
        CPPUNIT_ASSERT_EQUAL(true, converter.isTopPositioningSet());
        CPPUNIT_ASSERT_EQUAL(pixelsToHeightPixels(860, height), converter.topPositioningPixels());

        // Large font size - 80px at 1080px height, top positioning 790px at 1080px height.
        attributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, static_cast<uint32_t>(WebVTTAttributes::FontSize::LARGE));
        converter.setAttributes(attributes);
        CPPUNIT_ASSERT_EQUAL(pixelsToLineHeightVh(80), converter.lineHeightVh());
        CPPUNIT_ASSERT_EQUAL(pixelsToLineHeightPixels(80, height), converter.lineHeightPixels());
        CPPUNIT_ASSERT_EQUAL(pixelsToHeightPixels(80, height), converter.fontSizePixels());
        CPPUNIT_ASSERT_EQUAL(true, converter.isTopPositioningSet());
        CPPUNIT_ASSERT_EQUAL(pixelsToHeightPixels(790, height), converter.topPositioningPixels());

        // Extra large font size - 100px at 1080px height, top positioning at 770px at 1080px height.
        attributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, static_cast<uint32_t>(WebVTTAttributes::FontSize::EXTRA_LARGE));
        converter.setAttributes(attributes);
        CPPUNIT_ASSERT_EQUAL(pixelsToLineHeightVh(100), converter.lineHeightVh());
        CPPUNIT_ASSERT_EQUAL(pixelsToLineHeightPixels(100, height), converter.lineHeightPixels());
        CPPUNIT_ASSERT_EQUAL(pixelsToHeightPixels(100, height), converter.fontSizePixels());
        CPPUNIT_ASSERT_EQUAL(true, converter.isTopPositioningSet());
        CPPUNIT_ASSERT_EQUAL(pixelsToHeightPixels(770, height), converter.topPositioningPixels());

        // Embedded font size.
        attributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, static_cast<uint32_t>(-1));
        converter.setAttributes(attributes);
        CPPUNIT_ASSERT_EQUAL(defaultFontSizePixels, converter.fontSizePixels());
        CPPUNIT_ASSERT_EQUAL(defaultLineHeightPixels, converter.lineHeightPixels());
        CPPUNIT_ASSERT_EQUAL(defaultLineHeightVh, converter.lineHeightVh());
        CPPUNIT_ASSERT_EQUAL(false, converter.isTopPositioningSet());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(WebVTTConverterTest);
