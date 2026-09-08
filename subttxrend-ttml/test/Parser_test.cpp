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

#include <cstdint>
#include <list>
#include <string>
#include <tuple>
#include <vector>

#include <subttxrend/gfx/ColorArgb.hpp>

#include "Parser/Parser.hpp"

using namespace subttxrend::ttmlengine;

class ParserTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(ParserTest);
    CPPUNIT_TEST(testParseSimpleText);
    CPPUNIT_TEST(testParseImageDocument);
    CPPUNIT_TEST(testParseEmptyDocument);
    CPPUNIT_TEST(testParseWithoutTtRoot);
    CPPUNIT_TEST(testParseSortsTimeline);
    CPPUNIT_TEST(testParseDoesNotLeakState);
    CPPUNIT_TEST(testResetClearsPartialState);
    CPPUNIT_TEST(testApplyStyleOverride);
    CPPUNIT_TEST(testStyleOverrideWins);
    CPPUNIT_TEST(testStyleOverridePersistsReset);
    CPPUNIT_TEST(testEmptyStyleOverrideClearsPrevious);
    CPPUNIT_TEST(testIgnoreUnsupportedMetadata);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
    }

    void tearDown() override
    {
    }

protected:
    void testParseSimpleText()
    {
        Parser parser;

        const auto timeline = parseDocument(parser,
            makeTextDocument("00:00:01.000",
                             "00:00:03.000",
                             "hello parser",
                             "tts:fontFamily=\"monospace\" tts:textAlign=\"right\""));

        const auto& doc = assertSingleDocument(timeline);
        CPPUNIT_ASSERT(doc.m_timing.getStartTimeRef() == TimePoint(1000));
        CPPUNIT_ASSERT(doc.m_timing.getEndTimeRef() == TimePoint(3000));

        const auto& entity = assertSingleEntity(doc);
        CPPUNIT_ASSERT(entity.m_imageChunk.m_image == nullptr);
        const auto& chunk = assertSingleTextChunk(entity);
        CPPUNIT_ASSERT_EQUAL(std::string("hello parser"), chunk.m_text);
        CPPUNIT_ASSERT_EQUAL(std::string("monospace"), chunk.m_style.getFontFamily());
        CPPUNIT_ASSERT(chunk.m_style.getTextAlign() == StyleSet::TextAlign::RIGHT);
    }

    void testParseImageDocument()
    {
        Parser parser;

        const std::string imageId{"img1"};
        const std::string imageData{"YWJjMTIz"};
        const auto timeline = parseDocument(parser,
            makeImageDocument("00:00:00.000",
                              "00:00:02.000",
                              imageId,
                              imageData,
                              "tts:origin=\"10% 20%\" tts:extent=\"30% 40%\""));

        const auto& doc = assertSingleDocument(timeline);
        CPPUNIT_ASSERT(doc.m_timing.getStartTimeRef() == TimePoint(0));
        CPPUNIT_ASSERT(doc.m_timing.getEndTimeRef() == TimePoint(2000));

        const auto& entity = assertSingleEntity(doc);
        CPPUNIT_ASSERT(entity.m_imageChunk.m_image != nullptr);
        CPPUNIT_ASSERT_EQUAL(imageId, entity.m_imageChunk.m_image->getId());
        CPPUNIT_ASSERT_EQUAL(imageData, *entity.m_imageChunk.m_image->getBase64Data());
        CPPUNIT_ASSERT(entity.m_textLines.empty());
        CPPUNIT_ASSERT(entity.m_region != nullptr);
        CPPUNIT_ASSERT_EQUAL(1000, entity.m_region->getX().getValue());
        CPPUNIT_ASSERT_EQUAL(2000, entity.m_region->getY().getValue());
        CPPUNIT_ASSERT_EQUAL(3000, entity.m_region->getWidth().getValue());
        CPPUNIT_ASSERT_EQUAL(4000, entity.m_region->getHeight().getValue());
    }

    void testParseEmptyDocument()
    {
        Parser parser;

        const auto timeline = parseDocument(parser, makeEmptyDocument());

        CPPUNIT_ASSERT(timeline.empty());
    }

    void testParseWithoutTtRoot()
    {
        Parser parser;

        const auto timeline = parseDocument(parser,
            "<?xml version=\"1.0\"?>\n"
            "<html>\n"
            "  <body>\n"
            "    <p begin=\"00:00:00.000\" end=\"00:00:01.000\">ignored</p>\n"
            "  </body>\n"
            "</html>\n");

        CPPUNIT_ASSERT(timeline.empty());
    }

    void testParseSortsTimeline()
    {
        Parser parser;

        const auto timeline = parseDocument(parser,
            makeMultiCueDocument(
                "<p begin=\"00:00:05.000\" end=\"00:00:06.000\">late</p>\n"
                "        <p begin=\"00:00:01.000\" end=\"00:00:02.000\">early</p>\n"));

        CPPUNIT_ASSERT_EQUAL(std::size_t(2), timeline.size());

        auto it = timeline.begin();
        CPPUNIT_ASSERT(it->m_timing.getStartTimeRef() == TimePoint(1000));
        CPPUNIT_ASSERT(it->m_timing.getEndTimeRef() == TimePoint(2000));
        CPPUNIT_ASSERT_EQUAL(std::string("early"), firstTextChunk(*it).m_text);

        ++it;
        CPPUNIT_ASSERT(it != timeline.end());
        CPPUNIT_ASSERT(it->m_timing.getStartTimeRef() == TimePoint(5000));
        CPPUNIT_ASSERT(it->m_timing.getEndTimeRef() == TimePoint(6000));
        CPPUNIT_ASSERT_EQUAL(std::string("late"), firstTextChunk(*it).m_text);
    }

    void testParseDoesNotLeakState()
    {
        Parser parser;

        const auto firstTimeline = parseDocument(parser,
            makeTextDocument("00:00:00.000", "00:00:01.000", "first cue"));
        const auto secondTimeline = parseDocument(parser,
            makeTextDocument("00:00:02.000", "00:00:03.000", "second cue"));

        CPPUNIT_ASSERT_EQUAL(std::size_t(1), firstTimeline.size());
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), secondTimeline.size());
        CPPUNIT_ASSERT_EQUAL(std::string("first cue"), firstTextChunk(firstTimeline.front()).m_text);
        CPPUNIT_ASSERT_EQUAL(std::string("second cue"), firstTextChunk(secondTimeline.front()).m_text);
        CPPUNIT_ASSERT(secondTimeline.front().m_timing.getStartTimeRef() == TimePoint(2000));
    }

    void testResetClearsPartialState()
    {
        Parser parser;

        parseDocument(parser,
            "<?xml version=\"1.0\"?>\n"
            "<tt xmlns=\"http://www.w3.org/ns/ttml\">\n"
            "  <body><div><p begin=\"00:00:00.000\" end=\"00:00:01.000\">broken");

        parser.reset();

        const auto recoveredTimeline = parseDocument(parser,
            makeTextDocument("00:00:04.000", "00:00:05.000", "after reset"));

        const auto& doc = assertSingleDocument(recoveredTimeline);
        CPPUNIT_ASSERT(doc.m_timing.getStartTimeRef() == TimePoint(4000));
        CPPUNIT_ASSERT_EQUAL(std::string("after reset"), assertSingleTextChunk(assertSingleEntity(doc)).m_text);
    }

    void testApplyStyleOverride()
    {
        Parser parser;

        parser.setStyleOverrideAttributes({{"color", "blue"}, {"fontFamily", "OverrideFont"}});

        const auto timeline = parseDocument(parser,
            makeTextDocument("00:00:00.000", "00:00:01.000", "override me"));

        const auto& chunk = assertSingleTextChunk(assertSingleEntity(assertSingleDocument(timeline)));
        CPPUNIT_ASSERT(chunk.m_style.getColor() == subttxrend::gfx::ColorArgb::BLUE);
        CPPUNIT_ASSERT_EQUAL(std::string("OverrideFont"), chunk.m_style.getFontFamily());
    }

    void testStyleOverrideWins()
    {
        Parser parser;

        parser.setStyleOverrideAttributes({{"color", "blue"}});

        const auto timeline = parseDocument(parser,
            makeTextDocument("00:00:00.000",
                             "00:00:01.000",
                             "styled",
                             "tts:color=\"red\""));

        const auto& chunk = assertSingleTextChunk(assertSingleEntity(assertSingleDocument(timeline)));
        CPPUNIT_ASSERT(chunk.m_style.getColor() == subttxrend::gfx::ColorArgb::BLUE);
    }

    void testStyleOverridePersistsReset()
    {
        Parser parser;

        parser.setStyleOverrideAttributes({{"color", "red"}});
        parser.reset();

        const auto timeline = parseDocument(parser,
            makeTextDocument("00:00:00.000", "00:00:01.000", "still overridden"));

        const auto& chunk = assertSingleTextChunk(assertSingleEntity(assertSingleDocument(timeline)));
        CPPUNIT_ASSERT(chunk.m_style.getColor() == subttxrend::gfx::ColorArgb::RED);
    }

    void testEmptyStyleOverrideClearsPrevious()
    {
        Parser parser;

        parser.setStyleOverrideAttributes({{"color", "red"}});
        const auto overriddenTimeline = parseDocument(parser,
            makeTextDocument("00:00:00.000", "00:00:01.000", "red text"));
        CPPUNIT_ASSERT(assertSingleTextChunk(assertSingleEntity(assertSingleDocument(overriddenTimeline))).m_style.getColor()
                       == subttxrend::gfx::ColorArgb::RED);

        parser.setStyleOverrideAttributes({});

        const auto clearedTimeline = parseDocument(parser,
            makeTextDocument("00:00:02.000", "00:00:03.000", "default text"));

        const auto& chunk = assertSingleTextChunk(assertSingleEntity(assertSingleDocument(clearedTimeline)));
        CPPUNIT_ASSERT(chunk.m_style.getColor() == subttxrend::gfx::ColorArgb::WHITE);
    }

    void testIgnoreUnsupportedMetadata()
    {
        Parser parser;

        const auto timeline = parseDocument(parser,
            "<?xml version=\"1.0\"?>\n"
            "<tt xml:lang=\"en\" xmlns=\"http://www.w3.org/ns/ttml\" xmlns:tts=\"http://www.w3.org/ns/ttml#styling\" xmlns:ttm=\"http://www.w3.org/ns/ttml#metadata\" xmlns:ttp=\"http://www.w3.org/ns/ttml#parameter\">\n"
            "  <head>\n"
            "    <metadata>\n"
            "      <ttm:title>ignored title</ttm:title>\n"
            "      <extra:custom xmlns:extra=\"urn:test\">ignored payload</extra:custom>\n"
            "    </metadata>\n"
            "  </head>\n"
            "  <body>\n"
            "    <div>\n"
            "      <p begin=\"00:00:00.000\" end=\"00:00:01.000\">visible text</p>\n"
            "    </div>\n"
            "  </body>\n"
            "</tt>\n");

        const auto& doc = assertSingleDocument(timeline);
        CPPUNIT_ASSERT_EQUAL(std::string("visible text"), assertSingleTextChunk(assertSingleEntity(doc)).m_text);
    }

    std::list<IntermediateDocument> parseDocument(Parser& parser,
                                                  const std::string& document) const
    {
        const auto buffer = makeBuffer(document);
        return parser.parse(buffer.data(), buffer.size());
    }

    std::vector<std::uint8_t> makeBuffer(const std::string& document) const
    {
        return std::vector<std::uint8_t>(document.begin(), document.end());
    }

    std::string makeTextDocument(const std::string& begin,
                                 const std::string& end,
                                 const std::string& text,
                                 const std::string& spanAttributes = "") const
    {
        return makeMultiCueDocument(makeCue(begin, end, text, spanAttributes));
    }

    std::string makeMultiCueDocument(const std::string& bodyContent) const
    {
        return std::string("<?xml version=\"1.0\"?>\n")
            + "<tt xml:lang=\"en\" xmlns=\"http://www.w3.org/ns/ttml\" xmlns:tts=\"http://www.w3.org/ns/ttml#styling\" xmlns:ttp=\"http://www.w3.org/ns/ttml#parameter\" ttp:cellResolution=\"40 24\">\n"
            + "  <body>\n"
            + "    <div>\n"
            + bodyContent
            + "    </div>\n"
            + "  </body>\n"
            + "</tt>\n";
    }

    std::string makeCue(const std::string& begin,
                        const std::string& end,
                        const std::string& text,
                        const std::string& spanAttributes = "") const
    {
        std::string cue = "      <p begin=\"" + begin + "\" end=\"" + end + "\">";
        if (!spanAttributes.empty())
        {
            cue += "<span " + spanAttributes + ">" + text + "</span>";
        }
        else
        {
            cue += text;
        }
        cue += "</p>\n";
        return cue;
    }

    std::string makeImageDocument(const std::string& begin,
                                  const std::string& end,
                                  const std::string& imageId,
                                  const std::string& imageData,
                                  const std::string& regionAttributes) const
    {
        return std::string("<?xml version=\"1.0\"?>\n")
            + "<tt xml:lang=\"en\" xmlns=\"http://www.w3.org/ns/ttml\" xmlns:tts=\"http://www.w3.org/ns/ttml#styling\" xmlns:ttm=\"http://www.w3.org/ns/ttml#metadata\" xmlns:ttp=\"http://www.w3.org/ns/ttml#parameter\" ttp:cellResolution=\"40 24\">\n"
            + "  <head>\n"
            + "    <metadata>\n"
            + "      <image xml:id=\"" + imageId + "\">" + imageData + "</image>\n"
            + "    </metadata>\n"
            + "    <layout>\n"
            + "      <region xml:id=\"r1\" " + regionAttributes + " />\n"
            + "    </layout>\n"
            + "  </head>\n"
            + "  <body>\n"
            + "    <div>\n"
            + "      <p begin=\"" + begin + "\" end=\"" + end + "\" region=\"r1\" backgroundImage=\"#" + imageId + "\"></p>\n"
            + "    </div>\n"
            + "  </body>\n"
            + "</tt>\n";
    }

    std::string makeEmptyDocument() const
    {
        return std::string("<?xml version=\"1.0\"?>\n")
            + "<tt xml:lang=\"en\" xmlns=\"http://www.w3.org/ns/ttml\" xmlns:tts=\"http://www.w3.org/ns/ttml#styling\" xmlns:ttp=\"http://www.w3.org/ns/ttml#parameter\" ttp:cellResolution=\"40 24\">\n"
            + "  <body>\n"
            + "    <div/>\n"
            + "  </body>\n"
            + "</tt>\n";
    }

    const IntermediateDocument& assertSingleDocument(const std::list<IntermediateDocument>& timeline) const
    {
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), timeline.size());
        return timeline.front();
    }

    const IntermediateDocument::Entity& assertSingleEntity(const IntermediateDocument& doc) const
    {
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), doc.m_entites.size());
        return doc.m_entites.front();
    }

    const IntermediateDocument::TextChunk& assertSingleTextChunk(const IntermediateDocument::Entity& entity) const
    {
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), entity.m_textLines.size());
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), entity.m_textLines.front().size());
        return entity.m_textLines.front().front();
    }

    const IntermediateDocument::TextChunk& firstTextChunk(const IntermediateDocument& doc) const
    {
        return assertSingleTextChunk(assertSingleEntity(doc));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ParserTest);
