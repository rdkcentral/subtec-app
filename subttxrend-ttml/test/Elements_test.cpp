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
    CPPUNIT_TEST(defaultStyle);
    CPPUNIT_TEST(testElementParseAttributeSetsIdAndDerived);
    CPPUNIT_TEST(testElementExpectTextContentDefault);
    CPPUNIT_TEST(testStyleElementHandlerRegistration);
    CPPUNIT_TEST(testRegionElementDefaultConstructor);
    CPPUNIT_TEST(testRegionElementParameterizedConstructor);
    CPPUNIT_TEST(testRegionElementAddAttributeOriginExtent);
    CPPUNIT_TEST(testRegionElementAddAttributeDelegates);
    CPPUNIT_TEST(testRegionElementGetStyleId);
    CPPUNIT_TEST(testImageElementExpectTextContent);
    CPPUNIT_TEST(testImageElementAppendText);
    CPPUNIT_TEST(testImageElementIsSameImage);
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
    CPPUNIT_TEST(testStyleElementMergeAttributes);
    CPPUNIT_TEST(testRegionElementParseOriginExtentPercent);
    CPPUNIT_TEST(testRegionElementParseOriginExtentPixels);
    CPPUNIT_TEST(testRegionElementParseOriginExtentInvalid);
    CPPUNIT_TEST(testImageElementAppendMultipleChunks);
    CPPUNIT_TEST(testImageElementIsSameImageDifferentData);
    CPPUNIT_TEST(testBodyElementWhitespaceHandlingDefault);
    CPPUNIT_TEST(testBodyElementWhitespaceHandlingPreserve);
    CPPUNIT_TEST(testRegionElementToStr);
    CPPUNIT_TEST(testRegionElementAddAttributeInvalid);
    CPPUNIT_TEST(testBodyElementFinalizeAllLinesEmpty);
    CPPUNIT_TEST(testBodyElementParseAttributeDerivedUnknown);
    CPPUNIT_TEST(testTTElementParseAttributeDerivedCellResolutionInvalid);
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

    void defaultStyle()
    {
        StyleElement style{};
        style.parseAttribute("", "id", "style1");

        CPPUNIT_ASSERT(style.getId() == "style1");
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
    void testStyleElementHandlerRegistration()
    {
        StyleElement style;
        style.parseAttribute("tts", "color", "red");
        CPPUNIT_ASSERT(style.getStyleAttributes().at("color") == "red");
    }

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

    void testRegionElementAddAttributeOriginExtent()
    {
        RegionElement region;
        region.addAttribute("origin", "5% 10%");
        region.addAttribute("extent", "20% 30%");
        CPPUNIT_ASSERT(region.getX().getValue() == 500);
        CPPUNIT_ASSERT(region.getY().getValue() == 1000);
        CPPUNIT_ASSERT(region.getWidth().getValue() == 2000);
        CPPUNIT_ASSERT(region.getHeight().getValue() == 3000);
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

    void testImageElementAppendText()
    {
        ImageElement img;
        img.appendText("abc");
        CPPUNIT_ASSERT(*(img.getBase64Data()) == "abc");
    }

    void testImageElementIsSameImage()
    {
        auto img1 = std::make_shared<ImageElement>();
        auto img2 = std::make_shared<ImageElement>();
        img1->parseAttribute("", "id", "imgA");
        img2->parseAttribute("", "id", "imgA");
        img1->appendText("data");
        img2->appendText("data");
        CPPUNIT_ASSERT(img1->isSameImage(img2));
    }

    void testImageElementAppendMultipleChunks()
    {
        ImageElement img;
        img.appendText("abc");
        img.appendText("def");
        CPPUNIT_ASSERT(*(img.getBase64Data()) == "abcdef");
    }

    void testImageElementIsSameImageDifferentData()
    {
        auto img1 = std::make_shared<ImageElement>();
        auto img2 = std::make_shared<ImageElement>();
        img1->parseAttribute("", "id", "imgA");
        img2->parseAttribute("", "id", "imgA");
        img1->appendText("data1");
        img2->appendText("data2");
        CPPUNIT_ASSERT(!img1->isSameImage(img2));
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
        parent->parseAttribute("", "style", "parentStyle");
        BodyElement child(parent);
        CPPUNIT_ASSERT(child.getStyleId() == "parentStyle");
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
        CPPUNIT_ASSERT(body.hasValidContent());
        body.appendNewline();
        CPPUNIT_ASSERT(body.hasValidContent());
    }

    void testBodyElementFinalizeRemovesEmptyLines()
    {
        BodyElement body;
        body.appendText("");
        body.appendNewline();
        body.appendText("abc");
        body.finalize();
        for (const auto& line : body.getTextLines()) {
            CPPUNIT_ASSERT(!line.text.empty() || line.isForcedLine);
        }
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

    void testRegionElementAddAttributeInvalid()
    {
        RegionElement region;
        region.addAttribute("unknown", "value");
        // Should not throw, just ignored or added as style attribute
        CPPUNIT_ASSERT(region.getStyleAttributes().at("unknown") == "value");
    }

    void testBodyElementFinalizeAllLinesEmpty()
    {
        BodyElement body;
        body.appendText("");
        body.appendNewline();
        body.appendText("");
        body.finalize();
        for (const auto& line : body.getTextLines()) {
            CPPUNIT_ASSERT(!line.text.empty() || line.isForcedLine);
        }
    }

    void testBodyElementParseAttributeDerivedUnknown()
    {
        BodyElement body;
        body.parseAttribute("", "unknown", "value");
        // Should not throw or crash, just ignored
        CPPUNIT_ASSERT(true);
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

