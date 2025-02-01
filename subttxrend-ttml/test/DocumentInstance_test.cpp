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
    CPPUNIT_TEST(basicDocumentInstance);
    CPPUNIT_TEST(referredStyle);
    CPPUNIT_TEST(styleInheritanceFromParent);
    CPPUNIT_TEST(styleInheritanceRegionOverStyle);
    CPPUNIT_TEST(styleInheritanceRegionStyleOverStyle);
    CPPUNIT_TEST(styleInheritanceRegionOverRegionStyle);
    CPPUNIT_TEST(styleInheritanceElementOverRegion);
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
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( DocumentInstanceTest );

