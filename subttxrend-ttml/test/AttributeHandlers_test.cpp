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
#include "Parser/AttributeHandlers.hpp"

using namespace subttxrend::ttmlengine;

class AttributeHandlersTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( AttributeHandlersTest );
    CPPUNIT_TEST(merging);
    CPPUNIT_TEST(mergeAttributes_emptySource);
    CPPUNIT_TEST(mergeAttributes_overwrite);
    CPPUNIT_TEST(operatorStream_empty);
    CPPUNIT_TEST(operatorStream_nonEmpty);
    CPPUNIT_TEST(styleAttributeHandler_addAndGet);
    CPPUNIT_TEST(styleAttributeHandler_set);
    CPPUNIT_TEST(testStyleAttributeHandlerAddAttributeOverwrites);
    CPPUNIT_TEST(testStyleAttributeHandlerSetReplacesAll);
    CPPUNIT_TEST(testStyleAttributeHandlerGetStyleAttributesReturnsReference);
    CPPUNIT_TEST(testMergeAttributesEmptyDestination);
    CPPUNIT_TEST(testOperatorStreamSingleAttribute);
    CPPUNIT_TEST(testOperatorStreamSpecialChars);
    CPPUNIT_TEST(testStyleAttributeHandlerEmptyKeyValue);
    CPPUNIT_TEST(testStyleAttributeHandlerSpecialChars);
    CPPUNIT_TEST(testStyleAttributeHandlerSetEmpty);
    CPPUNIT_TEST(testStyleAttributeHandlerSetSelfAssign);
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

    void merging()
    {
        const auto someFontName_01 = "SomeFontName_01";

        Attributes dest = {
        };

        Attributes src_01 = {
                {"fontFamily", someFontName_01},
        };

        mergeAttributes(dest, src_01);
        CPPUNIT_ASSERT(dest.size() == 1);
        CPPUNIT_ASSERT(dest.count("fontFamily") == 1);
        auto search_01 = dest.find("fontFamily");
        CPPUNIT_ASSERT(search_01 != dest.end());
        CPPUNIT_ASSERT(search_01->second == someFontName_01);

        const auto someFontName_02 = "SomeFontName_02";
        Attributes src_02 = {
                {"fontFamily", someFontName_02},
        };

        mergeAttributes(dest, src_02);
        CPPUNIT_ASSERT(dest.size() == 1);
        CPPUNIT_ASSERT(dest.count("fontFamily") == 1);
        auto search_02 = dest.find("fontFamily");
        CPPUNIT_ASSERT(search_02 != dest.end());
        CPPUNIT_ASSERT(search_02->second == someFontName_02);

        const auto someFontName_03 = "SomeFontName_03";
        Attributes src_03 = {
                {"fontFamily", someFontName_03},
                {"color", "rgba(12,34,56,78)"},
                {"backgroundColor", "#ABCDEF12"},
                {"fontSize", "67.12%"},
        };

        mergeAttributes(dest, src_03);
        CPPUNIT_ASSERT(dest.size() == 4);
        CPPUNIT_ASSERT(dest.count("fontFamily") == 1);
        auto search_03 = dest.find("fontFamily");
        CPPUNIT_ASSERT(search_03 != dest.end());
        CPPUNIT_ASSERT(search_03->second == someFontName_03);


        StyleAttributeHandler styleAttrHndl;
        const auto someFontName_04 = "SomeFontName_04";
        styleAttrHndl.addAttribute("fontFamily", someFontName_01);
        styleAttrHndl.addAttribute("fontFamily", someFontName_02);
        styleAttrHndl.addAttribute("fontFamily", someFontName_03);
        styleAttrHndl.addAttribute("fontFamily", someFontName_04);


        mergeAttributes(dest, styleAttrHndl.getStyleAttributes());
        CPPUNIT_ASSERT(dest.size() == 4);
        CPPUNIT_ASSERT(dest.count("fontFamily") == 1);
        auto search_04 = dest.find("fontFamily");
        CPPUNIT_ASSERT(search_04 != dest.end());
        CPPUNIT_ASSERT(search_04->second == someFontName_04);
    }

    void mergeAttributes_emptySource()
    {
        Attributes dest = { {"a", "1"}, {"b", "2"} };
        Attributes src = {};
        mergeAttributes(dest, src);
        CPPUNIT_ASSERT(dest.size() == 2);
        CPPUNIT_ASSERT(dest.at("a") == "1");
        CPPUNIT_ASSERT(dest.at("b") == "2");
    }

    void mergeAttributes_overwrite()
    {
        Attributes dest = { {"a", "1"}, {"b", "2"} };
        Attributes src = { {"a", "3"}, {"c", "4"} };
        mergeAttributes(dest, src);
        CPPUNIT_ASSERT(dest.size() == 3);
        CPPUNIT_ASSERT(dest.at("a") == "3");
        CPPUNIT_ASSERT(dest.at("b") == "2");
        CPPUNIT_ASSERT(dest.at("c") == "4");
    }

    void operatorStream_empty()
    {
        Attributes attrSet = {};
        std::ostringstream oss;
        oss << attrSet;
        CPPUNIT_ASSERT(oss.str() == "");
    }

    void operatorStream_nonEmpty()
    {
        Attributes attrSet = { {"x", "10"}, {"y", "20"} };
        std::ostringstream oss;
        oss << attrSet;
        std::string out = oss.str();
        CPPUNIT_ASSERT(out.find("[x:10], ") != std::string::npos);
        CPPUNIT_ASSERT(out.find("[y:20], ") != std::string::npos);
    }

    void styleAttributeHandler_addAndGet()
    {
        StyleAttributeHandler handler;
        handler.addAttribute("foo", "bar");
        handler.addAttribute("foo", "baz");
        const Attributes& attrs = handler.getStyleAttributes();
        CPPUNIT_ASSERT(attrs.size() == 1);
        CPPUNIT_ASSERT(attrs.at("foo") == "baz");
    }

    void styleAttributeHandler_set()
    {
        StyleAttributeHandler handler;
        Attributes attrs = { {"a", "1"}, {"b", "2"} };
        handler.set(attrs);
        const Attributes& got = handler.getStyleAttributes();
        CPPUNIT_ASSERT(got.size() == 2);
        CPPUNIT_ASSERT(got.at("a") == "1");
        CPPUNIT_ASSERT(got.at("b") == "2");
    }

    // --- StyleAttributeHandler ---
    void testStyleAttributeHandlerAddAttributeOverwrites()
    {
        StyleAttributeHandler handler;
        handler.addAttribute("color", "red");
        handler.addAttribute("color", "blue");
        CPPUNIT_ASSERT(handler.getStyleAttributes().at("color") == "blue");
    }

    void testStyleAttributeHandlerSetReplacesAll()
    {
        StyleAttributeHandler handler;
        handler.addAttribute("color", "red");
        Attributes newAttrs = {{"font", "Arial"}};
        handler.set(newAttrs);
        CPPUNIT_ASSERT(handler.getStyleAttributes().size() == 1);
        CPPUNIT_ASSERT(handler.getStyleAttributes().at("font") == "Arial");
    }

    void testStyleAttributeHandlerGetStyleAttributesReturnsReference()
    {
        StyleAttributeHandler handler;
        handler.addAttribute("color", "red");
        const Attributes& ref = handler.getStyleAttributes();
        CPPUNIT_ASSERT(ref.at("color") == "red");
    }

    void testMergeAttributesEmptyDestination()
    {
        Attributes dest;
        Attributes src = { {"key1", "value1"}, {"key2", "value2"} };
        mergeAttributes(dest, src);
        CPPUNIT_ASSERT(dest.size() == 2);
        CPPUNIT_ASSERT(dest.at("key1") == "value1");
        CPPUNIT_ASSERT(dest.at("key2") == "value2");
    }

    void testOperatorStreamSingleAttribute()
    {
        Attributes attrSet = { {"single", "value"} };
        std::ostringstream oss;
        oss << attrSet;
        CPPUNIT_ASSERT(oss.str() == "[single:value], ");
    }

    void testOperatorStreamSpecialChars()
    {
        Attributes attrSet = { {"key:with:colon", "value,with,comma"}, {"key[bracket]", "value]bracket["} };
        std::ostringstream oss;
        oss << attrSet;
        std::string out = oss.str();
        CPPUNIT_ASSERT(out.find("[key:with:colon:value,with,comma], ") != std::string::npos);
        CPPUNIT_ASSERT(out.find("[key[bracket]:value]bracket[], ") != std::string::npos);
    }

    void testStyleAttributeHandlerEmptyKeyValue()
    {
        StyleAttributeHandler handler;
        handler.addAttribute("", "value");
        handler.addAttribute("key", "");
        const Attributes& attrs = handler.getStyleAttributes();
        CPPUNIT_ASSERT(attrs.at("") == "value");
        CPPUNIT_ASSERT(attrs.at("key") == "");
    }

    void testStyleAttributeHandlerSpecialChars()
    {
        StyleAttributeHandler handler;
        handler.addAttribute("key:with:colon", "value,with,comma");
        handler.addAttribute("key[bracket]", "value]bracket[");
        const Attributes& attrs = handler.getStyleAttributes();
        CPPUNIT_ASSERT(attrs.at("key:with:colon") == "value,with,comma");
        CPPUNIT_ASSERT(attrs.at("key[bracket]") == "value]bracket[");
    }

    void testStyleAttributeHandlerSetEmpty()
    {
        StyleAttributeHandler handler;
        handler.addAttribute("key", "value");
        Attributes emptyAttrs;
        handler.set(emptyAttrs);
        CPPUNIT_ASSERT(handler.getStyleAttributes().empty());
    }

    void testStyleAttributeHandlerSetSelfAssign()
    {
        StyleAttributeHandler handler;
        handler.addAttribute("key", "value");
        Attributes& current = const_cast<Attributes&>(handler.getStyleAttributes());
        handler.set(current);
        const Attributes& result = handler.getStyleAttributes();
        CPPUNIT_ASSERT(result.size() == 1);
        CPPUNIT_ASSERT(result.at("key") == "value");
    }

};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( AttributeHandlersTest );

