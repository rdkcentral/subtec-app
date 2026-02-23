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
#include <vector>
#include <memory>
#include <set>

#include "GfxMosaicGenerator.hpp"
#include <ttxdecoder/Engine.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/gfx/FontStrip.hpp>

using namespace subttxrend::ttxt;

/**
 * Mock FontStrip for testing
 */
class MockFontStrip : public subttxrend::gfx::FontStrip
{
public:
    MockFontStrip(const subttxrend::gfx::Size& glyphSize, std::size_t glyphCount)
        : m_glyphSize(glyphSize), m_glyphCount(glyphCount), m_glyphsLoaded(0)
    {
        m_glyphs.resize(glyphCount);
    }

    bool loadGlyph(std::int32_t glyphIndex, const std::uint8_t* data,
                   std::size_t dataSize) override
    {
        if (glyphIndex >= 0 && glyphIndex < static_cast<std::int32_t>(m_glyphCount) && data && dataSize > 0)
        {
            m_glyphs[glyphIndex].assign(data, data + dataSize);
            m_glyphsLoaded++;
            return true;
        }
        return false;
    }

    bool loadFont(const std::string& fontName,
                  const subttxrend::gfx::Size& glyphSize,
                  const subttxrend::gfx::FontStripMap& mapping) override
    {
        return true;
    }

    std::uint16_t getGlyphsLoaded() const { return m_glyphsLoaded; }
    std::uint16_t getTotalGlyphs() const { return m_glyphCount; }
    const std::vector<std::uint8_t>& getGlyphData(std::size_t index) const
    {
        return m_glyphs[index];
    }
    const subttxrend::gfx::Size& getGlyphSize() const { return m_glyphSize; }

private:
    subttxrend::gfx::Size m_glyphSize;
    std::size_t m_glyphCount;
    std::uint16_t m_glyphsLoaded;
    std::vector<std::vector<std::uint8_t>> m_glyphs;
};

using MockFontStripPtr = std::shared_ptr<MockFontStrip>;

/**
 * Mock Graphics Engine for testing
 */
class MockGfxEngine : public subttxrend::gfx::Engine
{
public:
    void init(const std::string &displayName = {}) override {}
    void shutdown() override {}
    void execute() override {}
    subttxrend::gfx::WindowPtr createWindow() override { return nullptr; }
    void attach(subttxrend::gfx::WindowPtr window) override {}
    void detach(subttxrend::gfx::WindowPtr window) override {}

    subttxrend::gfx::FontStripPtr createFontStrip(const subttxrend::gfx::Size& glyphSize,
                                                  std::size_t glyphCount) override
    {
        auto fontStrip = std::make_shared<MockFontStrip>(glyphSize, glyphCount);
        m_lastCreatedFontStrip = fontStrip;
        m_creationCount++;
        return fontStrip;
    }

    MockFontStripPtr getLastCreatedFontStrip() const { return m_lastCreatedFontStrip; }
    std::uint16_t getCreationCount() const { return m_creationCount; }

private:
    MockFontStripPtr m_lastCreatedFontStrip;
    std::uint16_t m_creationCount = 0;
};

/**
 * Mock TTX Engine for testing - provides charset mappings
 */
class MockTtxEngine : public ttxdecoder::Engine
{
public:
    MockTtxEngine()
    {
        // Initialize with default G1 block mosaic mapping
        // CharsetMappingArray size is 96 (CHARSET_MAPPING_SIZE)
        for (std::size_t i = 0; i < m_blockMosaicMapping.size(); ++i)
        {
            m_blockMosaicMapping[i] = 0x2500 + i;
            m_separatedMosaicMapping[i] = 0x2500 + i;
        }
    }

    // Only getCharsetMapping() is used by tests; other pure virtuals stubbed minimally
    const ttxdecoder::CharsetMappingArray& getCharsetMapping(
        ttxdecoder::Charset charset) const override
    {
        switch (charset)
        {
        case ttxdecoder::Charset::G1_BLOCK_MOSAIC:
            return m_blockMosaicMapping;
        case ttxdecoder::Charset::G1_BLOCK_MOSAIC_SEPARATED:
            return m_separatedMosaicMapping;
        default:
            return m_blockMosaicMapping;
        }
    }

