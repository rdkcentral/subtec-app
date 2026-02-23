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
#include "CharsetHandler.hpp"
#include <ttxdecoder/Property.hpp>

using namespace subttxrend::ttxt;

class CharsetHandlerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CharsetHandlerTest);

    CPPUNIT_TEST(testDefaultConstructor);
    CPPUNIT_TEST(testCharA_WithGrave);
    CPPUNIT_TEST(testCharA_WithAcute);
    CPPUNIT_TEST(testCharA_WithCircumflex);
    CPPUNIT_TEST(testCharA_WithTilde);
    CPPUNIT_TEST(testCharA_WithMacron);
    CPPUNIT_TEST(testCharA_WithBreve);
    CPPUNIT_TEST(testCharA_WithUmlaut);
    CPPUNIT_TEST(testCharA_WithRing);
    CPPUNIT_TEST(testCharA_WithOgonek);
    CPPUNIT_TEST(testCharA_WithCaron);
    CPPUNIT_TEST(testCharA_WithDotAbove);
    CPPUNIT_TEST(testCharC_WithAcute);
    CPPUNIT_TEST(testCharC_WithCircumflex);
    CPPUNIT_TEST(testCharC_WithDotAbove);
    CPPUNIT_TEST(testCharC_WithCedilla);
    CPPUNIT_TEST(testCharC_WithCaron);
    CPPUNIT_TEST(testCharE_WithGrave);
    CPPUNIT_TEST(testCharE_WithAcute);
    CPPUNIT_TEST(testCharE_WithCircumflex);
    CPPUNIT_TEST(testCharE_WithMacron);
    CPPUNIT_TEST(testCharE_WithBreve);
    CPPUNIT_TEST(testCharE_WithDotAbove);
    CPPUNIT_TEST(testCharE_WithUmlaut);
    CPPUNIT_TEST(testCharE_WithOgonek);
    CPPUNIT_TEST(testCharE_WithCaron);
    CPPUNIT_TEST(testCharE_WithCedilla);
    CPPUNIT_TEST(testCharO_WithGrave);
    CPPUNIT_TEST(testCharO_WithAcute);
    CPPUNIT_TEST(testCharO_WithCircumflex);
    CPPUNIT_TEST(testCharO_WithTilde);
    CPPUNIT_TEST(testCharO_WithMacron);
    CPPUNIT_TEST(testCharO_WithBreve);
    CPPUNIT_TEST(testCharO_WithUmlaut);
    CPPUNIT_TEST(testCharO_WithDoubleAcute);
    CPPUNIT_TEST(testCharO_WithCaron);
    CPPUNIT_TEST(testCharO_WithOgonek);
    CPPUNIT_TEST(testCharO_WithDotAbove);
    CPPUNIT_TEST(testCharLowerA_WithGrave);
    CPPUNIT_TEST(testCharLowerA_WithAcute);
    CPPUNIT_TEST(testCharLowerA_WithRing);
    CPPUNIT_TEST(testCharB_WithAnyDiacritic_ReturnsUnchanged);
    CPPUNIT_TEST(testCharF_WithAnyDiacritic_ReturnsUnchanged);
    CPPUNIT_TEST(testCharM_WithAnyDiacritic_ReturnsUnchanged);
    CPPUNIT_TEST(testZeroCharacter_WithDiacritic_ReturnsUnchanged);
    CPPUNIT_TEST(testSpaceCharacter_WithDiacritic_ReturnsUnchanged);
    CPPUNIT_TEST(testDigitZero_WithDiacritic_ReturnsUnchanged);
    CPPUNIT_TEST(testCharG_WithAcute);
    CPPUNIT_TEST(testCharG_WithCircumflex);
    CPPUNIT_TEST(testCharG_WithBreve);
    CPPUNIT_TEST(testCharG_WithDotAbove);
    CPPUNIT_TEST(testCharG_WithCedilla);
    CPPUNIT_TEST(testCharG_WithCaron);
    CPPUNIT_TEST(testCharH_WithCircumflex);
    CPPUNIT_TEST(testCharH_WithCaron);
    CPPUNIT_TEST(testCharI_WithGrave);
    CPPUNIT_TEST(testCharI_WithAcute);
    CPPUNIT_TEST(testCharI_WithCircumflex);
    CPPUNIT_TEST(testCharI_WithMacron);
    CPPUNIT_TEST(testCharI_WithBreve);
    CPPUNIT_TEST(testCharI_WithDotAbove);
    CPPUNIT_TEST(testCharI_WithUmlaut);
    CPPUNIT_TEST(testCharI_WithOgonek);
    CPPUNIT_TEST(testCharI_WithCaron);
    CPPUNIT_TEST(testCharI_WithTilde);
    CPPUNIT_TEST(testCharJ_WithCircumflex);
    CPPUNIT_TEST(testCharK_WithCedilla);
    CPPUNIT_TEST(testCharK_WithCaron);
    CPPUNIT_TEST(testCharL_WithAcute);
    CPPUNIT_TEST(testCharL_WithCedilla);
    CPPUNIT_TEST(testCharL_WithCaron);
    CPPUNIT_TEST(testCharN_WithGrave);
    CPPUNIT_TEST(testCharN_WithAcute);
    CPPUNIT_TEST(testCharN_WithTilde);
    CPPUNIT_TEST(testCharN_WithCedilla);
    CPPUNIT_TEST(testCharN_WithCaron);
    CPPUNIT_TEST(testCharR_WithAcute);
    CPPUNIT_TEST(testCharR_WithCedilla);
    CPPUNIT_TEST(testCharR_WithCaron);
    CPPUNIT_TEST(testCharS_WithAcute);
    CPPUNIT_TEST(testCharS_WithCircumflex);
    CPPUNIT_TEST(testCharS_WithCedilla);
    CPPUNIT_TEST(testCharS_WithCaron);
    CPPUNIT_TEST(testCharT_WithCedilla);
    CPPUNIT_TEST(testCharT_WithCaron);
    CPPUNIT_TEST(testCharU_WithGrave);
    CPPUNIT_TEST(testCharU_WithAcute);
    CPPUNIT_TEST(testCharU_WithCircumflex);
    CPPUNIT_TEST(testCharU_WithTilde);
    CPPUNIT_TEST(testCharU_WithMacron);
    CPPUNIT_TEST(testCharU_WithUmlaut);
    CPPUNIT_TEST(testCharU_WithRing);
    CPPUNIT_TEST(testCharU_WithDoubleAcute);
    CPPUNIT_TEST(testCharU_WithOgonek);
    CPPUNIT_TEST(testCharU_WithCaron);
    CPPUNIT_TEST(testCharU_WithBreve);
    CPPUNIT_TEST(testCharW_WithAcute);
    CPPUNIT_TEST(testCharW_WithCircumflex);
    CPPUNIT_TEST(testCharW_WithGrave);
    CPPUNIT_TEST(testCharW_WithUmlaut);
    CPPUNIT_TEST(testCharY_WithGrave);
    CPPUNIT_TEST(testCharY_WithAcute);
    CPPUNIT_TEST(testCharY_WithCircumflex);
    CPPUNIT_TEST(testCharY_WithTilde);
    CPPUNIT_TEST(testCharY_WithUmlaut);
    CPPUNIT_TEST(testCharY_WithMacron);
    CPPUNIT_TEST(testCharZ_WithAcute);
    CPPUNIT_TEST(testCharZ_WithDotAbove);
    CPPUNIT_TEST(testCharZ_WithCaron);
    CPPUNIT_TEST(testCharLowerC_WithAcute);
    CPPUNIT_TEST(testCharLowerC_WithCircumflex);
    CPPUNIT_TEST(testCharLowerC_WithDotAbove);
    CPPUNIT_TEST(testCharLowerC_WithCedilla);
    CPPUNIT_TEST(testCharLowerC_WithCaron);
    CPPUNIT_TEST(testCharLowerD_WithCaron);
    CPPUNIT_TEST(testCharLowerE_WithGrave);
    CPPUNIT_TEST(testCharLowerE_WithAcute);
    CPPUNIT_TEST(testCharLowerE_WithCircumflex);
    CPPUNIT_TEST(testCharLowerE_WithMacron);
    CPPUNIT_TEST(testCharLowerE_WithBreve);
    CPPUNIT_TEST(testCharLowerE_WithDotAbove);
    CPPUNIT_TEST(testCharLowerE_WithUmlaut);
    CPPUNIT_TEST(testCharLowerE_WithOgonek);
    CPPUNIT_TEST(testCharLowerE_WithCaron);
    CPPUNIT_TEST(testCharLowerE_WithCedilla);
    CPPUNIT_TEST(testCharLowerG_WithAcute);
    CPPUNIT_TEST(testCharLowerG_WithCircumflex);
    CPPUNIT_TEST(testCharLowerG_WithBreve);
    CPPUNIT_TEST(testCharLowerG_WithDotAbove);
    CPPUNIT_TEST(testCharLowerG_WithCaron);
    CPPUNIT_TEST(testCharLowerG_WithCedilla);
    CPPUNIT_TEST(testCharLowerH_WithCircumflex);
    CPPUNIT_TEST(testCharLowerH_WithCaron);
    CPPUNIT_TEST(testCharLowerI_WithCircumflex);
    CPPUNIT_TEST(testCharLowerI_WithTilde);
    CPPUNIT_TEST(testCharLowerI_WithMacron);
    CPPUNIT_TEST(testCharLowerI_WithBreve);
    CPPUNIT_TEST(testCharLowerI_WithUmlaut);
    CPPUNIT_TEST(testCharLowerI_WithCaron);
    CPPUNIT_TEST(testCharLowerI_WithOgonek);
    CPPUNIT_TEST(testCharLowerJ_WithCircumflex);
    CPPUNIT_TEST(testCharLowerK_WithCedilla);
    CPPUNIT_TEST(testCharLowerK_WithCaron);
    CPPUNIT_TEST(testCharLowerL_WithAcute);
    CPPUNIT_TEST(testCharLowerL_WithCedilla);
    CPPUNIT_TEST(testCharLowerL_WithCaron);
    CPPUNIT_TEST(testCharA_WithDoubleAcute_ReturnsUnchanged);
    CPPUNIT_TEST(testCharD_WithAcute_ReturnsUnchanged);
    CPPUNIT_TEST(testCharP_WithAnyDiacritic_ReturnsUnchanged);
    CPPUNIT_TEST(testCharQ_WithAnyDiacritic_ReturnsUnchanged);
    CPPUNIT_TEST(testCharLowerN_WithGrave);
    CPPUNIT_TEST(testCharLowerN_WithAcute);
    CPPUNIT_TEST(testCharLowerN_WithTilde);
    CPPUNIT_TEST(testCharLowerN_WithCedilla);
    CPPUNIT_TEST(testCharLowerN_WithCaron);
    CPPUNIT_TEST(testCharLowerO_WithGrave);
    CPPUNIT_TEST(testCharLowerO_WithAcute);
    CPPUNIT_TEST(testCharLowerO_WithCircumflex);
    CPPUNIT_TEST(testCharLowerO_WithTilde);
    CPPUNIT_TEST(testCharLowerO_WithUmlaut);
    CPPUNIT_TEST(testCharLowerO_WithDoubleAcute);
    CPPUNIT_TEST(testCharLowerR_WithAcute);
    CPPUNIT_TEST(testCharLowerR_WithCedilla);
    CPPUNIT_TEST(testCharLowerR_WithCaron);
    CPPUNIT_TEST(testCharLowerS_WithAcute);
    CPPUNIT_TEST(testCharLowerS_WithCircumflex);
    CPPUNIT_TEST(testCharLowerS_WithCedilla);
    CPPUNIT_TEST(testCharLowerS_WithCaron);
    CPPUNIT_TEST(testCharLowerT_WithCedilla);
    CPPUNIT_TEST(testCharLowerT_WithCaron);
    CPPUNIT_TEST(testCharLowerU_WithGrave);
    CPPUNIT_TEST(testCharLowerU_WithAcute);
    CPPUNIT_TEST(testCharLowerU_WithCircumflex);
    CPPUNIT_TEST(testCharLowerU_WithUmlaut);
    CPPUNIT_TEST(testCharLowerU_WithRing);
    CPPUNIT_TEST(testCharLowerU_WithDoubleAcute);
    CPPUNIT_TEST(testCharLowerY_WithAcute);
    CPPUNIT_TEST(testCharLowerY_WithUmlaut);
    CPPUNIT_TEST(testCharLowerZ_WithAcute);
    CPPUNIT_TEST(testCharLowerZ_WithCaron);
    CPPUNIT_TEST(testCharLowerZ_WithDotAbove);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
    }

    void tearDown()
    {
    }

