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
#include "Parser/StyleSet.hpp"
#include "Parser/Outline.hpp"
#include <subttxrend/gfx/ColorArgb.hpp>

using namespace subttxrend::ttmlengine;

class StyleSetTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( StyleSetTest );
    CPPUNIT_TEST(checkDefaults);
    CPPUNIT_TEST(merging);
    CPPUNIT_TEST(testInvalidColorParsing);
    CPPUNIT_TEST(testTransparentBackgroundColor);
    CPPUNIT_TEST(testInvalidFontSizeAndLineHeight);
    CPPUNIT_TEST(testCellUnitParsing);
    CPPUNIT_TEST(testOutlineWithOnlyThickness);
    CPPUNIT_TEST(testOutlineWithInvalidColorOrThickness);
    CPPUNIT_TEST(testValidAlignParsing);
    CPPUNIT_TEST(testUnknownTextAlignAndDisplayAlign);
    CPPUNIT_TEST(testToStrOutput);
    CPPUNIT_TEST(testOperatorEquality);
    CPPUNIT_TEST(testSetAndGetStyleId);
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

    void checkDefaults()
    {
        StyleSet styleSet{};

        CPPUNIT_ASSERT(styleSet.getColor() == subttxrend::gfx::ColorArgb::WHITE);
        CPPUNIT_ASSERT(styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb::TRANSPARENT);
        CPPUNIT_ASSERT(styleSet.getFontFamily() == "default");
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::CELL_HUNDREDTHS, 100}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 100*100}));
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::CENTER);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::BEFORE);
        auto outline = Outline{DomainValue{DomainValue::Type::CELL_HUNDREDTHS, 0}, subttxrend::gfx::ColorArgb{subttxrend::gfx::ColorArgb::BLACK}};
        CPPUNIT_ASSERT((styleSet.getOutline() == outline));
    }

    void merging()
    {
        StyleSet styleSet{};

        const auto someFontName = "SomeFontName";

        Attributes oneAttribute = {
                {"fontFamily", someFontName},
        };

        styleSet.merge(oneAttribute);

        CPPUNIT_ASSERT(styleSet.getColor() == subttxrend::gfx::ColorArgb::WHITE);
        CPPUNIT_ASSERT(styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb::TRANSPARENT);
        CPPUNIT_ASSERT(styleSet.getFontFamily() == std::string{someFontName});
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::CELL_HUNDREDTHS, 100}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 100*100}));
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::CENTER);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::BEFORE);
        auto outline = Outline{DomainValue{DomainValue::Type::CELL_HUNDREDTHS, 0}, subttxrend::gfx::ColorArgb{subttxrend::gfx::ColorArgb::BLACK}};
        CPPUNIT_ASSERT((styleSet.getOutline() == outline));

        const auto totalyDifferentFontName = "totalyDifferentFontName";

        Attributes styleAttributes = {
                {"color", "rgba(12,34,56,78)"},
                {"backgroundColor", "#ABCDEF12"},
                {"fontFamily", totalyDifferentFontName},
                {"fontSize", "67.12%"},
                {"lineHeight", "90.98%"},
                {"textAlign", "end"},
                {"displayAlign", "after"},
                {"textOutline", "rgba(12,34,56,78) 67.12%"}
        };

        styleSet.merge(styleAttributes);

        CPPUNIT_ASSERT((styleSet.getColor() == subttxrend::gfx::ColorArgb{0x4E, 0x0C, 0x22, 0x38}));
        CPPUNIT_ASSERT((styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb{0x12, 0xAB, 0xCD, 0xEF}));
        CPPUNIT_ASSERT(styleSet.getFontFamily() == std::string{totalyDifferentFontName});
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 6712}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 9098}));
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::RIGHT);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::AFTER);
        outline = Outline{DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 6712}, subttxrend::gfx::ColorArgb{0x4E, 0x0C, 0x22, 0x38}};
        CPPUNIT_ASSERT((styleSet.getOutline() == outline));

        Attributes colorOnlyAttribute = {
                {"color", "rgba(255,255,255,255)"},
        };
        styleSet.merge(colorOnlyAttribute);

        CPPUNIT_ASSERT((styleSet.getColor() == subttxrend::gfx::ColorArgb{0xFF, 0xFF, 0xFF, 0xFF}));
        CPPUNIT_ASSERT((styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb{0x12, 0xAB, 0xCD, 0xEF}));
        CPPUNIT_ASSERT(styleSet.getFontFamily() == std::string{totalyDifferentFontName});
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 6712}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 9098}));
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::RIGHT);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::AFTER);
        CPPUNIT_ASSERT((styleSet.getOutline() == outline));

        Attributes yellowColorOnlyAttribute = {
                {"color", "YeLloW"},
        };
        styleSet.merge(yellowColorOnlyAttribute);

        CPPUNIT_ASSERT((styleSet.getColor() == subttxrend::gfx::ColorArgb::YELLOW));
        CPPUNIT_ASSERT((styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb{0x12, 0xAB, 0xCD, 0xEF}));
        CPPUNIT_ASSERT(styleSet.getFontFamily() == std::string{totalyDifferentFontName});
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 6712}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 9098}));
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::RIGHT);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::AFTER);
        CPPUNIT_ASSERT((styleSet.getOutline() == outline));

        Attributes greenBgColorOnlyAttribute = {
                {"backgroundColor", "rgba( 0 , 128 , 0 , 255 )"},
        };
        styleSet.merge(greenBgColorOnlyAttribute);

        CPPUNIT_ASSERT((styleSet.getColor() == subttxrend::gfx::ColorArgb::YELLOW));
        CPPUNIT_ASSERT((styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb::GREEN));
        CPPUNIT_ASSERT(styleSet.getFontFamily() == std::string{totalyDifferentFontName});
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 6712}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 9098}));
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::RIGHT);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::AFTER);
        CPPUNIT_ASSERT((styleSet.getOutline() == outline));

        Attributes invalidAttribute = {
                {"something", "something else"},
        };
        styleSet.merge(invalidAttribute);

        CPPUNIT_ASSERT((styleSet.getColor() == subttxrend::gfx::ColorArgb::YELLOW));
        CPPUNIT_ASSERT((styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb::GREEN));
        CPPUNIT_ASSERT(styleSet.getFontFamily() == std::string{totalyDifferentFontName});
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 6712}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 9098}));
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::RIGHT);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::AFTER);
        CPPUNIT_ASSERT((styleSet.getOutline() == outline));
    }

    void testInvalidColorParsing()
    {
        StyleSet styleSet;
        Attributes attrs = {
            {"color", "notacolor"},
            {"backgroundColor", "#GGHHII"},
        };
        styleSet.merge(attrs);
        CPPUNIT_ASSERT(styleSet.getColor() == subttxrend::gfx::ColorArgb::WHITE);
        CPPUNIT_ASSERT(styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb::TRANSPARENT);
    }

    void testTransparentBackgroundColor()
    {
        StyleSet styleSet;
        Attributes attrs = {
            {"backgroundColor", "rgba(0,0,0,0)"}
        };
        styleSet.merge(attrs);
        CPPUNIT_ASSERT(styleSet.getBackgroundColor() == subttxrend::gfx::ColorArgb::BLACK);
    }

    void testInvalidFontSizeAndLineHeight()
    {
        StyleSet styleSet;
        Attributes attrs = {
            {"fontSize", "abc"},
            {"lineHeight", "100percent"}
        };
        styleSet.merge(attrs);
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::CELL_HUNDREDTHS, 100}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 100*100}));
    }

    void testCellUnitParsing()
    {
        StyleSet styleSet;
        Attributes attrs = {
            {"fontSize", "0.80c"},
            {"lineHeight", "1.25c"}
        };
        styleSet.merge(attrs);
        CPPUNIT_ASSERT((styleSet.getFontSize() == DomainValue{DomainValue::Type::CELL_HUNDREDTHS, 80}));
        CPPUNIT_ASSERT((styleSet.getLineHeight() == DomainValue{DomainValue::Type::CELL_HUNDREDTHS, 125}));
    }

    void testOutlineWithOnlyThickness()
    {
        StyleSet styleSet;
        Attributes attrs = {
            {"textOutline", "2px"}
        };
        styleSet.merge(attrs);
        Outline expected{DomainValue{DomainValue::Type::PIXEL, 2}, subttxrend::gfx::ColorArgb::BLACK};
        CPPUNIT_ASSERT(styleSet.getOutline() == expected);
    }

    void testOutlineWithInvalidColorOrThickness()
    {
        StyleSet styleSet;
        Outline expected{DomainValue{DomainValue::Type::CELL_HUNDREDTHS, 0}, subttxrend::gfx::ColorArgb::BLACK};

        Attributes invalidColor = {
            {"textOutline", "notacolor 2px"}
        };
        styleSet.merge(invalidColor);
        CPPUNIT_ASSERT(styleSet.getOutline() == expected);

        Attributes invalidThickness = {
            {"textOutline", "rgba(0,0,0,255) notasize"}
        };
        styleSet.merge(invalidThickness);
        CPPUNIT_ASSERT(styleSet.getOutline() == expected);
    }

    void testValidAlignParsing()
    {
        StyleSet styleSet;

        styleSet.merge({{"textAlign", "left"}, {"displayAlign", "center"}});
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::LEFT);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::CENTER);

        styleSet.merge({{"textAlign", "center"}, {"displayAlign", "after"}});
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::CENTER);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::AFTER);
    }

    void testUnknownTextAlignAndDisplayAlign()
    {
        StyleSet styleSet;
        Attributes attrs = {
            {"textAlign", "foo"},
            {"displayAlign", "bar"}
        };
        styleSet.merge(attrs);
        CPPUNIT_ASSERT(styleSet.getTextAlign() == StyleSet::TextAlign::LEFT);
        CPPUNIT_ASSERT(styleSet.getDisplayAlign() == StyleSet::DisplayAlign::BEFORE);
    }

    void testToStrOutput()
    {
        StyleSet styleSet;
        styleSet.setStyleId("myStyle");
        styleSet.merge({
            {"fontFamily", "MyFont"},
            {"textAlign", "right"},
            {"displayAlign", "after"},
            {"textOutline", "2px"}
        });
        std::string str = styleSet.toStr();
        CPPUNIT_ASSERT(str.find("[myStyle]:") != std::string::npos);
        CPPUNIT_ASSERT(str.find("font:") != std::string::npos);
        CPPUNIT_ASSERT(str.find("bg:") != std::string::npos);
        CPPUNIT_ASSERT(str.find("MyFont") != std::string::npos);
        CPPUNIT_ASSERT(str.find("lineH:") != std::string::npos);
        CPPUNIT_ASSERT(str.find("text: right") != std::string::npos);
        CPPUNIT_ASSERT(str.find("display: after") != std::string::npos);
        CPPUNIT_ASSERT(str.find("outline:") != std::string::npos);
    }

    void testOperatorEquality()
    {
        StyleSet s1, s2;
        CPPUNIT_ASSERT(s1 == s2);

        s1.merge({{"color", "yellow"}});
        CPPUNIT_ASSERT(!(s1 == s2));
        s2.merge({{"color", "yellow"}});
        CPPUNIT_ASSERT(s1 == s2);

        s1.merge({{"backgroundColor", "#11223344"}});
        CPPUNIT_ASSERT(!(s1 == s2));
        s2.merge({{"backgroundColor", "#11223344"}});
        CPPUNIT_ASSERT(s1 == s2);

        s1.merge({{"fontFamily", "AltFont"}});
        CPPUNIT_ASSERT(!(s1 == s2));
        s2.merge({{"fontFamily", "AltFont"}});
        CPPUNIT_ASSERT(s1 == s2);

        s1.merge({{"fontSize", "55%"}});
        CPPUNIT_ASSERT(!(s1 == s2));
        s2.merge({{"fontSize", "55%"}});
        CPPUNIT_ASSERT(s1 == s2);

        s1.merge({{"textAlign", "right"}});
        CPPUNIT_ASSERT(!(s1 == s2));
        s2.merge({{"textAlign", "right"}});
        CPPUNIT_ASSERT(s1 == s2);

        s1.merge({{"displayAlign", "after"}});
        CPPUNIT_ASSERT(!(s1 == s2));
        s2.merge({{"displayAlign", "after"}});
        CPPUNIT_ASSERT(s1 == s2);

        s1.merge({{"lineHeight", "120%"}});
        CPPUNIT_ASSERT(!(s1 == s2));
        s2.merge({{"lineHeight", "120%"}});
        CPPUNIT_ASSERT(s1 == s2);

        s1.merge({{"textOutline", "rgba(0,0,0,255) 2px"}});
        CPPUNIT_ASSERT(!(s1 == s2));
        s2.merge({{"textOutline", "rgba(0,0,0,255) 2px"}});
        CPPUNIT_ASSERT(s1 == s2);

        s1.setStyleId("left-id");
        s2.setStyleId("right-id");
        CPPUNIT_ASSERT(s1 == s2);
    }

    void testSetAndGetStyleId()
    {
        StyleSet styleSet;
        styleSet.setStyleId("style42");
        CPPUNIT_ASSERT(styleSet.getStyleId() == "style42");
    }
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( StyleSetTest );

