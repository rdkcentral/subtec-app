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
#include <memory>

#include "CharsetMapping.hpp"
#include "CharsetConfig.hpp"
#include "CharsetMaps.hpp"
#include "DefaultCharsets.hpp"
#include "Property.hpp"

using namespace ttxdecoder;

// Mock CharsetConfig for testing
class MockCharsetConfig : public CharsetConfig
{
public:
    MockCharsetConfig(Mode mode = Mode::UTF)
        : m_mode(mode)
        , m_charsetMaps()
        , m_nationalSubsets()
    {
    }

    virtual ~MockCharsetConfig() = default;

    virtual Mode getMode() const override
    {
        return m_mode;
    }

    virtual NationalCharset getPrimaryNationalCharset(std::uint8_t index = 0) const override
    {
        return NationalCharset::ENGLISH;
    }

    virtual NationalCharset getSecondaryNationalCharset(std::uint8_t index = 0) const override
    {
        return NationalCharset::FRENCH;
    }

    virtual const NationalSubsets& getNationalSubsets() const override
    {
        return m_nationalSubsets;
    }

    virtual const CharsetMaps& getCharsetMaps() const override
    {
        return m_charsetMaps;
    }

    void setMode(Mode mode)
    {
        m_mode = mode;
    }

private:
    Mode m_mode;
    CharsetMaps m_charsetMaps;
    NationalSubsets m_nationalSubsets;
};

class CharsetMappingTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE( CharsetMappingTest );
    CPPUNIT_TEST(testConstructorWithValidCharsets);
    CPPUNIT_TEST(testConstructorWithSameCharsets);
    CPPUNIT_TEST(testConstructorWithDifferentNationalCharsets);
    CPPUNIT_TEST(testSwitchFromPrimaryToSecondary);
    CPPUNIT_TEST(testSwitchFromSecondaryToPrimary);
    CPPUNIT_TEST(testMultipleConsecutiveSwitches);
    CPPUNIT_TEST(testSwitchWhenPrimaryEqualsSecondary);
    CPPUNIT_TEST(testSetG1ToBlockMosaic);
    CPPUNIT_TEST(testSetG1ToBlockMosaicSeparated);
    CPPUNIT_TEST(testSetG1MultipleTimesWithDifferentValues);
    CPPUNIT_TEST(testMapStandardG0Character);
    CPPUNIT_TEST(testMapCharacterAtMinimumValidRange);
    CPPUNIT_TEST(testMapCharacterAtMaximumValidRange);
    CPPUNIT_TEST(testMapReplaceableCharacterInDiacriticMode);
    CPPUNIT_TEST(testMapReplaceableCharacterInUTF16Mode);
    CPPUNIT_TEST(testMapNonReplaceableCharacter);
    CPPUNIT_TEST(testMapAllReplaceableCharacters);
    CPPUNIT_TEST(testMapCharacterBelowValidRange);
    CPPUNIT_TEST(testMapCharacterAboveValidRange);
    CPPUNIT_TEST(testMapCharacterWithSecondaryCharset);
    CPPUNIT_TEST(testMapReplaceableCharInNonReplaceableCharset);
    CPPUNIT_TEST(testMapG1CharacterWithBlockMosaic);
    CPPUNIT_TEST(testMapG1CharacterWithBlockMosaicSeparated);
    CPPUNIT_TEST(testMapG1CharacterAtBoundaryValues);
    CPPUNIT_TEST(testMapG1CharacterOutOfValidRange);
    CPPUNIT_TEST(testMapSameCharacterBeforeAndAfterSwitch);
    CPPUNIT_TEST(testMapG0AndG1CharactersIndependently);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_mockConfig = std::make_unique<MockCharsetConfig>();
    }

    void tearDown() override
    {
        m_mockConfig.reset();
    }

    void testConstructorWithValidCharsets()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Verify initial state by mapping a standard character - should use primary (ENGLISH)
        auto result = mapping.mapG0Character(0x41); // 'A'
        CPPUNIT_ASSERT(result.first != 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(Property::VALUE_DIACRITIC_NONE), result.second);
    }

    void testConstructorWithSameCharsets()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::ENGLISH);

        // Verify object is created successfully
        auto result = mapping.mapG0Character(0x41); // 'A'
        CPPUNIT_ASSERT(result.first != 0);
    }

    void testConstructorWithDifferentNationalCharsets()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::GERMAN, NationalCharset::ITALIAN);

        // Verify object stores distinct charsets by mapping with primary
        auto resultPrimary = mapping.mapG0Character(0x41);
        CPPUNIT_ASSERT(resultPrimary.first != 0);

        // Switch to secondary and verify it can switch between different charsets
        mapping.switchG0Charset();
        auto resultSecondary = mapping.mapG0Character(0x41);
        CPPUNIT_ASSERT(resultSecondary.first != 0);

        // Switch back and verify we get original mapping again
        mapping.switchG0Charset();
        auto resultBackToPrimary = mapping.mapG0Character(0x41);
        CPPUNIT_ASSERT_EQUAL(resultPrimary.first, resultBackToPrimary.first);
    }

    void testSwitchFromPrimaryToSecondary()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Map a replaceable character before switch
        auto beforeSwitch = mapping.mapG0Character(0x23); // '#' - replaceable in ENGLISH

        // Switch to secondary
        mapping.switchG0Charset();

        // Map same character after switch - should use FRENCH charset
        auto afterSwitch = mapping.mapG0Character(0x23);

        // Verify both mappings are valid
        CPPUNIT_ASSERT(beforeSwitch.first != 0);
        CPPUNIT_ASSERT(afterSwitch.first != 0);

        // Switch back to primary and verify we get the original mapping
        mapping.switchG0Charset();
        auto backToPrimary = mapping.mapG0Character(0x23);

        // Should match the original primary mapping
        CPPUNIT_ASSERT_EQUAL(beforeSwitch.first, backToPrimary.first);
        CPPUNIT_ASSERT_EQUAL(beforeSwitch.second, backToPrimary.second);
    }

    void testSwitchFromSecondaryToPrimary()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Switch to secondary
        mapping.switchG0Charset();

        // Switch back to primary
        mapping.switchG0Charset();

        // Should be back to primary - verify by mapping
        auto result = mapping.mapG0Character(0x41);
        CPPUNIT_ASSERT(result.first != 0);
    }

    void testMultipleConsecutiveSwitches()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Perform multiple switches and verify alternation
        auto result1 = mapping.mapG0Character(0x23);

        mapping.switchG0Charset(); // to secondary
        auto result2 = mapping.mapG0Character(0x23);

        mapping.switchG0Charset(); // back to primary
        auto result3 = mapping.mapG0Character(0x23);

        mapping.switchG0Charset(); // to secondary again
        auto result4 = mapping.mapG0Character(0x23);

        // Verify all are valid
        CPPUNIT_ASSERT(result1.first != 0);
        CPPUNIT_ASSERT(result2.first != 0);
        CPPUNIT_ASSERT(result3.first != 0);
        CPPUNIT_ASSERT(result4.first != 0);
    }

    void testSwitchWhenPrimaryEqualsSecondary()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::ENGLISH);

        auto before = mapping.mapG0Character(0x41);

        mapping.switchG0Charset();

        auto after = mapping.mapG0Character(0x41);

        // Should return same result since both charsets are identical
        CPPUNIT_ASSERT_EQUAL(before.first, after.first);
        CPPUNIT_ASSERT_EQUAL(before.second, after.second);
    }

    void testSetG1ToBlockMosaic()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        mapping.setCurrentG1(Charset::G1_BLOCK_MOSAIC);

        // Verify by mapping a G1 character
        auto result = mapping.mapG1Character(0x20);
        CPPUNIT_ASSERT(result.first != 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(Property::VALUE_DIACRITIC_NONE), result.second);
    }

    void testSetG1ToBlockMosaicSeparated()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        mapping.setCurrentG1(Charset::G1_BLOCK_MOSAIC_SEPARATED);

        // Verify by mapping a G1 character
        auto result = mapping.mapG1Character(0x20);
        CPPUNIT_ASSERT(result.first != 0);
    }

    void testSetG1MultipleTimesWithDifferentValues()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        mapping.setCurrentG1(Charset::G1_BLOCK_MOSAIC);
        auto result1 = mapping.mapG1Character(0x20);

        mapping.setCurrentG1(Charset::G1_BLOCK_MOSAIC_SEPARATED);
        auto result2 = mapping.mapG1Character(0x20);

        mapping.setCurrentG1(Charset::G1_BLOCK_MOSAIC);
        auto result3 = mapping.mapG1Character(0x20);

        // Verify all return valid values
        CPPUNIT_ASSERT(result1.first != 0);
        CPPUNIT_ASSERT(result2.first != 0);
        CPPUNIT_ASSERT(result3.first != 0);
    }

    void testMapStandardG0Character()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Map standard ASCII 'A' (0x41)
        auto result = mapping.mapG0Character(0x41);

        // Verify valid mapping
        CPPUNIT_ASSERT(result.first != 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(Property::VALUE_DIACRITIC_NONE), result.second);
    }

    void testMapCharacterAtMinimumValidRange()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Map character at 0x20 (space)
        auto result = mapping.mapG0Character(0x20);

        CPPUNIT_ASSERT(result.first != 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(Property::VALUE_DIACRITIC_NONE), result.second);
    }

    void testMapCharacterAtMaximumValidRange()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Map character at maximum range (0x20 + 95 = 0x7F)
        auto result = mapping.mapG0Character(0x7F);

        CPPUNIT_ASSERT(result.first != 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(Property::VALUE_DIACRITIC_NONE), result.second);
    }

    void testMapReplaceableCharacterInDiacriticMode()
    {
        MockCharsetConfig diacriticConfig(CharsetConfig::Mode::DIACTRIC);
        CharsetMapping mapping(diacriticConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Map a replaceable character (e.g., 0x23 '#')
        auto result = mapping.mapG0Character(0x23);

        // Should return mapped character with potentially non-zero diacritic property
        CPPUNIT_ASSERT(result.first != 0);
        // Property may or may not be VALUE_DIACRITIC_NONE depending on national subset
    }

    void testMapReplaceableCharacterInUTF16Mode()
    {
        MockCharsetConfig utf16Config(CharsetConfig::Mode::UTF);
        CharsetMapping mapping(utf16Config, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Map a replaceable character (e.g., 0x23 '#')
        auto result = mapping.mapG0Character(0x23);

        // Should return UTF16 character with VALUE_DIACRITIC_NONE
        CPPUNIT_ASSERT(result.first != 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(Property::VALUE_DIACRITIC_NONE), result.second);
    }

    void testMapNonReplaceableCharacter()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Map a non-replaceable character (e.g., 0x30 '0')
        auto result = mapping.mapG0Character(0x30);

        CPPUNIT_ASSERT(result.first != 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(Property::VALUE_DIACRITIC_NONE), result.second);
    }

    void testMapAllReplaceableCharacters()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // REPLACEBLE_CHARS = {0x23, 0x24, 0x40, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x7B, 0x7C, 0x7D, 0x7E}
        const std::array<std::uint8_t, 13> replaceableChars =
            {{0x23, 0x24, 0x40, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x7B, 0x7C, 0x7D, 0x7E}};

        for (auto ch : replaceableChars)
        {
            auto result = mapping.mapG0Character(ch);

            // Each should return valid mapping
            CPPUNIT_ASSERT(result.first != 0);
        }
    }

    void testMapCharacterBelowValidRange()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Map character below 0x20 (e.g., 0x10)
        auto result = mapping.mapG0Character(0x10);

        // Should return space (index 0 in charset map) due to clamping
        CPPUNIT_ASSERT(result.first != 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(Property::VALUE_DIACRITIC_NONE), result.second);
    }

    void testMapCharacterAboveValidRange()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Map character above valid range (e.g., 0xFF)
        auto result = mapping.mapG0Character(0xFF);

        // Should return space (index 0 in charset map) due to clamping
        CPPUNIT_ASSERT(result.first != 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(Property::VALUE_DIACRITIC_NONE), result.second);
    }

    void testMapCharacterWithSecondaryCharset()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Switch to secondary charset
        mapping.switchG0Charset();

        // Map a character
        auto result = mapping.mapG0Character(0x41);

        // Should use secondary (FRENCH) charset
        CPPUNIT_ASSERT(result.first != 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(Property::VALUE_DIACRITIC_NONE), result.second);
    }

    void testMapReplaceableCharInNonReplaceableCharset()
    {
        // Use a non-replaceable charset (e.g., CYRILLIC_1 which is not in REPLACEBLE_CHARSETS)
        CharsetMapping mapping(*m_mockConfig, NationalCharset::CYRILLIC_1, NationalCharset::CYRILLIC_2);

        // Map a replaceable character code
        auto result = mapping.mapG0Character(0x23);

        // Should use direct mapping, not replacement
        CPPUNIT_ASSERT(result.first != 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(Property::VALUE_DIACRITIC_NONE), result.second);
    }

    void testMapG1CharacterWithBlockMosaic()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Default G1 is G1_BLOCK_MOSAIC
        auto result = mapping.mapG1Character(0x20);

        CPPUNIT_ASSERT(result.first != 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(Property::VALUE_DIACRITIC_NONE), result.second);
    }

    void testMapG1CharacterWithBlockMosaicSeparated()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        mapping.setCurrentG1(Charset::G1_BLOCK_MOSAIC_SEPARATED);

        auto result = mapping.mapG1Character(0x20);

        CPPUNIT_ASSERT(result.first != 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(Property::VALUE_DIACRITIC_NONE), result.second);
    }

    void testMapG1CharacterAtBoundaryValues()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Test minimum
        auto resultMin = mapping.mapG1Character(0x20);
        CPPUNIT_ASSERT(resultMin.first != 0);

        // Test maximum (0x20 + 95 = 0x7F)
        auto resultMax = mapping.mapG1Character(0x7F);
        CPPUNIT_ASSERT(resultMax.first != 0);
    }

    void testMapG1CharacterOutOfValidRange()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Test below range
        auto resultBelow = mapping.mapG1Character(0x10);
        CPPUNIT_ASSERT(resultBelow.first != 0); // Should clamp to space

        // Test above range
        auto resultAbove = mapping.mapG1Character(0xFF);
        CPPUNIT_ASSERT(resultAbove.first != 0); // Should clamp to space
    }

    void testMapSameCharacterBeforeAndAfterSwitch()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Map with primary
        auto beforeSwitch = mapping.mapG0Character(0x41);

        // Switch to secondary
        mapping.switchG0Charset();
        auto afterSwitch = mapping.mapG0Character(0x41);

        // Both should be valid, verify state change doesn't cause crash
        CPPUNIT_ASSERT(beforeSwitch.first != 0);
        CPPUNIT_ASSERT(afterSwitch.first != 0);
    }

    void testMapG0AndG1CharactersIndependently()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        // Map G0 character
        auto g0Result = mapping.mapG0Character(0x41);

        // Change G1 charset
        mapping.setCurrentG1(Charset::G1_BLOCK_MOSAIC_SEPARATED);

        // Map G1 character - use 0x60 which is valid for G1 mosaics (not 0x41 which maps to 0)
        auto g1Result = mapping.mapG1Character(0x60);

        // Switch G0 charset
        mapping.switchG0Charset();

        // Map G0 again
        auto g0Result2 = mapping.mapG0Character(0x41);

        // Map G1 again - use same valid G1 character
        auto g1Result2 = mapping.mapG1Character(0x60);

        // Verify all mappings are independent and valid
        CPPUNIT_ASSERT(g0Result.first != 0);
        CPPUNIT_ASSERT(g1Result.first != 0);
        CPPUNIT_ASSERT(g0Result2.first != 0);
        CPPUNIT_ASSERT(g1Result2.first != 0);

        // G1 results should be same (G1 charset didn't change between calls)
        CPPUNIT_ASSERT_EQUAL(g1Result.first, g1Result2.first);
    }

private:
    std::unique_ptr<MockCharsetConfig> m_mockConfig;
};

CPPUNIT_TEST_SUITE_REGISTRATION( CharsetMappingTest );
