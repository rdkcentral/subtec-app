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
#include <map>
#include <memory>
#include <algorithm>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <LineBuilder.hpp>
#include <WebVTTDocument.hpp>
#include <WebVTTCue.hpp>
#include <Constants.hpp>

using namespace subttxrend::webvttengine;
using namespace subttxrend::webvttengine::linebuilder;

class LineBuilderTest : public CppUnit::TestFixture {

CPPUNIT_TEST_SUITE(LineBuilderTest);
CPPUNIT_TEST(TestLineBuilder);
CPPUNIT_TEST(TestFontSizes);
CPPUNIT_TEST(TestLineBreaking);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {

    }

    void tearDown()
    {

    }

    /**
     * @brief Build output lines test helper function.
     *
     * @param builder
     *      LineBuilder instance.
     * @param stream
     *      WebVTT data stream.
     * @return
     *      Line list.
    */
    std::list<Line> buildOutputLines(LineBuilder &builder, std::istringstream &stream)
    {
        CueList cueList;
        CueSharedList sh_list;
        WebVTTDocument documentParser;

        std::tie(cueList, std::ignore) = documentParser.parseCueList(stream, 0);
        std::for_each(cueList.begin(), cueList.end(), [&sh_list](CuePtr &unique_ptr) {
            sh_list.emplace_back(static_cast<CueSharedPtr>(std::move(unique_ptr)));
        });
        return builder.buildOutputLines(sh_list, {{}});
    }

    /**
     * @brief Render output lines into a string for testing.
     *
     * @param output
     *      Output lines.
     * @return
     *      Test string.
     */
    std::string renderOutputLines(std::list<Line> &output)
    {
        std::string rendered;

        for (auto &line : output)
        {
            bool firstInLine = true;
            for (auto &token : line.tokenVector)
            {
                if (firstInLine)
                {
                    firstInLine = false;
                }
                else
                {
                    /* Separate tokens with a space. */
                    rendered.append(" ");
                }

                rendered.append(token.token->mStr);
            }

            /* End lines with a newline. */
            rendered.append("\n");
        }

        return rendered;
    }

    void TestLineBuilder()
    {
std::istringstream origin_top_left(
            R"(WEBVTT
X-TIMESTAMP-MAP=MPEGTS:00000,LOCAL:00:00:00

00:01:31.000 --> 00:01:33.000 align:start line:0%
English subtitle 16 -Unforced- (00:01:31.000)
align:start line:0

)");
        Converter converter{1920, 1080};
        LineBuilder builder{1920, 1080};
        auto line_height_pixels = converter.lineHeightPixels();
        auto output = buildOutputLines(builder, origin_top_left);

        CPPUNIT_ASSERT_EQUAL(std::size_t{2}, output.size());
        Line line_one = output.front();
        Line line_two = output.back();
        CPPUNIT_ASSERT_EQUAL(converter.screenPaddingWidthPixels(), line_one.lineRectangle.m_x);
        CPPUNIT_ASSERT_EQUAL(converter.screenPaddingHeightPixels(), line_one.lineRectangle.m_y);
        
std::istringstream auto_line_align_left(
            R"(WEBVTT
X-TIMESTAMP-MAP=MPEGTS:00000,LOCAL:00:00:00

00:01:31.000 --> 00:01:33.000 align:start
English subtitle 16 -Unforced- (00:01:31.000)
align:start line:0

)");
        builder = LineBuilder(1920, 1080);
        output = buildOutputLines(builder, auto_line_align_left);

        CPPUNIT_ASSERT_EQUAL(std::size_t{2}, output.size());
        line_one = output.front();
        line_two = output.back();
        CPPUNIT_ASSERT_EQUAL(converter.screenPaddingWidthPixels(), line_one.lineRectangle.m_x);

std::istringstream auto_line_align_center(
            R"(WEBVTT
X-TIMESTAMP-MAP=MPEGTS:00000,LOCAL:00:00:00

00:01:31.000 --> 00:01:33.000
1234567890
default settings
blahdeblah

)");
        builder = LineBuilder(1920, 1080);
        output = buildOutputLines(builder, auto_line_align_center);

        CPPUNIT_ASSERT_EQUAL(std::size_t{3}, output.size());
        line_one = output.front();
        line_two = output.back();
        // CPPUNIT_ASSERT_EQUAL((1920 / 2), line_one.lineRectangle.m_x);
        // CPPUNIT_ASSERT_EQUAL(1080 - (line_height_pixels * 3), line_one.lineRectangle.m_y);

std::istringstream auto_empty(
            R"(WEBVTT
X-TIMESTAMP-MAP=MPEGTS:00000,LOCAL:00:00:00

00:01:31.000 --> 00:01:33.000

)");
        builder = LineBuilder(1920, 1080);
        output = buildOutputLines(builder, auto_empty);

        CPPUNIT_ASSERT_EQUAL(std::size_t{0}, output.size());

std::istringstream origin_top_left_empty(
            R"(WEBVTT
X-TIMESTAMP-MAP=MPEGTS:00000,LOCAL:00:00:00

00:01:31.000 --> 00:01:33.000 align:start line:0%

)");
        builder = LineBuilder(1920, 1080);
        output = buildOutputLines(builder, origin_top_left_empty);

        CPPUNIT_ASSERT_EQUAL(std::size_t{0}, output.size());
    }