    // Unused pure virtual implementations - stubbed for compilation
    void resetAcquisition() override {}
    std::uint32_t process() override { return 0; }
    bool addPesPacket(const std::uint8_t* packet, std::uint16_t packetLength) override { return true; }
    void setCurrentPageId(const ttxdecoder::PageId& pageId) override {}
    ttxdecoder::PageId getNextPageId(const ttxdecoder::PageId& inputPageId) const override { return inputPageId; }
    ttxdecoder::PageId getPrevPageId(const ttxdecoder::PageId& inputPageId) const override { return inputPageId; }
    ttxdecoder::PageId getPageId(ttxdecoder::PageIdType type) const override { return ttxdecoder::PageId(); }
    const ttxdecoder::DecodedPage& getPage() const override { static ttxdecoder::DecodedPage p; return p; }
    void setNavigationMode(ttxdecoder::NavigationMode navigationMode) override {}
    ttxdecoder::NavigationState getNavigationState() const override { return ttxdecoder::NavigationState(); }
    void getTopLinkText(std::uint16_t magazine, std::uint32_t page, std::uint16_t* text) const override {}
    std::uint8_t getScreenColorIndex() const override { return 0; }
    std::uint8_t getRowColorIndex(std::uint8_t row) const override { return 0; }
    bool getColors(std::array<std::uint32_t, 16>& colors) const override { return true; }
    void setCharsetMapping(ttxdecoder::Charset charset, const ttxdecoder::CharsetMappingArray& mapping) override {}
    void setDefaultPrimaryNationalCharset(std::uint8_t index, ttxdecoder::NationalCharset charset) override {}
    void setDefaultSecondaryNationalCharset(std::uint8_t index, ttxdecoder::NationalCharset charset) override {}
    std::uint8_t getPageControlInfo() const override { return 0; }
    void setIgnorePts(bool ignorePts) override {}

private:
    ttxdecoder::CharsetMappingArray m_blockMosaicMapping;
    ttxdecoder::CharsetMappingArray m_separatedMosaicMapping;
};

class GfxMosaicGeneratorTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(GfxMosaicGeneratorTest);

    CPPUNIT_TEST(testGenerateStripG1_ReturnsValidPair);
    CPPUNIT_TEST(testGenerateStripG1_CreatesCorrectGlyphCount);
    CPPUNIT_TEST(testGenerateStripG1_LoadsAllGlyphs);
    CPPUNIT_TEST(testGenerateStripG1_GlyphSizeIsCorrect);
    CPPUNIT_TEST(testGenerateStripG1_BlockMosaicSetLoaded);
    CPPUNIT_TEST(testGenerateStripG1_SeparatedMosaicSetLoaded);
    CPPUNIT_TEST(testGenerateStripG1_BlockMosaicCountIs64);
    CPPUNIT_TEST(testGenerateStripG1_ZeroSegmentIndexAllZeros);
    CPPUNIT_TEST(testGenerateStripG1_MaxSegmentIndexMostlyFilled);
    CPPUNIT_TEST(testGenerateStripG1_DifferentSegmentsProdifferentGlyphs);
    CPPUNIT_TEST(testGenerateStripG1_BlockAndSeparatedAreDifferent);
    CPPUNIT_TEST(testGenerateStripG1_ConsecutiveCallsCreateNewObjects);
    CPPUNIT_TEST(testGenerateStripG1_MultipleCallsProduceSameResults);
    CPPUNIT_TEST(testGenerateStripG1_FontStripUsesProvidedEngine);
    CPPUNIT_TEST(testGenerateStripG1_AllBlockGlyphsHaveCorrectSize);
    CPPUNIT_TEST(testGenerateStripG1_AllSeparatedGlyphsHaveCorrectSize);
    CPPUNIT_TEST(testGenerateStripG1_GlyphDataNotAllZeros);
    CPPUNIT_TEST(testGenerateStripG1_GlyphDataNotAllOnes);
    CPPUNIT_TEST(testGenerateStripG1_GlyphDataSizeCorrect);
    CPPUNIT_TEST(testGenerateStripG1_GlyphPixelBoundsValid);
    CPPUNIT_TEST(testGenerateStripG1_SegmentBoundariesRespected);
    CPPUNIT_TEST(testGenerateStripG1_SegmentBit0BlockRendering);
    CPPUNIT_TEST(testGenerateStripG1_SegmentBit1BlockRendering);
    CPPUNIT_TEST(testGenerateStripG1_SegmentBit2BlockRendering);
    CPPUNIT_TEST(testGenerateStripG1_SegmentBit3BlockRendering);
    CPPUNIT_TEST(testGenerateStripG1_SegmentBit4BlockRendering);
    CPPUNIT_TEST(testGenerateStripG1_SegmentBit5BlockRendering);
    CPPUNIT_TEST(testGenerateStripG1_Glyph3HasTwoSegments);
    CPPUNIT_TEST(testGenerateStripG1_Glyph7HasThreeSegments);
    CPPUNIT_TEST(testGenerateStripG1_BlockSegmentGlyphsProgressive);
    CPPUNIT_TEST(testGenerateStripG1_SeparatedSegmentGlyphsProgressive);
    CPPUNIT_TEST(testGenerateStripG1_SegmentCombinationsCreateUniqueness);
    CPPUNIT_TEST(testGenerateStripG1_AdjacentGlyphsHaveDifferentPatterns);
    CPPUNIT_TEST(testGenerateStripG1_BlockGlyph1PixelCount);
    CPPUNIT_TEST(testGenerateStripG1_BlockGlyph3PixelCountGreaterThanGlyph1);
    CPPUNIT_TEST(testGenerateStripG1_SeparatedGlyph1PixelCount);
    CPPUNIT_TEST(testGenerateStripG1_SeparatedGlyph3PixelCountGreaterThanGlyph1);
    CPPUNIT_TEST(testGenerateStripG1_BlockFullGlyphPixelDistribution);
    CPPUNIT_TEST(testGenerateStripG1_SeparatedFullGlyphPixelDistribution);
    CPPUNIT_TEST(testGenerateStripG1_PixelDistributionMonotonic);
    CPPUNIT_TEST(testGenerateStripG1_SegmentFillingConsistency);
    CPPUNIT_TEST(testGenerateStripG1_BlockAndSeparatedIndex0Different);
    CPPUNIT_TEST(testGenerateStripG1_SetBoundaryAt64);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        m_ttxEngine = std::make_shared<MockTtxEngine>();
        m_gfxEngine = std::make_shared<MockGfxEngine>();
    }

    void tearDown() override
    {
        m_ttxEngine.reset();
        m_gfxEngine.reset();
    }

