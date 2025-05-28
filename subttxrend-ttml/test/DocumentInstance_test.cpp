/*****************************************************************************
* If not stated otherwise in this file or this component's LICENSE file the
* following copyright and licenses apply:
*
* Copyright 2021 Liberty Global Service B.V.
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
*****************************************************************************/

#include <cppunit/extensions/HelperMacros.h>
#include "Parser/DocumentInstance.hpp"

using namespace subttxrend::ttmlengine;

class DocumentInstanceTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( DocumentInstanceTest );
    CPPUNIT_TEST(ignoreDocumentWithoutRootElement),
    CPPUNIT_TEST(ignoreDocumentWithoutTTRootElement),
    CPPUNIT_TEST(basicDocumentInstance);
    CPPUNIT_TEST(referredStyle);
    CPPUNIT_TEST(styleInheritanceFromParent);
    CPPUNIT_TEST(styleInheritanceRegionOverStyle);
    CPPUNIT_TEST(styleInheritanceRegionStyleOverStyle);
    CPPUNIT_TEST(styleInheritanceRegionOverRegionStyle);
    CPPUNIT_TEST(styleInheritanceElementOverRegion);
    CPPUNIT_TEST(regionGeometry);
    CPPUNIT_TEST(regionGeometryInheritanceFromBody);
    CPPUNIT_TEST(multiSpanRegionCase1);
    CPPUNIT_TEST(multiSpanRegionCase2);
    CPPUNIT_TEST(defaultWhitespaceHandling);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
        // noop
    }

    void tearDown()
    {
        // noop
    }

    void ignoreDocumentWithoutRootElement()
    {
        DocumentInstance doc{};

        CPPUNIT_ASSERT(doc.generateTimeline().empty());

        auto div = doc.startElement("div");
        div->parseAttribute("", "begin", "00:00:00");
        div->parseAttribute("", "end", "00:00:10");
        div->appendText("text");
        doc.endElement();

        auto timeline = doc.generateTimeline();

        CPPUNIT_ASSERT(timeline.empty());
    }

    void ignoreDocumentWithoutTTRootElement()
    {
        DocumentInstance doc{};

        CPPUNIT_ASSERT(doc.generateTimeline().empty());

        doc.startElement("html");
        auto div = doc.startElement("div");
        div->parseAttribute("", "begin", "00:00:00");
        div->parseAttribute("", "end", "00:00:10");
        div->appendText("text");
        doc.endElement();

        doc.endElement(); //html

        auto timeline = doc.generateTimeline();

        CPPUNIT_ASSERT(timeline.empty());
    }

    void basicDocumentInstance()
    {
        DocumentInstance doc{};

        CPPUNIT_ASSERT(doc.generateTimeline().empty());

        doc.startElement("tt");
        auto div = doc.startElement("div");
        div->parseAttribute("", "begin", "00:00:00");
        div->parseAttribute("", "end", "00:00:10");
        div->appendText("text");
        doc.endElement();

        doc.endElement(); //tt

        auto timeline = doc.generateTimeline();

        CPPUNIT_ASSERT(! timeline.empty());
        CPPUNIT_ASSERT(timeline.size() == 1);

        auto firstDoc = timeline.front();

        auto t = Timing{TimePoint{0}, TimePoint{10000}};
        CPPUNIT_ASSERT(firstDoc.m_timing == t);

        CPPUNIT_ASSERT(firstDoc.m_entites.size() == 1);

        auto firstEntity = firstDoc.m_entites[0];
        CPPUNIT_ASSERT(firstEntity.m_textLines.size() == 1);

        auto firstLine = firstEntity.m_textLines[0];
        CPPUNIT_ASSERT(firstLine.size() == 1);

        auto textChunk = firstLine[0];
        CPPUNIT_ASSERT(textChunk.m_text == "text");

        CPPUNIT_ASSERT(textChunk.m_style == StyleSet{});
    }

    void referredStyle()
    {
        DocumentInstance doc{};

        doc.startElement("tt");
        auto testFontName = "SomeFont";
        auto style = doc.startElement("style");
        style->parseAttribute("", "id", "style1");
        style->parseAttribute("tts", "fontFamily", testFontName);
        doc.endElement();

        auto div = doc.startElement("div");
        div->parseAttribute("", "style", "style1");
        div->parseAttribute("", "begin", "00:00:00");
        div->parseAttribute("", "end", "00:00:10");
        div->appendText("text");
        doc.endElement();

        doc.endElement(); //tt

        auto timeline = doc.generateTimeline();

        CPPUNIT_ASSERT(! timeline.empty());
        CPPUNIT_ASSERT(timeline.size() == 1);

        auto firstDoc = timeline.front();

        CPPUNIT_ASSERT(firstDoc.m_entites.size() == 1);

        auto firstEntity = firstDoc.m_entites[0];
        CPPUNIT_ASSERT(firstEntity.m_textLines.size() == 1);

        auto firstLine = firstEntity.m_textLines[0];
        CPPUNIT_ASSERT(firstLine.size() == 1);

        auto textChunk = firstLine[0];
        CPPUNIT_ASSERT(textChunk.m_text == "text");

        CPPUNIT_ASSERT(!(textChunk.m_style == StyleSet{}));
        CPPUNIT_ASSERT(textChunk.m_style.getFontFamily() == std::string{testFontName});
    }

    void styleInheritanceFromParent()
    {
        DocumentInstance doc{};

        CPPUNIT_ASSERT(doc.generateTimeline().empty());

        doc.startElement("tt");
        auto div = doc.startElement("div");
        div->parseAttribute("tts", "textAlign", "center");
        div->parseAttribute("", "begin", "00:00:00");
        div->parseAttribute("", "end", "00:00:10");
        div->appendText("div_text");

        auto pElem = doc.startElement("p");
        pElem->parseAttribute("", "begin", "00:00:00");
        pElem->parseAttribute("", "end", "00:00:10");
        pElem->appendText("p_text");

        doc.endElement(); //p
        doc.endElement(); //div
        doc.endElement(); //tt

        auto timeline = doc.generateTimeline();
        CPPUNIT_ASSERT(timeline.size() == 1);

        auto firstDoc = timeline.front();
        CPPUNIT_ASSERT(firstDoc.m_entites.size() == 1);

        auto firstEntity = firstDoc.m_entites[0];
        CPPUNIT_ASSERT(firstEntity.m_textLines.size() == 1);

        auto firstLine = firstEntity.m_textLines[0];
        CPPUNIT_ASSERT(firstLine.size() == 2);

        auto firstChunk = firstLine[0];
        CPPUNIT_ASSERT(firstChunk.m_text == "div_text");
        CPPUNIT_ASSERT(firstChunk.m_style.getTextAlign() == StyleSet::TextAlign::CENTER);

        auto secondChunk = firstLine[1];
        CPPUNIT_ASSERT(secondChunk.m_text == "p_text");
        CPPUNIT_ASSERT(firstChunk.m_style.getTextAlign() == StyleSet::TextAlign::CENTER);
    }

    void styleInheritanceRegionOverStyle()
    {
        DocumentInstance doc{};

        CPPUNIT_ASSERT(doc.generateTimeline().empty());

        doc.startElement("tt");
        auto style = doc.startElement("style");
        style->parseAttribute("", "id", "style1");
        style->parseAttribute("tts", "textAlign", "right");
        doc.endElement();

        auto region = doc.startElement("region");
        region->parseAttribute("","id", "region1");
        region->parseAttribute("tts", "textAlign", "center");
        doc.endElement();

        auto div = doc.startElement("div");
        div->parseAttribute("", "style", "style1");
        div->parseAttribute("", "region", "region1");
        div->parseAttribute("", "begin", "00:00:00");
        div->parseAttribute("", "end", "00:00:10");
        div->appendText("text");
        doc.endElement();

        doc.endElement(); //tt

        auto timeline = doc.generateTimeline();
        CPPUNIT_ASSERT(timeline.size() == 1);

        auto firstDoc = timeline.front();
        CPPUNIT_ASSERT(firstDoc.m_entites.size() == 1);

        auto firstEntity = firstDoc.m_entites[0];
        CPPUNIT_ASSERT(firstEntity.m_textLines.size() == 1);

        auto firstLine = firstEntity.m_textLines[0];
        CPPUNIT_ASSERT(firstLine.size() == 1);

        auto textChunk = firstLine[0];
        CPPUNIT_ASSERT(textChunk.m_text == "text");

        CPPUNIT_ASSERT(textChunk.m_style.getTextAlign() == StyleSet::TextAlign::CENTER);
    }

    void styleInheritanceRegionStyleOverStyle()
    {
        DocumentInstance doc{};

        CPPUNIT_ASSERT(doc.generateTimeline().empty());

        doc.startElement("tt");
        auto style1 = doc.startElement("style");
        style1->parseAttribute("", "id", "style1");
        style1->parseAttribute("tts", "textAlign", "right");
        doc.endElement();

        auto style2 = doc.startElement("style");
        style2->parseAttribute("", "id", "style2");
        style2->parseAttribute("tts", "textAlign", "center");
        doc.endElement();

        auto region = doc.startElement("region");
        region->parseAttribute("","id", "region1");
        region->parseAttribute("", "style", "style2");
        doc.endElement();

        auto div = doc.startElement("div");
        div->parseAttribute("", "style", "style1");
        div->parseAttribute("", "region", "region1");
        div->parseAttribute("", "begin", "00:00:00");
        div->parseAttribute("", "end", "00:00:10");
        div->appendText("text");
        doc.endElement();

        doc.endElement(); //tt

        auto timeline = doc.generateTimeline();
        CPPUNIT_ASSERT(timeline.size() == 1);

        auto firstDoc = timeline.front();
        CPPUNIT_ASSERT(firstDoc.m_entites.size() == 1);

        auto firstEntity = firstDoc.m_entites[0];
        CPPUNIT_ASSERT(firstEntity.m_textLines.size() == 1);

        auto firstLine = firstEntity.m_textLines[0];
        CPPUNIT_ASSERT(firstLine.size() == 1);

        auto textChunk = firstLine[0];
        CPPUNIT_ASSERT(textChunk.m_text == "text");

        CPPUNIT_ASSERT(textChunk.m_style.getTextAlign() == StyleSet::TextAlign::CENTER);
    }

    void styleInheritanceRegionOverRegionStyle()
    {
        DocumentInstance doc{};

        CPPUNIT_ASSERT(doc.generateTimeline().empty());

        doc.startElement("tt");
        auto style1 = doc.startElement("style");
        style1->parseAttribute("", "id", "style1");
        style1->parseAttribute("tts", "textAlign", "right");
        doc.endElement();

        auto style2 = doc.startElement("style");
        style2->parseAttribute("", "id", "style2");
        style2->parseAttribute("tts", "textAlign", "end");
        doc.endElement();

        auto region = doc.startElement("region");
        region->parseAttribute("","id", "region1");
        region->parseAttribute("", "style", "style2");
        region->parseAttribute("tts", "textAlign", "center");
        doc.endElement();

        auto div = doc.startElement("div");
        div->parseAttribute("", "style", "style1");
        div->parseAttribute("", "region", "region1");
        div->parseAttribute("", "begin", "00:00:00");
        div->parseAttribute("", "end", "00:00:10");
        div->appendText("text");
        doc.endElement();

        doc.endElement(); //tt

        auto timeline = doc.generateTimeline();
        CPPUNIT_ASSERT(timeline.size() == 1);

        auto firstDoc = timeline.front();
        CPPUNIT_ASSERT(firstDoc.m_entites.size() == 1);

        auto firstEntity = firstDoc.m_entites[0];
        CPPUNIT_ASSERT(firstEntity.m_textLines.size() == 1);

        auto firstLine = firstEntity.m_textLines[0];
        CPPUNIT_ASSERT(firstLine.size() == 1);

        auto textChunk = firstLine[0];
        CPPUNIT_ASSERT(textChunk.m_text == "text");

        CPPUNIT_ASSERT(textChunk.m_style.getTextAlign() == StyleSet::TextAlign::CENTER);
    }

    void styleInheritanceElementOverRegion()
    {
      DocumentInstance doc{};

      CPPUNIT_ASSERT(doc.generateTimeline().empty());

      doc.startElement("tt");
      auto style1 = doc.startElement("style");
      style1->parseAttribute("", "id", "style1");
      style1->parseAttribute("tts", "textAlign", "right");
      doc.endElement();

      auto style2 = doc.startElement("style");
      style2->parseAttribute("", "id", "style2");
      style2->parseAttribute("tts", "textAlign", "end");
      doc.endElement();

      auto region = doc.startElement("region");
      region->parseAttribute("","id", "region1");
      region->parseAttribute("", "style", "style2");
      region->parseAttribute("tts", "textAlign", "left");
      doc.endElement();

      auto div = doc.startElement("div");
      div->parseAttribute("", "style", "style1");
      div->parseAttribute("", "region", "region1");
      div->parseAttribute("tts", "textAlign", "center");
      div->parseAttribute("", "begin", "00:00:00");
      div->parseAttribute("", "end", "00:00:10");
      div->appendText("text");
      doc.endElement();

      doc.endElement(); //tt

      auto timeline = doc.generateTimeline();
      CPPUNIT_ASSERT(timeline.size() == 1);

      auto firstDoc = timeline.front();
      CPPUNIT_ASSERT(firstDoc.m_entites.size() == 1);

      auto firstEntity = firstDoc.m_entites[0];
      CPPUNIT_ASSERT(firstEntity.m_textLines.size() == 1);

      auto firstLine = firstEntity.m_textLines[0];
      CPPUNIT_ASSERT(firstLine.size() == 1);

      auto textChunk = firstLine[0];
      CPPUNIT_ASSERT(textChunk.m_text == "text");

      CPPUNIT_ASSERT(textChunk.m_style.getTextAlign() == StyleSet::TextAlign::CENTER);
    }

    /**
     * @brief Test region geometry
     *
     * Parse a TTML document:
     *
     * <tt>
     *   ...
     *   <region id="region1"
     *           tts:origin="10% 20%"
     *           tts:extent="30% 40%"
     *           tts:textAlign="center"/>
     *   ...
     *   <p region="region1"
     *      begin="00:00:00.000"
     *      end="00:00:10.000">
     *     p_text
     *   </p>
     *   ...
     * </tt>
     *
     * Verify the entity region origin and extent.
     */
    void regionGeometry()
    {
        DocumentInstance doc{};

        CPPUNIT_ASSERT(doc.generateTimeline().empty());

        doc.startElement("tt");
        auto region = doc.startElement("region");
        region->parseAttribute("", "id", "region1");
        region->parseAttribute("tts", "origin", "10% 20%");
        region->parseAttribute("tts", "extent", "30% 40%");
        region->parseAttribute("tts", "textAlign", "center");
        doc.endElement(); // region

        auto pElem = doc.startElement("p");
        pElem->parseAttribute("", "begin", "00:00:00");
        pElem->parseAttribute("", "end", "00:00:10");
        pElem->parseAttribute("", "region", "region1");
        pElem->appendText("p_text");

        doc.endElement(); // p
        doc.endElement(); // tt

        auto timeline = doc.generateTimeline();
        CPPUNIT_ASSERT(timeline.size() == 1);

        auto firstDoc = timeline.front();
        CPPUNIT_ASSERT(firstDoc.m_entites.size() == 1);

        auto firstEntity = firstDoc.m_entites[0];
        CPPUNIT_ASSERT(firstEntity.m_textLines.size() == 1);
        CPPUNIT_ASSERT(firstEntity.m_region);
        CPPUNIT_ASSERT(firstEntity.m_region->getX().getValue() == 1000);
        CPPUNIT_ASSERT(firstEntity.m_region->getY().getValue() == 2000);
        CPPUNIT_ASSERT(firstEntity.m_region->getWidth().getValue() == 3000);
        CPPUNIT_ASSERT(firstEntity.m_region->getHeight().getValue() == 4000);

        auto firstLine = firstEntity.m_textLines[0];
        CPPUNIT_ASSERT(firstLine.size() == 1);

        auto firstChunk = firstLine[0];
        CPPUNIT_ASSERT(firstChunk.m_text == "p_text");
        CPPUNIT_ASSERT(firstChunk.m_style.getTextAlign() == StyleSet::TextAlign::CENTER);
    }

    /**
     * @brief Test region geometry inherited from a body element
     *
     * Parse a TTML document:
     *
     * <tt>
     *   ...
     *   <region id="region1"
     *           tts:origin="10% 20%"
     *           tts:extent="30% 40%"
     *           tts:textAlign="center"/>
     *   ...
     *   <body region="region1">
     *     ...
     *     <p begin="00:00:00.000"
     *        end="00:00:10.000">
     *       p_text
     *     </p>
     *     ...
     *   </body>
     * </tt>
     *
     * Verify the entity region origin and extent.
     */
    void regionGeometryInheritanceFromBody()
    {
        DocumentInstance doc{};

        CPPUNIT_ASSERT(doc.generateTimeline().empty());

        doc.startElement("tt");
        auto region = doc.startElement("region");
        region->parseAttribute("", "id", "region1");
        region->parseAttribute("tts", "origin", "10% 20%");
        region->parseAttribute("tts", "extent", "30% 40%");
        region->parseAttribute("tts", "textAlign", "center");
        doc.endElement(); // region

        auto body = doc.startElement("body");
        body->parseAttribute("", "region", "region1");

        auto pElem = doc.startElement("p");
        pElem->parseAttribute("", "begin", "00:00:00");
        pElem->parseAttribute("", "end", "00:00:10");
        pElem->appendText("p_text");

        doc.endElement(); // p
        doc.endElement(); // body
        doc.endElement(); // tt

        auto timeline = doc.generateTimeline();
        CPPUNIT_ASSERT(timeline.size() == 1);

        auto firstDoc = timeline.front();
        CPPUNIT_ASSERT(firstDoc.m_entites.size() == 1);

        auto firstEntity = firstDoc.m_entites[0];
        CPPUNIT_ASSERT(firstEntity.m_textLines.size() == 1);
        CPPUNIT_ASSERT(firstEntity.m_region);
        CPPUNIT_ASSERT(firstEntity.m_region->getX().getValue() == 1000);
        CPPUNIT_ASSERT(firstEntity.m_region->getY().getValue() == 2000);
        CPPUNIT_ASSERT(firstEntity.m_region->getWidth().getValue() == 3000);
        CPPUNIT_ASSERT(firstEntity.m_region->getHeight().getValue() == 4000);

        auto firstLine = firstEntity.m_textLines[0];
        CPPUNIT_ASSERT(firstLine.size() == 1);

        auto firstChunk = firstLine[0];
        CPPUNIT_ASSERT(firstChunk.m_text == "p_text");
        CPPUNIT_ASSERT(firstChunk.m_style.getTextAlign() == StyleSet::TextAlign::CENTER);
    }

    /**
     * @brief Test region geometry applied to multiple elements - case 1
     *
     * Parse a TTML document:
     *
     * <tt>
     * ...
     * <region xml:id="speaker_1" tts:origin="14% 60%" tts:extent="68% 16%" tts:textAlign="center"/>
     * <region xml:id="speaker_2" tts:origin="14% 70%" tts:extent="68% 16%" tts:textAlign="center"/>
     * ...
     * <body>
     *   ...
     *   <p begin="00:00:00.000" end="999999999:26:21.587" region="speaker_2">
     *     <span>Span 1</span>
     *     <br/>
     *     <span>-Span 2-</span>
     *     <br/>
     *     <span>--Span 3--</span>
     *   </p>
     *   ...
     * </body>
     * ...
     * </tt>
     */
    void multiSpanRegionCase1()
    {
        DocumentInstance doc{};

        CPPUNIT_ASSERT(doc.generateTimeline().empty());

        doc.startElement("tt");

        auto region1 = doc.startElement("region");
        region1->parseAttribute("xml", "id", "speaker_1");
        region1->parseAttribute("tts", "origin", "14% 60%");
        region1->parseAttribute("tts", "extent", "68% 16%");
        region1->parseAttribute("tts", "textAlign", "center");
        doc.endElement(); // region

        auto region2 = doc.startElement("region");
        region2->parseAttribute("xml", "id", "speaker_2");
        region2->parseAttribute("tts", "origin", "14% 70%");
        region2->parseAttribute("tts", "extent", "68% 16%");
        region2->parseAttribute("tts", "textAlign", "center");
        doc.endElement(); // region

        auto body = doc.startElement("body");

        auto pElem = doc.startElement("p");
        pElem->parseAttribute("", "begin", "00:00:00.000");
        pElem->parseAttribute("", "end", "999999999:26:21.587");
        pElem->parseAttribute("", "region", "speaker_2");

        auto span1 = doc.startElement("span");
        span1->appendText("Span 1");
        doc.endElement(); // span

        auto br1 = doc.startElement("br");
        doc.endElement(); // br

        auto span2 = doc.startElement("span");
        span2->appendText("-Span 2-");
        doc.endElement(); // span

        auto br2 = doc.startElement("br");
        doc.endElement(); // br

        auto span3 = doc.startElement("span");
        span3->appendText("--Span 3--");
        doc.endElement(); // span

        doc.endElement(); // p
        doc.endElement(); // body
        doc.endElement(); // tt

        auto timeline = doc.generateTimeline();
        CPPUNIT_ASSERT(timeline.size() == 1);

        auto firstDoc = timeline.front();
        CPPUNIT_ASSERT(firstDoc.m_entites.size() == 1);

        auto firstEntity = firstDoc.m_entites[0];
        CPPUNIT_ASSERT(firstEntity.m_textLines.size() == 3);
        CPPUNIT_ASSERT(firstEntity.m_region);
        CPPUNIT_ASSERT(firstEntity.m_region->getX().getValue() == 1400);
        CPPUNIT_ASSERT(firstEntity.m_region->getY().getValue() == 7000);
        CPPUNIT_ASSERT(firstEntity.m_region->getWidth().getValue() == 6800);
        CPPUNIT_ASSERT(firstEntity.m_region->getHeight().getValue() == 1600);

        auto firstLine = firstEntity.m_textLines[0];
        CPPUNIT_ASSERT(firstLine.size() == 1);

        auto firstChunk = firstLine[0];
        CPPUNIT_ASSERT(firstChunk.m_text == "Span 1");
        CPPUNIT_ASSERT(firstChunk.m_style.getTextAlign() == StyleSet::TextAlign::CENTER);

        auto secondLine = firstEntity.m_textLines[1];
        CPPUNIT_ASSERT(secondLine.size() == 1);

        auto secondChunk = secondLine[0];
        CPPUNIT_ASSERT(secondChunk.m_text == "-Span 2-");
        CPPUNIT_ASSERT(secondChunk.m_style.getTextAlign() == StyleSet::TextAlign::CENTER);

        auto thirdLine = firstEntity.m_textLines[2];
        CPPUNIT_ASSERT(thirdLine.size() == 1);

        auto thirdChunk = thirdLine[0];
        CPPUNIT_ASSERT(thirdChunk.m_text == "--Span 3--");
        CPPUNIT_ASSERT(thirdChunk.m_style.getTextAlign() == StyleSet::TextAlign::CENTER);
    }

    /**
     * @brief Test region geometry applied to multiple elements - case 2
     *
     * Parse a TTML document:
     *
     * <tt>
     * ...
     * <region xml:id="speaker_1" tts:origin="14% 60%" tts:extent="68% 16%" tts:textAlign="center"/>
     * <region xml:id="speaker_2" tts:origin="14% 70%" tts:extent="68% 16%" tts:textAlign="center"/>
     * ...
     * <body>
     *   ...
     *   <p begin="00:00:00.000" end="999999999:26:21.587" region="speaker_1">
     *     <span>Span 1</span>
     *     <span>-Span 2-</span>
     *     <span>--Span 3--</span>*
     *   </p>
     *   ...
     * </body>
     * ...
     * </tt>
     */
    void multiSpanRegionCase2()
    {
        DocumentInstance doc{};

        CPPUNIT_ASSERT(doc.generateTimeline().empty());

        doc.startElement("tt");

        auto region1 = doc.startElement("region");
        region1->parseAttribute("xml", "id", "speaker_1");
        region1->parseAttribute("tts", "origin", "14% 60%");
        region1->parseAttribute("tts", "extent", "68% 16%");
        region1->parseAttribute("tts", "textAlign", "center");
        doc.endElement(); // region

        auto region2 = doc.startElement("region");
        region2->parseAttribute("xml", "id", "speaker_2");
        region2->parseAttribute("tts", "origin", "14% 70%");
        region2->parseAttribute("tts", "extent", "68% 16%");
        region2->parseAttribute("tts", "textAlign", "center");
        doc.endElement(); // region

        auto body = doc.startElement("body");

        auto pElem = doc.startElement("p");
        pElem->parseAttribute("", "begin", "00:00:00.000");
        pElem->parseAttribute("", "end", "999999999:26:21.587");
        pElem->parseAttribute("", "region", "speaker_1");

        auto span1 = doc.startElement("span");
        span1->appendText("Span 1");
        doc.endElement(); // span

        auto span2 = doc.startElement("span");
        span2->appendText("-Span 2-");
        doc.endElement(); // span

        auto span3 = doc.startElement("span");
        span3->appendText("--Span 3--");
        doc.endElement(); // span

        doc.endElement(); // p
        doc.endElement(); // body
        doc.endElement(); // tt

        auto timeline = doc.generateTimeline();
        CPPUNIT_ASSERT(timeline.size() == 1);

        auto firstDoc = timeline.front();
        CPPUNIT_ASSERT(firstDoc.m_entites.size() == 1);

        auto firstEntity = firstDoc.m_entites[0];
        CPPUNIT_ASSERT(firstEntity.m_textLines.size() == 1);
        CPPUNIT_ASSERT(firstEntity.m_region);
        CPPUNIT_ASSERT(firstEntity.m_region->getX().getValue() == 1400);
        CPPUNIT_ASSERT(firstEntity.m_region->getY().getValue() == 6000);
        CPPUNIT_ASSERT(firstEntity.m_region->getWidth().getValue() == 6800);
        CPPUNIT_ASSERT(firstEntity.m_region->getHeight().getValue() == 1600);

        auto firstLine = firstEntity.m_textLines[0];
        CPPUNIT_ASSERT(firstLine.size() == 3);

        auto firstChunk = firstLine[0];
        CPPUNIT_ASSERT(firstChunk.m_text == "Span 1");
        CPPUNIT_ASSERT(firstChunk.m_style.getTextAlign() == StyleSet::TextAlign::CENTER);

        auto secondChunk = firstLine[1];
        CPPUNIT_ASSERT(secondChunk.m_text == "-Span 2-");
        CPPUNIT_ASSERT(secondChunk.m_style.getTextAlign() == StyleSet::TextAlign::CENTER);

        auto thirdChunk = firstLine[2];
        CPPUNIT_ASSERT(thirdChunk.m_text == "--Span 3--");
        CPPUNIT_ASSERT(thirdChunk.m_style.getTextAlign() == StyleSet::TextAlign::CENTER);
    }

    void defaultWhitespaceHandling()
    {
      DocumentInstance doc{};

      CPPUNIT_ASSERT(doc.generateTimeline().empty());

      doc.startElement("tt");
      auto div = doc.startElement("div");
      div->parseAttribute("", "begin", "00:00:00");
      div->parseAttribute("", "end", "00:00:10");
      div->appendText("  \n \n div  text \n   ");

      auto pElem1 = doc.startElement("p");
      pElem1->appendText("    p  \n\n\n  text \n\n ");
      doc.endElement(); //p

      auto pElem2 = doc.startElement("p");
      pElem2->appendText("p_text");
      doc.endElement(); //p

      auto pElem3 = doc.startElement("p");

      auto spanElem1 = doc.startElement("span");
      spanElem1->appendText(" 123");
      doc.endElement(); //span

      //mimic <span> element in a new line
      auto pElem3Copy = doc.getCurrentElement();
      if (pElem3Copy) {
          pElem3Copy->appendText("\n    \n    ");
      }

      auto spanElem2 = doc.startElement("span");
      spanElem2->appendText("456 ");
      doc.endElement(); //span

      doc.endElement(); //p

      doc.endElement(); //div
      doc.endElement(); //tt

      auto timeline = doc.generateTimeline();
      CPPUNIT_ASSERT(timeline.size() == 1);

      auto firstDoc = timeline.front();
      CPPUNIT_ASSERT(firstDoc.m_entites.size() == 1);

      auto firstEntity = firstDoc.m_entites[0];
      CPPUNIT_ASSERT(firstEntity.m_textLines.size() == 1);

      auto firstLine = firstEntity.m_textLines[0];
      CPPUNIT_ASSERT(firstLine.size() == 6);

      auto firstChunk = firstLine[0];
      CPPUNIT_ASSERT(firstChunk.m_text == "div text");

      auto secondChunk = firstLine[1];
      CPPUNIT_ASSERT(secondChunk.m_text == " p text ");

      auto thirdChunk = firstLine[2];
      CPPUNIT_ASSERT(thirdChunk.m_text == "p_text");

      auto fourthChunk = firstLine[3];
      CPPUNIT_ASSERT(fourthChunk.m_text == " 123");

      auto fifthChunk = firstLine[4];
      CPPUNIT_ASSERT(fifthChunk.m_text == " ");

      auto sixthChunk = firstLine[5];
      CPPUNIT_ASSERT(sixthChunk.m_text == "456");
    }

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( DocumentInstanceTest );