    void TestFontSizes()
    {
        // Test with different font sizes.
        const WebVTTAttributes::FontSize fontSizes[] =
        {
            WebVTTAttributes::FontSize::SMALL,
            WebVTTAttributes::FontSize::MEDIUM,
            WebVTTAttributes::FontSize::LARGE,
            WebVTTAttributes::FontSize::EXTRA_LARGE
        };

        for (const auto fontSize : fontSizes)
        {
            WebVTTConfig config;
            WebVTTAttributes attributes;
            attributes.setInteger(WebVTTAttributes::AttributeType::FONT_SIZE, static_cast<uint32_t>(fontSize));
            LineBuilder builder{1920, 1080, config, attributes};
            Converter converter{1920, 1080, config, attributes};
            const auto viewportHeight = converter.height();
            const auto padding = converter.screenPaddingHeightPixels();
            const auto topPosition = converter.topPositioningPixels();
            const auto lineHeight = converter.lineHeightPixels();

            // Parse the WebVTT data.
std::istringstream auto_line(
            R"(WEBVTT

00:01:31.000 --> 00:01:33.000
1234567890

)");
            auto output = buildOutputLines(builder, auto_line);

            CPPUNIT_ASSERT_EQUAL(std::size_t{1}, output.size());

            // Verify that the line position (including padding) is as expected.
            Line line = output.front();
            auto expectedTopPosition = topPosition + padding;
            CPPUNIT_ASSERT_EQUAL(expectedTopPosition, line.lineRectangle.m_y);

            // Verify the line height.
            CPPUNIT_ASSERT_EQUAL(lineHeight, line.lineRectangle.m_h);

            // Parse the WebVTT data where the bottom line is specified.
std::istringstream bottom_line(
            R"(WEBVTT

00:01:31.000 --> 00:01:33.000 line:-1
1234567890

)");
            builder = LineBuilder{1920, 1080, config, attributes};
            output = buildOutputLines(builder, bottom_line);

            CPPUNIT_ASSERT_EQUAL(std::size_t{1}, output.size());

            // Verify that the line position (including padding) is as expected i.e.
            // the topPositioning value is ignored as the line is specified.
            line = output.front();
            expectedTopPosition = (viewportHeight - lineHeight) + padding;
            CPPUNIT_ASSERT_EQUAL(expectedTopPosition, line.lineRectangle.m_y);

            // Verify the line height.
            CPPUNIT_ASSERT_EQUAL(lineHeight, line.lineRectangle.m_h);

            // Parse the WebVTT data where the cue is empty.
std::istringstream auto_empty(
            R"(WEBVTT

00:01:31.000 --> 00:01:33.000

)");
            builder = LineBuilder{1920, 1080, config, attributes};
            output = buildOutputLines(builder, auto_empty);

            CPPUNIT_ASSERT_EQUAL(std::size_t{0}, output.size());
        }
    }

    /**
     * @brief Test line breaking
     */
    void TestLineBreaking()
    {
        static struct
        {
            std::string webvtt_data;
            std::string expected_rendering;
        } test_data[] =
        {
            /* Simple test case. */
            {
                R"(WEBVTT

00:01:31.000 --> 00:01:33.000
1234567890

)",
                "1234567890\n"
            },

            /* Multiple lines. */
            {
                R"(WEBVTT

00:01:31.000 --> 00:01:33.000
1234567890
12345 67890

)",
                "1234567890\n12345 67890\n"
            },

            /* Line break due to reduced width.
             *
             * The mock prerendered font characters are 20 pixels wide so 9 can
             * fit in a box of width 10% of 1920 pixels. The cue line will be
             * broken at the space.
             */
            {
                R"(WEBVTT

00:01:00.000 --> 00:01:10.000 size:10%
12345 67890

)",
                "12345\n67890\n"
            },

            /* Overflow due to reduced width.
             *
             * The mock prerendered font characters are 20 pixels wide so 9 can
             * fit in a box of width 10% of 1920 pixels. The overflowing cue
             * line will not be broken or moved onto a new line.
             */
            {
                R"(WEBVTT

00:01:00.000 --> 00:01:10.000 size:10%
1234567890

)",
                "1234567890\n"
            }
        };

        for (const auto &test_case : test_data)
        {
            LineBuilder builder{1920, 1080};
            std::istringstream stream(test_case.webvtt_data);
            auto output = buildOutputLines(builder, stream);
            std::string rendered = renderOutputLines(output);
            CPPUNIT_ASSERT_EQUAL(test_case.expected_rendering, rendered);
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(LineBuilderTest);