protected:
    void testDefaultConstructor()
    {
        CharsetHandler handler;
        // Constructor should complete without exceptions
        // No meaningful state to verify for default constructor
        CPPUNIT_ASSERT(true);
    }

    void testCharA_WithGrave()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0041, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00C0), result); // À
    }

    void testCharA_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0041, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00C1), result); // Á
    }

    void testCharA_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0041, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00C2), result); // Â
    }

    void testCharA_WithTilde()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0041, ttxdecoder::Property::VALUE_DIACRITIC_TILDE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00C3), result); // Ã
    }

    void testCharA_WithMacron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0041, ttxdecoder::Property::VALUE_DIACRITIC_MACRON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0100), result); // Ā
    }

    void testCharA_WithBreve()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0041, ttxdecoder::Property::VALUE_DIACRITIC_BREVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0102), result); // Ă
    }

    void testCharA_WithUmlaut()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0041, ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00C4), result); // Ä
    }

    void testCharA_WithRing()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0041, ttxdecoder::Property::VALUE_DIACRITIC_RING);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00C5), result); // Å
    }

    void testCharA_WithOgonek()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0041, ttxdecoder::Property::VALUE_DIACRITIC_OGONEK);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0104), result); // Ą
    }

    void testCharA_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0041, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x01CD), result); // Ǎ
    }

    void testCharA_WithDotAbove()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0041, ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0226), result); // Ȧ
    }

    void testCharC_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0043, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0106), result); // Ć
    }

    void testCharC_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0043, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0108), result); // Ĉ
    }

    void testCharC_WithDotAbove()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0043, ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x010A), result); // Ċ
    }

    void testCharC_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0043, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00C7), result); // Ç
    }

    void testCharC_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0043, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x010C), result); // Č
    }

    void testCharD_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0044, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x010E), result); // Ď
    }

    void testCharE_WithGrave()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0045, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00C8), result); // È
    }

    void testCharE_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0045, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00C9), result); // É
    }

    void testCharE_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0045, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00CA), result); // Ê
    }

    void testCharE_WithMacron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0045, ttxdecoder::Property::VALUE_DIACRITIC_MACRON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0112), result); // Ē
    }

    void testCharE_WithBreve()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0045, ttxdecoder::Property::VALUE_DIACRITIC_BREVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0114), result); // Ĕ
    }

    void testCharE_WithDotAbove()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0045, ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0116), result); // Ė
    }

    void testCharE_WithUmlaut()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0045, ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00CB), result); // Ë
    }

    void testCharE_WithOgonek()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0045, ttxdecoder::Property::VALUE_DIACRITIC_OGONEK);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0118), result); // Ę
    }

    void testCharE_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0045, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x011A), result); // Ě
    }

    void testCharE_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0045, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0228), result); // Ȩ
    }

    void testCharO_WithGrave()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004F, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00D2), result); // Ò
    }

    void testCharO_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004F, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00D3), result); // Ó
    }

    void testCharO_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004F, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00D4), result); // Ô
    }

    void testCharO_WithTilde()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004F, ttxdecoder::Property::VALUE_DIACRITIC_TILDE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00D5), result); // Õ
    }

    void testCharO_WithMacron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004F, ttxdecoder::Property::VALUE_DIACRITIC_MACRON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x014C), result); // Ō
    }

    void testCharO_WithBreve()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004F, ttxdecoder::Property::VALUE_DIACRITIC_BREVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x014E), result); // Ŏ
    }

    void testCharO_WithUmlaut()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004F, ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00D6), result); // Ö
    }

    void testCharO_WithDoubleAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004F, ttxdecoder::Property::VALUE_DIACRITIC_DOUBLE_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0150), result); // Ő
    }

    void testCharO_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004F, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x01D1), result); // Ǒ
    }

    void testCharO_WithOgonek()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004F, ttxdecoder::Property::VALUE_DIACRITIC_OGONEK);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x01EA), result); // Ǫ
    }

    void testCharO_WithDotAbove()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004F, ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x022E), result); // Ȯ
    }

    void testCharLowerA_WithGrave()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0061, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00E0), result); // à
    }

    void testCharLowerA_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0061, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00E1), result); // á
    }

    void testCharLowerA_WithRing()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0061, ttxdecoder::Property::VALUE_DIACRITIC_RING);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00E5), result); // å
    }

    void testCharB_WithAnyDiacritic_ReturnsUnchanged()
    {
        // Character 'B' has no diacritic support
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0042, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0042), result); // B unchanged
    }

    void testCharF_WithAnyDiacritic_ReturnsUnchanged()
    {
        // Character 'F' has no diacritic support
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0046, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0046), result); // F unchanged
    }

    void testCharM_WithAnyDiacritic_ReturnsUnchanged()
    {
        // Character 'M' has no diacritic support
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004D, ttxdecoder::Property::VALUE_DIACRITIC_TILDE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x004D), result); // M unchanged
    }

    void testZeroCharacter_WithDiacritic_ReturnsUnchanged()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0000, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0000), result);
    }

    void testSpaceCharacter_WithDiacritic_ReturnsUnchanged()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0020, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0020), result);
    }

    void testDigitZero_WithDiacritic_ReturnsUnchanged()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0030, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0030), result);
    }

    void testCharG_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0047, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x01F4), result); // Ǵ
    }

    void testCharG_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0047, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x011C), result); // Ĝ
    }

    void testCharG_WithBreve()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0047, ttxdecoder::Property::VALUE_DIACRITIC_BREVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x011E), result); // Ğ
    }

    void testCharG_WithDotAbove()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0047, ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0120), result); // Ġ
    }

    void testCharG_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0047, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0122), result); // Ģ
    }

    void testCharG_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0047, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x01E6), result); // Ǧ
    }

    void testCharH_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0048, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0124), result); // Ĥ
    }

    void testCharH_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0048, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x021E), result); // Ȟ
    }

    void testCharI_WithGrave()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0049, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00CC), result); // Ì
    }

    void testCharI_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0049, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00CD), result); // Í
    }

    void testCharI_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0049, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00CE), result); // Î
    }

    void testCharI_WithMacron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0049, ttxdecoder::Property::VALUE_DIACRITIC_MACRON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x012A), result); // Ī
    }

    void testCharI_WithBreve()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0049, ttxdecoder::Property::VALUE_DIACRITIC_BREVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x012C), result); // Ĭ
    }

    void testCharI_WithDotAbove()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0049, ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0130), result); // İ
    }

    void testCharI_WithUmlaut()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0049, ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00CF), result); // Ï
    }

    void testCharI_WithOgonek()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0049, ttxdecoder::Property::VALUE_DIACRITIC_OGONEK);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x012E), result); // Į
    }

    void testCharI_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0049, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x01CF), result); // Ǐ
    }

    void testCharI_WithTilde()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0049, ttxdecoder::Property::VALUE_DIACRITIC_TILDE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0128), result); // Ĩ
    }

    void testCharJ_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004A, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0134), result); // Ĵ
    }

    void testCharK_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004B, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0136), result); // Ķ
    }

    void testCharK_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004B, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x01E8), result); // Ǩ
    }

    void testCharL_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004C, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0139), result); // Ĺ
    }

    void testCharL_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004C, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x013B), result); // Ļ
    }

    void testCharL_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004C, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x013D), result); // Ľ
    }

    void testCharN_WithGrave()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004E, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x01F8), result); // Ǹ
    }

    void testCharN_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004E, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0143), result); // Ń
    }

    void testCharN_WithTilde()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004E, ttxdecoder::Property::VALUE_DIACRITIC_TILDE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00D1), result); // Ñ
    }

    void testCharN_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004E, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0145), result); // Ņ
    }

    void testCharN_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x004E, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0147), result); // Ň
    }

    void testCharR_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0052, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0154), result); // Ŕ
    }

    void testCharR_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0052, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0156), result); // Ŗ
    }

    void testCharR_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0052, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0158), result); // Ř
    }

    void testCharS_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0053, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x015A), result); // Ś
    }

    void testCharS_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0053, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x015C), result); // Ŝ
    }

    void testCharS_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0053, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x015E), result); // Ş
    }

    void testCharS_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0053, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0160), result); // Š
    }

    void testCharT_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0054, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0162), result); // Ţ
    }

    void testCharT_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0054, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0164), result); // Ť
    }

    void testCharU_WithGrave()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0055, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00D9), result); // Ù
    }

    void testCharU_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0055, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00DA), result); // Ú
    }

    void testCharU_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0055, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00DB), result); // Û
    }

    void testCharU_WithTilde()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0055, ttxdecoder::Property::VALUE_DIACRITIC_TILDE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0168), result); // Ũ
    }

    void testCharU_WithMacron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0055, ttxdecoder::Property::VALUE_DIACRITIC_MACRON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x016A), result); // Ū
    }

    void testCharU_WithUmlaut()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0055, ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00DC), result); // Ü
    }

    void testCharU_WithRing()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0055, ttxdecoder::Property::VALUE_DIACRITIC_RING);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x016E), result); // Ů
    }

    void testCharU_WithDoubleAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0055, ttxdecoder::Property::VALUE_DIACRITIC_DOUBLE_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0170), result); // Ű
    }

    void testCharU_WithOgonek()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0055, ttxdecoder::Property::VALUE_DIACRITIC_OGONEK);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0172), result); // Ų
    }

    void testCharU_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0055, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x01D3), result); // Ǔ
    }

    void testCharU_WithBreve()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0055, ttxdecoder::Property::VALUE_DIACRITIC_BREVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x016C), result); // Ŭ
    }

    void testCharW_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0057, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x1E82), result); // Ẃ
    }

    void testCharW_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0057, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0174), result); // Ŵ
    }

    void testCharW_WithGrave()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0057, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x1E80), result); // Ẁ
    }

    void testCharW_WithUmlaut()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0057, ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x1E84), result); // Ẅ
    }

    void testCharY_WithGrave()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0059, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x1EF2), result); // Ỳ
    }

    void testCharY_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0059, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00DD), result); // Ý
    }

    void testCharY_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0059, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0176), result); // Ŷ
    }

    void testCharY_WithTilde()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0059, ttxdecoder::Property::VALUE_DIACRITIC_TILDE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x1EF8), result); // Ỹ
    }

    void testCharY_WithUmlaut()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0059, ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0178), result); // Ÿ
    }

    void testCharY_WithMacron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0059, ttxdecoder::Property::VALUE_DIACRITIC_MACRON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0232), result); // Ȳ
    }

    void testCharZ_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x005A, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0179), result); // Ź
    }

    void testCharZ_WithDotAbove()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x005A, ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x017B), result); // Ż
    }

    void testCharZ_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x005A, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x017D), result); // Ž
    }

    void testCharLowerC_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0063, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0107), result); // ć
    }

    void testCharLowerC_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0063, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0109), result); // ĉ
    }

    void testCharLowerC_WithDotAbove()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0063, ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x010B), result); // ċ
    }

    void testCharLowerC_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0063, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00E7), result); // ç
    }

    void testCharLowerC_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0063, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x010D), result); // č
    }

    void testCharLowerD_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0064, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x010F), result); // ď
    }

    void testCharLowerE_WithGrave()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0065, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00E8), result); // è
    }

    void testCharLowerE_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0065, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00E9), result); // é
    }

    void testCharLowerE_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0065, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00EA), result); // ê
    }

    void testCharLowerE_WithMacron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0065, ttxdecoder::Property::VALUE_DIACRITIC_MACRON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0113), result); // ē
    }

    void testCharLowerE_WithBreve()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0065, ttxdecoder::Property::VALUE_DIACRITIC_BREVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0115), result); // ĕ
    }

    void testCharLowerE_WithDotAbove()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0065, ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0117), result); // ė
    }

    void testCharLowerE_WithUmlaut()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0065, ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00EB), result); // ë
    }

    void testCharLowerE_WithOgonek()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0065, ttxdecoder::Property::VALUE_DIACRITIC_OGONEK);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0119), result); // ę
    }

    void testCharLowerE_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0065, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x011B), result); // ě
    }

    void testCharLowerE_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0065, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0229), result); // ȩ
    }

    void testCharLowerG_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0067, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x01F5), result); // ǵ
    }

    void testCharLowerG_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0067, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x011D), result); // ĝ
    }

    void testCharLowerG_WithBreve()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0067, ttxdecoder::Property::VALUE_DIACRITIC_BREVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x011F), result); // ğ
    }

    void testCharLowerG_WithDotAbove()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0067, ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0121), result); // ġ
    }

    void testCharLowerG_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0067, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x01E7), result); // ǧ
    }

    void testCharLowerG_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0067, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0123), result); // ģ
    }

    void testCharLowerH_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0068, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0125), result); // ĥ
    }

    void testCharLowerH_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0068, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x021F), result); // ȟ
    }

    void testCharLowerI_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0069, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00EE), result); // î
    }

    void testCharLowerI_WithTilde()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0069, ttxdecoder::Property::VALUE_DIACRITIC_TILDE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0129), result); // ĩ
    }

    void testCharLowerI_WithMacron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0069, ttxdecoder::Property::VALUE_DIACRITIC_MACRON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x012B), result); // ī
    }

    void testCharLowerI_WithBreve()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0069, ttxdecoder::Property::VALUE_DIACRITIC_BREVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x012D), result); // ĭ
    }

    void testCharLowerI_WithUmlaut()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0069, ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00EF), result); // ï
    }

    void testCharLowerI_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0069, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x01D0), result); // ǐ
    }

    void testCharLowerI_WithOgonek()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0069, ttxdecoder::Property::VALUE_DIACRITIC_OGONEK);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x012F), result); // į
    }

    void testCharLowerJ_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006A, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0135), result); // ĵ
    }

    void testCharLowerK_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006B, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0137), result); // ķ
    }

    void testCharLowerK_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006B, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x01E9), result); // ǩ
    }

    void testCharLowerL_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006C, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x013A), result); // ĺ
    }

    void testCharLowerL_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006C, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x013C), result); // ļ
    }

    void testCharLowerL_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006C, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x013E), result); // ľ
    }

    void testCharA_WithDoubleAcute_ReturnsUnchanged()
    {
        // Character 'A' does not support DOUBLE_ACUTE (only O and U support it)
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0041, ttxdecoder::Property::VALUE_DIACRITIC_DOUBLE_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0041), result); // A unchanged
    }

    void testCharD_WithAcute_ReturnsUnchanged()
    {
        // Character 'D' does not support ACUTE (only supports CARON)
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0044, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0044), result); // D unchanged
    }

    void testCharP_WithAnyDiacritic_ReturnsUnchanged()
    {
        // Character 'P' has no diacritic support
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0050, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0050), result); // P unchanged
    }

    void testCharQ_WithAnyDiacritic_ReturnsUnchanged()
    {
        // Character 'Q' has no diacritic support
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0051, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0051), result); // Q unchanged
    }

    void testCharLowerN_WithGrave()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006E, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x01F9), result); // ǹ
    }

    void testCharLowerN_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006E, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0144), result); // ń
    }

    void testCharLowerN_WithTilde()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006E, ttxdecoder::Property::VALUE_DIACRITIC_TILDE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00F1), result); // ñ
    }

    void testCharLowerN_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006E, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0146), result); // ņ
    }

    void testCharLowerN_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006E, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0148), result); // ň
    }

    void testCharLowerO_WithGrave()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006F, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00F2), result); // ò
    }

    void testCharLowerO_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006F, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00F3), result); // ó
    }

    void testCharLowerO_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006F, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00F4), result); // ô
    }

    void testCharLowerO_WithTilde()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006F, ttxdecoder::Property::VALUE_DIACRITIC_TILDE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00F5), result); // õ
    }

    void testCharLowerO_WithUmlaut()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006F, ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00F6), result); // ö
    }

    void testCharLowerO_WithDoubleAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x006F, ttxdecoder::Property::VALUE_DIACRITIC_DOUBLE_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0151), result); // ő
    }

    void testCharLowerR_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0072, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0155), result); // ŕ
    }

    void testCharLowerR_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0072, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0157), result); // ŗ
    }

    void testCharLowerR_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0072, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0159), result); // ř
    }

    void testCharLowerS_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0073, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x015B), result); // ś
    }

    void testCharLowerS_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0073, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x015D), result); // ŝ
    }

    void testCharLowerS_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0073, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x015F), result); // ş
    }

    void testCharLowerS_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0073, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0161), result); // š
    }

    void testCharLowerT_WithCedilla()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0074, ttxdecoder::Property::VALUE_DIACRITIC_CEDILLA);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0163), result); // ţ
    }

    void testCharLowerT_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0074, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0165), result); // ť
    }

    void testCharLowerU_WithGrave()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0075, ttxdecoder::Property::VALUE_DIACRITIC_GRAVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00F9), result); // ù
    }

    void testCharLowerU_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0075, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00FA), result); // ú
    }

    void testCharLowerU_WithCircumflex()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0075, ttxdecoder::Property::VALUE_DIACRITIC_CIRCUMFLEX);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00FB), result); // û
    }

    void testCharLowerU_WithUmlaut()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0075, ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00FC), result); // ü
    }

    void testCharLowerU_WithRing()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0075, ttxdecoder::Property::VALUE_DIACRITIC_RING);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x016F), result); // ů
    }

    void testCharLowerU_WithDoubleAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0075, ttxdecoder::Property::VALUE_DIACRITIC_DOUBLE_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x0171), result); // ű
    }

    void testCharLowerY_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0079, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00FD), result); // ý
    }

    void testCharLowerY_WithUmlaut()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x0079, ttxdecoder::Property::VALUE_DIACRITIC_UMLAUT);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x00FF), result); // ÿ
    }

    void testCharLowerZ_WithAcute()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x007A, ttxdecoder::Property::VALUE_DIACRITIC_ACUTE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x017A), result); // ź
    }

    void testCharLowerZ_WithCaron()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x007A, ttxdecoder::Property::VALUE_DIACRITIC_CARON);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x017E), result); // ž
    }

    void testCharLowerZ_WithDotAbove()
    {
        std::uint16_t result = CharsetHandler::getDiacriticCharacterCode(
            0x007A, ttxdecoder::Property::VALUE_DIACRITIC_DOT_ABOVE);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0x017C), result); // ż
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CharsetHandlerTest);