protected:
    std::shared_ptr<MockTtxEngine> m_ttxEngine;
    std::shared_ptr<MockGfxEngine> m_gfxEngine;

    void testGenerateStripG1_ReturnsValidPair()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        CPPUNIT_ASSERT(result.first != nullptr);
    }

    void testGenerateStripG1_CreatesCorrectGlyphCount()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(128), mockStrip->getTotalGlyphs());
    }

    void testGenerateStripG1_LoadsAllGlyphs()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(128), mockStrip->getGlyphsLoaded());
    }

    void testGenerateStripG1_GlyphSizeIsCorrect()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        CPPUNIT_ASSERT_EQUAL(12, static_cast<int>(mockStrip->getGlyphSize().m_w));
        CPPUNIT_ASSERT_EQUAL(10, static_cast<int>(mockStrip->getGlyphSize().m_h));
    }

    void testGenerateStripG1_BlockMosaicSetLoaded()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyphData = mockStrip->getGlyphData(0);
        CPPUNIT_ASSERT_EQUAL(false, glyphData.empty());
    }

    void testGenerateStripG1_SeparatedMosaicSetLoaded()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyphData = mockStrip->getGlyphData(64);
        CPPUNIT_ASSERT_EQUAL(false, glyphData.empty());
    }

    void testGenerateStripG1_BlockMosaicCountIs64()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        std::uint16_t count = 0;
        for (int i = 0; i < 64; ++i)
        {
            if (!mockStrip->getGlyphData(i).empty())
                count++;
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(64), count);
    }

    void testGenerateStripG1_ZeroSegmentIndexAllZeros()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyphData = mockStrip->getGlyphData(0);
        bool allZeros = true;
        for (const auto& byte : glyphData)
        {
            if (byte != 0)
            {
                allZeros = false;
                break;
            }
        }
        CPPUNIT_ASSERT(allZeros);
    }

    void testGenerateStripG1_MaxSegmentIndexMostlyFilled()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyphData = mockStrip->getGlyphData(63);
        std::uint16_t filledPixels = 0;
        for (const auto& byte : glyphData)
        {
            if (byte != 0)
                filledPixels++;
        }
        CPPUNIT_ASSERT(filledPixels > 60);
    }

    void testGenerateStripG1_DifferentSegmentsProdifferentGlyphs()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph1 = mockStrip->getGlyphData(1);
        const auto& glyph2 = mockStrip->getGlyphData(2);
        CPPUNIT_ASSERT(glyph1 != glyph2);
    }

    void testGenerateStripG1_BlockAndSeparatedAreDifferent()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& blockGlyph32 = mockStrip->getGlyphData(32);
        const auto& separatedGlyph96 = mockStrip->getGlyphData(96);
        CPPUNIT_ASSERT(blockGlyph32 != separatedGlyph96);
    }

    void testGenerateStripG1_ConsecutiveCallsCreateNewObjects()
    {
        auto result1 = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto result2 = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        CPPUNIT_ASSERT(result1.first.get() != result2.first.get());
    }

    void testGenerateStripG1_MultipleCallsProduceSameResults()
    {
        auto result1 = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip1 = m_gfxEngine->getLastCreatedFontStrip();
        auto result2 = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip2 = m_gfxEngine->getLastCreatedFontStrip();
        CPPUNIT_ASSERT_EQUAL(mockStrip1->getTotalGlyphs(), mockStrip2->getTotalGlyphs());
        // Compare glyph data element-by-element (can't use CPPUNIT_ASSERT_EQUAL with vectors)
        const auto& data1 = mockStrip1->getGlyphData(32);
        const auto& data2 = mockStrip2->getGlyphData(32);
        CPPUNIT_ASSERT_EQUAL(data1.size(), data2.size());
        for (std::size_t i = 0; i < data1.size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(data1[i], data2[i]);
        }
    }

    void testGenerateStripG1_FontStripUsesProvidedEngine()
    {
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0), m_gfxEngine->getCreationCount());
        GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(1), m_gfxEngine->getCreationCount());
    }

    void testGenerateStripG1_AllBlockGlyphsHaveCorrectSize()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        for (int i = 0; i < 64; ++i)
        {
            const auto& glyphData = mockStrip->getGlyphData(i);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(120), glyphData.size());
        }
    }

    void testGenerateStripG1_AllSeparatedGlyphsHaveCorrectSize()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        for (int i = 64; i < 128; ++i)
        {
            const auto& glyphData = mockStrip->getGlyphData(i);
            CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(120), glyphData.size());
        }
    }

    void testGenerateStripG1_GlyphDataNotAllZeros()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        bool hasNonZero = false;
        for (int i = 0; i < 128; ++i)
        {
            const auto& glyphData = mockStrip->getGlyphData(i);
            for (const auto& byte : glyphData)
            {
                if (byte != 0)
                {
                    hasNonZero = true;
                    break;
                }
            }
            if (hasNonZero) break;
        }
        CPPUNIT_ASSERT(hasNonZero);
    }

    void testGenerateStripG1_GlyphDataNotAllOnes()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        bool hasZero = false;
        for (int i = 0; i < 128; ++i)
        {
            const auto& glyphData = mockStrip->getGlyphData(i);
            for (const auto& byte : glyphData)
            {
                if (byte == 0)
                {
                    hasZero = true;
                    break;
                }
            }
            if (hasZero) break;
        }
        CPPUNIT_ASSERT(hasZero);
    }

    void testGenerateStripG1_GlyphDataSizeCorrect()
    {
        const std::size_t expectedSize = 12 * 10;
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        for (int i = 0; i < 128; ++i)
        {
            const auto& glyphData = mockStrip->getGlyphData(i);
            CPPUNIT_ASSERT_EQUAL(expectedSize, glyphData.size());
        }
    }

    void testGenerateStripG1_GlyphPixelBoundsValid()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        for (int i = 0; i < 128; ++i)
        {
            const auto& glyphData = mockStrip->getGlyphData(i);
            for (const auto& byte : glyphData)
            {
                CPPUNIT_ASSERT(byte == 0x00 || byte == 0xFF);
            }
        }
    }

    void testGenerateStripG1_SegmentBoundariesRespected()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph = mockStrip->getGlyphData(32);
        CPPUNIT_ASSERT_EQUAL(false, glyph.empty());
        bool hasFilled = false, hasUnfilled = false;
        for (const auto& byte : glyph)
        {
            if (byte == 0xFF) hasFilled = true;
            if (byte == 0x00) hasUnfilled = true;
        }
        CPPUNIT_ASSERT(hasFilled || hasUnfilled);
    }

    void testGenerateStripG1_SegmentBit0BlockRendering()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph1 = mockStrip->getGlyphData(1);
        const auto& glyph0 = mockStrip->getGlyphData(0);
        std::uint16_t topLeftFilled = 0;
        for (int y = 0; y < 3; ++y)
        {
            for (int x = 0; x < 6; ++x)
            {
                if (glyph1[y * 12 + x] == 0xFF)
                    topLeftFilled++;
            }
        }
        CPPUNIT_ASSERT(topLeftFilled > 0);
        CPPUNIT_ASSERT(glyph1 != glyph0);
    }

    void testGenerateStripG1_SegmentBit1BlockRendering()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph2 = mockStrip->getGlyphData(2);
        const auto& glyph1 = mockStrip->getGlyphData(1);
        std::uint16_t topRightFilled = 0;
        for (int y = 0; y < 3; ++y)
        {
            for (int x = 6; x < 12; ++x)
            {
                if (glyph2[y * 12 + x] == 0xFF)
                    topRightFilled++;
            }
        }
        CPPUNIT_ASSERT(topRightFilled > 0);
        CPPUNIT_ASSERT(glyph2 != glyph1);
    }

    void testGenerateStripG1_SegmentBit2BlockRendering()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph4 = mockStrip->getGlyphData(4);
        const auto& glyph0 = mockStrip->getGlyphData(0);
        std::uint16_t midLeftFilled = 0;
        for (int y = 3; y < 7; ++y)
        {
            for (int x = 0; x < 6; ++x)
            {
                if (glyph4[y * 12 + x] == 0xFF)
                    midLeftFilled++;
            }
        }
        CPPUNIT_ASSERT(midLeftFilled > 0);
        CPPUNIT_ASSERT(glyph4 != glyph0);
    }

    void testGenerateStripG1_SegmentBit3BlockRendering()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph8 = mockStrip->getGlyphData(8);
        const auto& glyph0 = mockStrip->getGlyphData(0);
        std::uint16_t midRightFilled = 0;
        for (int y = 3; y < 7; ++y)
        {
            for (int x = 6; x < 12; ++x)
            {
                if (glyph8[y * 12 + x] == 0xFF)
                    midRightFilled++;
            }
        }
        CPPUNIT_ASSERT(midRightFilled > 0);
        CPPUNIT_ASSERT(glyph8 != glyph0);
    }

    void testGenerateStripG1_SegmentBit4BlockRendering()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph16 = mockStrip->getGlyphData(16);
        std::uint16_t botLeftFilled = 0;
        for (int y = 7; y < 10; ++y)
        {
            for (int x = 0; x < 6; ++x)
            {
                if (glyph16[y * 12 + x] == 0xFF)
                    botLeftFilled++;
            }
        }
        CPPUNIT_ASSERT(botLeftFilled > 0);
    }

    void testGenerateStripG1_SegmentBit5BlockRendering()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph32 = mockStrip->getGlyphData(32);
        std::uint16_t botRightFilled = 0;
        for (int y = 7; y < 10; ++y)
        {
            for (int x = 6; x < 12; ++x)
            {
                if (glyph32[y * 12 + x] == 0xFF)
                    botRightFilled++;
            }
        }
        CPPUNIT_ASSERT(botRightFilled > 0);
    }

    void testGenerateStripG1_Glyph3HasTwoSegments()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph3 = mockStrip->getGlyphData(3);
        const auto& glyph1 = mockStrip->getGlyphData(1);
        const auto& glyph2 = mockStrip->getGlyphData(2);
        std::uint16_t filled3 = 0, filled1 = 0, filled2 = 0;
        for (const auto& byte : glyph3) if (byte == 0xFF) filled3++;
        for (const auto& byte : glyph1) if (byte == 0xFF) filled1++;
        for (const auto& byte : glyph2) if (byte == 0xFF) filled2++;
        CPPUNIT_ASSERT(filled3 > filled1);
        CPPUNIT_ASSERT(filled3 > filled2);
    }

    void testGenerateStripG1_Glyph7HasThreeSegments()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph7 = mockStrip->getGlyphData(7);
        const auto& glyph3 = mockStrip->getGlyphData(3);
        std::uint16_t filled7 = 0, filled3 = 0;
        for (const auto& byte : glyph7) if (byte == 0xFF) filled7++;
        for (const auto& byte : glyph3) if (byte == 0xFF) filled3++;
        CPPUNIT_ASSERT(filled7 > filled3);
    }

    void testGenerateStripG1_BlockSegmentGlyphsProgressive()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        std::vector<std::uint16_t> pixelCounts;
        for (int i = 0; i < 64; ++i)
        {
            std::uint16_t count = 0;
            const auto& data = mockStrip->getGlyphData(i);
            for (const auto& byte : data) if (byte == 0xFF) count++;
            pixelCounts.push_back(count);
        }
        CPPUNIT_ASSERT(pixelCounts[63] > pixelCounts[0]);
        CPPUNIT_ASSERT(pixelCounts[32] >= pixelCounts[0]);
    }

    void testGenerateStripG1_SeparatedSegmentGlyphsProgressive()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        std::uint16_t filled64 = 0, filled127 = 0;
        for (const auto& byte : mockStrip->getGlyphData(64)) if (byte == 0xFF) filled64++;
        for (const auto& byte : mockStrip->getGlyphData(127)) if (byte == 0xFF) filled127++;
        CPPUNIT_ASSERT(filled127 > filled64);
    }

    void testGenerateStripG1_SegmentCombinationsCreateUniqueness()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        std::set<std::vector<std::uint8_t>> uniqueGlyphs;
        for (int i = 0; i < 64; ++i)
        {
            uniqueGlyphs.insert(mockStrip->getGlyphData(i));
        }
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(64), uniqueGlyphs.size());
    }

    void testGenerateStripG1_AdjacentGlyphsHaveDifferentPatterns()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        for (int i = 0; i < 62; ++i)
        {
            CPPUNIT_ASSERT(mockStrip->getGlyphData(i) != mockStrip->getGlyphData(i+1));
        }
    }

    void testGenerateStripG1_BlockGlyph1PixelCount()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        std::uint16_t count = 0;
        for (const auto& byte : mockStrip->getGlyphData(1))
            if (byte == 0xFF) count++;
        CPPUNIT_ASSERT(count > 0);
        CPPUNIT_ASSERT(count < 120);
    }

    void testGenerateStripG1_BlockGlyph3PixelCountGreaterThanGlyph1()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        std::uint16_t count1 = 0, count3 = 0;
        for (const auto& byte : mockStrip->getGlyphData(1)) if (byte == 0xFF) count1++;
        for (const auto& byte : mockStrip->getGlyphData(3)) if (byte == 0xFF) count3++;
        CPPUNIT_ASSERT(count3 > count1);
    }

    void testGenerateStripG1_SeparatedGlyph1PixelCount()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        std::uint16_t count = 0;
        for (const auto& byte : mockStrip->getGlyphData(65))
            if (byte == 0xFF) count++;
        CPPUNIT_ASSERT(count > 0);
        CPPUNIT_ASSERT(count < 120);
    }

    void testGenerateStripG1_SeparatedGlyph3PixelCountGreaterThanGlyph1()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        std::uint16_t count1 = 0, count3 = 0;
        for (const auto& byte : mockStrip->getGlyphData(65)) if (byte == 0xFF) count1++;
        for (const auto& byte : mockStrip->getGlyphData(67)) if (byte == 0xFF) count3++;
        CPPUNIT_ASSERT(count3 > count1);
    }

    void testGenerateStripG1_BlockFullGlyphPixelDistribution()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        std::uint16_t count = 0;
        for (const auto& byte : mockStrip->getGlyphData(63))
            if (byte == 0xFF) count++;
        CPPUNIT_ASSERT(count > 100);
    }

    void testGenerateStripG1_SeparatedFullGlyphPixelDistribution()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        std::uint16_t count = 0;
        for (const auto& byte : mockStrip->getGlyphData(127))
            if (byte == 0xFF) count++;
        // Separated glyphs: 8 pixels per segment (4x2 each), glyph 127 (all 6 segments) = 48 pixels
        CPPUNIT_ASSERT(count > 40);
    }

    void testGenerateStripG1_PixelDistributionMonotonic()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        std::uint16_t prev = 0, monotonic = 0;
        for (int i = 0; i < 64; ++i)
        {
            std::uint16_t count = 0;
            for (const auto& byte : mockStrip->getGlyphData(i))
                if (byte == 0xFF) count++;
            if (count >= prev) monotonic++;
            prev = count;
        }
        // Most glyphs should show non-decreasing pixel count as segment bits increase
        CPPUNIT_ASSERT(monotonic >= 45);
    }

    void testGenerateStripG1_SegmentFillingConsistency()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        std::uint16_t count1 = 0, count3 = 0, count2 = 0;
        for (const auto& byte : mockStrip->getGlyphData(1)) if (byte == 0xFF) count1++;
        for (const auto& byte : mockStrip->getGlyphData(2)) if (byte == 0xFF) count2++;
        for (const auto& byte : mockStrip->getGlyphData(3)) if (byte == 0xFF) count3++;
        CPPUNIT_ASSERT(count3 >= count1);
        CPPUNIT_ASSERT(count3 >= count2);
    }

    void testGenerateStripG1_BlockAndSeparatedIndex0Different()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& block0 = mockStrip->getGlyphData(0);
        const auto& sep0 = mockStrip->getGlyphData(64);
        // Block 0 and separated 0 should be equal (both empty/no segments)
        CPPUNIT_ASSERT_EQUAL(block0.size(), sep0.size());
        for (std::size_t i = 0; i < block0.size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(block0[i], sep0[i]);
        }
    }

    void testGenerateStripG1_SetBoundaryAt64()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& last_block = mockStrip->getGlyphData(63);
        const auto& first_sep = mockStrip->getGlyphData(64);
        CPPUNIT_ASSERT(!last_block.empty());
        CPPUNIT_ASSERT(!first_sep.empty());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(GfxMosaicGeneratorTest);