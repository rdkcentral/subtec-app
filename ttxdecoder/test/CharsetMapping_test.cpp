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

namespace
{

void assertMappedCharacter(const CharPropertyPair& result,
                           std::uint16_t expectedCharacter,
                           std::uint16_t expectedProperty = static_cast<std::uint16_t>(Property::VALUE_DIACRITIC_NONE))
{
    CPPUNIT_ASSERT_EQUAL(expectedCharacter, result.first);
    CPPUNIT_ASSERT_EQUAL(expectedProperty, result.second);
}

} // namespace

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
    CPPUNIT_TEST(testConstructorWithDifferentNationalCharsets);
    CPPUNIT_TEST(testSwitchFromPrimaryToSecondary);
    CPPUNIT_TEST(testMultipleConsecutiveSwitches);
    CPPUNIT_TEST(testSwitchWhenPrimaryEqualsSecondary);
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

        // Construction should select the primary G0 charset immediately.
        assertMappedCharacter(mapping.mapG0Character(0x23), 0x00A3);
    }

    void testConstructorWithDifferentNationalCharsets()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::GERMAN, NationalCharset::ITALIAN);

        auto resultPrimary = mapping.mapG0Character(0x23);
        assertMappedCharacter(resultPrimary, 0x0023);

        mapping.switchG0Charset();
        auto resultSecondary = mapping.mapG0Character(0x23);
        assertMappedCharacter(resultSecondary, 0x00A3);
        CPPUNIT_ASSERT(resultPrimary.first != resultSecondary.first);

        mapping.switchG0Charset();
        assertMappedCharacter(mapping.mapG0Character(0x23), 0x0023);
    }

    void testSwitchFromPrimaryToSecondary()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        auto beforeSwitch = mapping.mapG0Character(0x23);
        assertMappedCharacter(beforeSwitch, 0x00A3);

        mapping.switchG0Charset();

        auto afterSwitch = mapping.mapG0Character(0x23);
        assertMappedCharacter(afterSwitch, 0x00E9);
        CPPUNIT_ASSERT(beforeSwitch.first != afterSwitch.first);

        mapping.switchG0Charset();
        auto backToPrimary = mapping.mapG0Character(0x23);
        CPPUNIT_ASSERT_EQUAL(beforeSwitch.first, backToPrimary.first);
        CPPUNIT_ASSERT_EQUAL(beforeSwitch.second, backToPrimary.second);
    }

    void testMultipleConsecutiveSwitches()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        auto result1 = mapping.mapG0Character(0x23);
        assertMappedCharacter(result1, 0x00A3);

        mapping.switchG0Charset();
        auto result2 = mapping.mapG0Character(0x23);
        assertMappedCharacter(result2, 0x00E9);

        mapping.switchG0Charset();
        auto result3 = mapping.mapG0Character(0x23);
        assertMappedCharacter(result3, 0x00A3);

        mapping.switchG0Charset();
        auto result4 = mapping.mapG0Character(0x23);
        assertMappedCharacter(result4, 0x00E9);
    }

    void testSwitchWhenPrimaryEqualsSecondary()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::ENGLISH);

        auto before = mapping.mapG0Character(0x23);
        assertMappedCharacter(before, 0x00A3);

        mapping.switchG0Charset();

        auto after = mapping.mapG0Character(0x23);

        assertMappedCharacter(after, 0x00A3);
    }

    void testSetG1MultipleTimesWithDifferentValues()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        mapping.setCurrentG1(Charset::G1_BLOCK_MOSAIC);
        auto result1 = mapping.mapG1Character(0x20);
        assertMappedCharacter(result1, 0xE320);

        mapping.setCurrentG1(Charset::G1_BLOCK_MOSAIC_SEPARATED);
        auto result2 = mapping.mapG1Character(0x20);
        assertMappedCharacter(result2, 0xE420);

        mapping.setCurrentG1(Charset::G1_BLOCK_MOSAIC);
        auto result3 = mapping.mapG1Character(0x20);
        assertMappedCharacter(result3, 0xE320);
    }

    void testMapStandardG0Character()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        assertMappedCharacter(mapping.mapG0Character(0x41), 0x0041);
    }

    void testMapCharacterAtMinimumValidRange()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        assertMappedCharacter(mapping.mapG0Character(0x20), 0x0020);
    }

    void testMapCharacterAtMaximumValidRange()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        assertMappedCharacter(mapping.mapG0Character(0x7F), 0x25A0);
    }

    void testMapReplaceableCharacterInDiacriticMode()
    {
        MockCharsetConfig diacriticConfig(CharsetConfig::Mode::DIACTRIC);
        CharsetMapping mapping(diacriticConfig, NationalCharset::FRENCH, NationalCharset::ENGLISH);

        assertMappedCharacter(mapping.mapG0Character(0x23), 0x0065,
            static_cast<std::uint16_t>(Property::VALUE_DIACRITIC_ACUTE));
    }

    void testMapReplaceableCharacterInUTF16Mode()
    {
        MockCharsetConfig utf16Config(CharsetConfig::Mode::UTF);
        CharsetMapping mapping(utf16Config, NationalCharset::FRENCH, NationalCharset::ENGLISH);

        assertMappedCharacter(mapping.mapG0Character(0x23), 0x00E9);
    }

    void testMapNonReplaceableCharacter()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        assertMappedCharacter(mapping.mapG0Character(0x30), 0x0030);
    }

    void testMapAllReplaceableCharacters()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        const std::array<std::uint16_t, 13> expectedEnglishMappings =
            {{0x00A3, 0x0024, 0x0040, 0x2190, 0x00BD, 0x2192, 0x2191, 0x0023, 0x2014, 0x00BC, 0x2016, 0x00BE, 0x00F7}};

        for (std::size_t i = 0; i < REPLACEBLE_CHARS.size(); ++i)
        {
            assertMappedCharacter(mapping.mapG0Character(REPLACEBLE_CHARS[i]), expectedEnglishMappings[i]);
        }
    }

    void testMapCharacterBelowValidRange()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        assertMappedCharacter(mapping.mapG0Character(0x10), 0x0020);
    }

    void testMapCharacterAboveValidRange()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        assertMappedCharacter(mapping.mapG0Character(0xFF), 0x0020);
    }

    void testMapReplaceableCharInNonReplaceableCharset()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::CYRILLIC_1, NationalCharset::CYRILLIC_2);

        assertMappedCharacter(mapping.mapG0Character(0x23), 0x0023);
    }

    void testMapG1CharacterWithBlockMosaic()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        assertMappedCharacter(mapping.mapG1Character(0x20), 0xE320);
    }

    void testMapG1CharacterWithBlockMosaicSeparated()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        mapping.setCurrentG1(Charset::G1_BLOCK_MOSAIC_SEPARATED);

        assertMappedCharacter(mapping.mapG1Character(0x20), 0xE420);
    }

    void testMapG1CharacterAtBoundaryValues()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        assertMappedCharacter(mapping.mapG1Character(0x20), 0xE320);
        assertMappedCharacter(mapping.mapG1Character(0x7F), 0xE37F);
    }

    void testMapG1CharacterOutOfValidRange()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        assertMappedCharacter(mapping.mapG1Character(0x10), 0xE320);
        assertMappedCharacter(mapping.mapG1Character(0xFF), 0xE320);
    }

    void testMapSameCharacterBeforeAndAfterSwitch()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        auto beforeSwitch = mapping.mapG0Character(0x41);
        assertMappedCharacter(beforeSwitch, 0x0041);

        mapping.switchG0Charset();
        auto afterSwitch = mapping.mapG0Character(0x41);

        CPPUNIT_ASSERT_EQUAL(beforeSwitch.first, afterSwitch.first);
        CPPUNIT_ASSERT_EQUAL(beforeSwitch.second, afterSwitch.second);
    }

    void testMapG0AndG1CharactersIndependently()
    {
        CharsetMapping mapping(*m_mockConfig, NationalCharset::ENGLISH, NationalCharset::FRENCH);

        auto g0Result = mapping.mapG0Character(0x23);
        assertMappedCharacter(g0Result, 0x00A3);

        mapping.setCurrentG1(Charset::G1_BLOCK_MOSAIC_SEPARATED);

        auto g1Result = mapping.mapG1Character(0x60);
        assertMappedCharacter(g1Result, 0xE460);

        mapping.switchG0Charset();

        auto g0Result2 = mapping.mapG0Character(0x23);
        assertMappedCharacter(g0Result2, 0x00E9);

        auto g1Result2 = mapping.mapG1Character(0x60);
        CPPUNIT_ASSERT_EQUAL(g1Result.first, g1Result2.first);
        CPPUNIT_ASSERT_EQUAL(g1Result.second, g1Result2.second);
    }

private:
    std::unique_ptr<MockCharsetConfig> m_mockConfig;
};

CPPUNIT_TEST_SUITE_REGISTRATION( CharsetMappingTest );
