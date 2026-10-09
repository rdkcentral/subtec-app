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
#include "Parser/Elements.hpp"

using namespace subttxrend::ttmlengine;

class ElementsTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( ElementsTest );
    CPPUNIT_TEST(testElementParseAttributeSetsIdAndDerived);
    CPPUNIT_TEST(testElementExpectTextContentDefault);
    CPPUNIT_TEST(testRegionElementDefaultConstructor);
    CPPUNIT_TEST(testRegionElementParameterizedConstructor);
    CPPUNIT_TEST(testRegionElementAddAttributeDelegates);
    CPPUNIT_TEST(testRegionElementGetStyleId);
    CPPUNIT_TEST(testImageElementExpectTextContent);
    CPPUNIT_TEST(testBodyElementDefaultConstructor);
    CPPUNIT_TEST(testBodyElementParentConstructor);
    CPPUNIT_TEST(testBodyElementExpectTextContent);
    CPPUNIT_TEST(testBodyElementAppendText);
    CPPUNIT_TEST(testBodyElementAppendNewline);
    CPPUNIT_TEST(testBodyElementGetters);
    CPPUNIT_TEST(testBodyElementHasValidContent);
    CPPUNIT_TEST(testBodyElementFinalizeRemovesEmptyLines);
    CPPUNIT_TEST(testBodyElementParseAttributeDerived);
    CPPUNIT_TEST(testTTElementDefaultConstructor);
    CPPUNIT_TEST(testTTElementParseAttributeDerivedCellResolution);
    CPPUNIT_TEST(testTTElementParseAttributeDerivedCellResolutionZero);
    CPPUNIT_TEST(testTTElementParseAttributeDerivedCellResolutionInvalid);
    CPPUNIT_TEST(testStyleElementMergeAttributes);
    CPPUNIT_TEST(testRegionElementParseOriginExtentPercent);
    CPPUNIT_TEST(testRegionElementParseOriginExtentPixels);
    CPPUNIT_TEST(testRegionElementParseOriginExtentInvalid);
    CPPUNIT_TEST(testImageElementAppendMultipleChunks);
    CPPUNIT_TEST(testBodyElementWhitespaceHandlingDefault);
    CPPUNIT_TEST(testBodyElementWhitespaceHandlingPreserve);
    CPPUNIT_TEST(testRegionElementToStr);
    CPPUNIT_TEST(testBodyElementParseAttributeDerivedUnknown);
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

    // --- Element base class ---
    void testElementParseAttributeSetsIdAndDerived()
    {
        class TestElement : public Element {
        public:
            bool derivedCalled = false;
            void parseAttributeDerived(const std::string& name, const std::string& value) override {
                derivedCalled = true;
            }
        };
        TestElement elem;
        elem.parseAttribute("", "id", "foo");
        CPPUNIT_ASSERT_EQUAL(std::string("foo"), elem.getId());
        elem.parseAttribute("", "other", "bar");
        CPPUNIT_ASSERT(elem.derivedCalled);
    }

    void testElementExpectTextContentDefault()
    {
        Element* elem = new StyleElement();
        CPPUNIT_ASSERT(!elem->expectTextContent());
        delete elem;
    }

    // --- StyleElement ---
    void testStyleElementMergeAttributes()
    {
        StyleElement style;
        style.parseAttribute("tts", "color", "red");
        style.parseAttribute("tts", "font", "Arial");
        CPPUNIT_ASSERT(style.getStyleAttributes().at("color") == "red");
        CPPUNIT_ASSERT(style.getStyleAttributes().at("font") == "Arial");
    }

    // --- RegionElement ---
    void testRegionElementDefaultConstructor()
    {
        RegionElement region;
        CPPUNIT_ASSERT(region.getX().getValue() == 0);
        CPPUNIT_ASSERT(region.getY().getValue() == 0);
        CPPUNIT_ASSERT(region.getWidth().getValue() == 10000);
        CPPUNIT_ASSERT(region.getHeight().getValue() == 10000);
    }

    void testRegionElementParameterizedConstructor()
    {
        RegionElement region(10, 20, 30, 40);
        CPPUNIT_ASSERT(region.getX().getValue() == 10);
        CPPUNIT_ASSERT(region.getY().getValue() == 20);
        CPPUNIT_ASSERT(region.getWidth().getValue() == 30);
        CPPUNIT_ASSERT(region.getHeight().getValue() == 40);
    }

    void testRegionElementAddAttributeDelegates()
    {
        RegionElement region;
        region.addAttribute("color", "blue");
        CPPUNIT_ASSERT(region.getStyleAttributes().at("color") == "blue");
    }

    void testRegionElementGetStyleId()
    {
        RegionElement region;
        region.parseAttribute("", "style", "myStyle");
        CPPUNIT_ASSERT(region.getStyleId() == "myStyle");
    }

    void testRegionElementParseOriginExtentPercent()
    {
        RegionElement region;
        region.addAttribute("origin", "12.5% 87.5%");
        region.addAttribute("extent", "50% 25%");
        CPPUNIT_ASSERT(region.getX().getValue() == 1250);
        CPPUNIT_ASSERT(region.getY().getValue() == 8750);
        CPPUNIT_ASSERT(region.getWidth().getValue() == 5000);
        CPPUNIT_ASSERT(region.getHeight().getValue() == 2500);
    }

    void testRegionElementParseOriginExtentPixels()
    {
        RegionElement region;
        region.addAttribute("origin", "100px 200px");
        region.addAttribute("extent", "300px 400px");
        CPPUNIT_ASSERT(region.getX().getValue() == 100);
        CPPUNIT_ASSERT(region.getY().getValue() == 200);
        CPPUNIT_ASSERT(region.getWidth().getValue() == 300);
        CPPUNIT_ASSERT(region.getHeight().getValue() == 400);
    }

    void testRegionElementParseOriginExtentInvalid()
    {
        RegionElement region;
        region.addAttribute("origin", "bad bad");
        region.addAttribute("extent", "bad bad");
        CPPUNIT_ASSERT(region.getX().getValue() == 0);
        CPPUNIT_ASSERT(region.getY().getValue() == 0);
        CPPUNIT_ASSERT(region.getWidth().getValue() == 0);
        CPPUNIT_ASSERT(region.getHeight().getValue() == 0);
    }

    // --- ImageElement ---
    void testImageElementExpectTextContent()
    {
        ImageElement img;
        CPPUNIT_ASSERT(img.expectTextContent());
    }

    void testImageElementAppendMultipleChunks()
    {
        ImageElement img;
        img.appendText("abc");
        img.appendText("def");
        CPPUNIT_ASSERT(*(img.getBase64Data()) == "abcdef");
    }

    // --- BodyElement ---
    void testBodyElementDefaultConstructor()
    {
        BodyElement body;
        CPPUNIT_ASSERT(body.getWhiteSpaceHandling() == XmlSpace::DEFAULT);
        CPPUNIT_ASSERT(body.getStyleId().empty());
        CPPUNIT_ASSERT(body.getRegionId().empty());
        CPPUNIT_ASSERT(body.getTextLines().size() == 1);
    }

    void testBodyElementParentConstructor()
    {
        auto parent = std::make_shared<BodyElement>();
        parent->parseAttribute("", "begin", "00:00:01.100");
        parent->parseAttribute("", "end", "00:00:02.200");
        parent->parseAttribute("", "space", "preserve");
        parent->parseAttribute("", "style", "parentStyle");
        parent->parseAttribute("", "region", "parentRegion");
        BodyElement child(parent);

        CPPUNIT_ASSERT(child.getParent() == parent);
        CPPUNIT_ASSERT(child.getWhiteSpaceHandling() == XmlSpace::PRESERVE);
        CPPUNIT_ASSERT(child.getStyleId() == "parentStyle");
        CPPUNIT_ASSERT(child.getRegionId() == "parentRegion");
        CPPUNIT_ASSERT(child.getTiming().getStartTimeRef() == TimePoint(1100));
        CPPUNIT_ASSERT(child.getTiming().getEndTimeRef() == TimePoint(2200));
    }

    void testBodyElementExpectTextContent()
    {
        BodyElement body;
        CPPUNIT_ASSERT(body.expectTextContent());
    }

    void testBodyElementAppendText()
    {
        BodyElement body;
        body.appendText("hello");
        CPPUNIT_ASSERT(body.getTextLines().back().text == "hello");
    }

    void testBodyElementAppendNewline()
    {
        BodyElement body;
        body.appendText("hello");
        body.appendNewline();
        CPPUNIT_ASSERT(body.getTextLines().size() == 2);
        CPPUNIT_ASSERT(body.getTextLines().back().isForcedLine);
    }

    void testBodyElementGetters()
    {
        BodyElement body;
        body.parseAttribute("", "style", "s1");
        body.parseAttribute("", "region", "r1");
        body.parseAttribute("", "backgroundImage", "#img1");
        CPPUNIT_ASSERT(body.getStyleId() == "s1");
        CPPUNIT_ASSERT(body.getRegionId() == "r1");
        CPPUNIT_ASSERT(body.getBackgroundImageId() == "img1");
    }

    void testBodyElementHasValidContent()
    {
        BodyElement body;
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), body.getTextLines().size());
        CPPUNIT_ASSERT(body.hasValidContent());
        body.appendNewline();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), body.getTextLines().size());
        CPPUNIT_ASSERT(body.getTextLines().back().isForcedLine);
        CPPUNIT_ASSERT(body.hasValidContent());
    }

    void testBodyElementFinalizeRemovesEmptyLines()
    {
        BodyElement body;
        body.appendText("");       // No-op on the initial non-forced empty line
        body.appendNewline();      // Adds a forced empty line
        body.appendText("abc");    // Appends to the current forced line
        body.appendNewline();      // Adds another forced empty line
        body.appendText("");       // No-op on the trailing forced empty line
        body.finalize();

        const auto& lines = body.getTextLines();
        CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), lines.size());
        CPPUNIT_ASSERT(lines[0].isForcedLine);
        CPPUNIT_ASSERT_EQUAL(std::string("abc"), lines[0].text);
        CPPUNIT_ASSERT(lines[1].isForcedLine);
        CPPUNIT_ASSERT(lines[1].text.empty());
    }

    void testBodyElementParseAttributeDerived()
    {
        BodyElement body;
        body.parseAttribute("", "begin", "01:02:03.004");
        body.parseAttribute("", "end", "01:02:04.005");
        body.parseAttribute("", "space", "preserve");
        body.parseAttribute("", "style", "s1");
        body.parseAttribute("", "region", "r1");
        body.parseAttribute("", "backgroundImage", "#img1");
        const Timing timing = body.getTiming();

        CPPUNIT_ASSERT(timing.getStartTimeRef() == TimePoint(std::chrono::hours(1), std::chrono::minutes(2),
                                                             std::chrono::seconds(3), std::chrono::milliseconds(4)));
        CPPUNIT_ASSERT(timing.getEndTimeRef() == TimePoint(std::chrono::hours(1), std::chrono::minutes(2),
                                                           std::chrono::seconds(4), std::chrono::milliseconds(5)));
        CPPUNIT_ASSERT(body.getWhiteSpaceHandling() == XmlSpace::PRESERVE);
        CPPUNIT_ASSERT(body.getStyleId() == "s1");
        CPPUNIT_ASSERT(body.getRegionId() == "r1");
        CPPUNIT_ASSERT(body.getBackgroundImageId() == "img1");
    }

    void testBodyElementWhitespaceHandlingDefault()
    {
        BodyElement body;
        body.appendText("a   b");
        body.finalize();
        // Should collapse spaces if XmlSpace::DEFAULT
        CPPUNIT_ASSERT(body.getTextLines().back().text.find("  ") == std::string::npos);
    }

    void testBodyElementWhitespaceHandlingPreserve()
    {
        BodyElement body;
        body.parseAttribute("", "space", "preserve");
        body.appendText("a   b");
        body.finalize();
        // Should preserve spaces
        CPPUNIT_ASSERT(body.getTextLines().back().text.find("  ") != std::string::npos);
    }

    // --- TTElement ---
    void testTTElementDefaultConstructor()
    {
        TTElement tt;
        CPPUNIT_ASSERT(tt.getCellResolution().m_w == 32);
        CPPUNIT_ASSERT(tt.getCellResolution().m_h == 15);
    }

    void testTTElementParseAttributeDerivedCellResolution()
    {
        TTElement tt;
        tt.parseAttribute("", "cellResolution", "41 24");
        CPPUNIT_ASSERT(tt.getCellResolution().m_w == 41);
        CPPUNIT_ASSERT(tt.getCellResolution().m_h == 24);
    }

    void testTTElementParseAttributeDerivedCellResolutionZero()
    {
        TTElement tt;
        tt.parseAttribute("", "cellResolution", "0 0");
        CPPUNIT_ASSERT(tt.getCellResolution().m_w == 32);
        CPPUNIT_ASSERT(tt.getCellResolution().m_h == 15);
    }

    void testRegionElementToStr()
    {
        RegionElement region;
        region.parseAttribute("", "id", "regionX");
        region.addAttribute("origin", "1% 2%");
        region.addAttribute("extent", "3% 4%");
        std::string str = region.toStr();

        CPPUNIT_ASSERT(str.find("regionX") != std::string::npos);
        CPPUNIT_ASSERT(str.find("[1%, 2%, 3%, 4%]") != std::string::npos);
    }

    void testBodyElementParseAttributeDerivedUnknown()
    {
        BodyElement body;
        body.parseAttribute("", "space", "preserve");
        body.parseAttribute("", "style", "s1");
        body.parseAttribute("", "region", "r1");
        body.parseAttribute("", "backgroundImage", "#img1");

        body.parseAttribute("", "unknown", "value");

        CPPUNIT_ASSERT(body.getWhiteSpaceHandling() == XmlSpace::PRESERVE);
        CPPUNIT_ASSERT(body.getStyleId() == "s1");
        CPPUNIT_ASSERT(body.getRegionId() == "r1");
        CPPUNIT_ASSERT(body.getBackgroundImageId() == "img1");
        CPPUNIT_ASSERT(body.getTextLines().size() == 1);
    }

    void testTTElementParseAttributeDerivedCellResolutionInvalid()
    {
        TTElement tt;
        tt.parseAttribute("", "cellResolution", "bad value");
        // Should fall back to default
        CPPUNIT_ASSERT(tt.getCellResolution().m_w == 32);
        CPPUNIT_ASSERT(tt.getCellResolution().m_h == 15);
    }
};
// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( ElementsTest );
