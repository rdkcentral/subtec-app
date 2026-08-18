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
        // Use distinct block and separated mappings so reverse-map collisions
        // are visible to the tests.
        for (std::size_t i = 0; i < m_blockMosaicMapping.size(); ++i)
        {
            m_blockMosaicMapping[i] = static_cast<std::uint16_t>(0x3000 + i);
            m_separatedMosaicMapping[i] = static_cast<std::uint16_t>(0x3400 + i);
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
    void setCharsetMapping(ttxdecoder::Charset charset,
                           const ttxdecoder::CharsetMappingArray& mapping) override
    {
        switch (charset)
        {
        case ttxdecoder::Charset::G1_BLOCK_MOSAIC:
            m_blockMosaicMapping = mapping;
            break;
        case ttxdecoder::Charset::G1_BLOCK_MOSAIC_SEPARATED:
            m_separatedMosaicMapping = mapping;
            break;
        default:
            break;
        }
    }
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
    CPPUNIT_TEST(testGenerateStripG1_DifferentSegmentsProdifferentGlyphs);
    CPPUNIT_TEST(testGenerateStripG1_BlockAndSeparatedAreDifferent);
    CPPUNIT_TEST(testGenerateStripG1_ConsecutiveCallsCreateNewObjects);
    CPPUNIT_TEST(testGenerateStripG1_MultipleCallsProduceSameResults);
    CPPUNIT_TEST(testGenerateStripG1_FontStripUsesProvidedEngine);
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
    CPPUNIT_TEST(testGenerateStripG1_BlockGlyph3PixelCountGreaterThanGlyph1);
    CPPUNIT_TEST(testGenerateStripG1_SeparatedGlyph3PixelCountGreaterThanGlyph1);
    CPPUNIT_TEST(testGenerateStripG1_SegmentFillingConsistency);
    CPPUNIT_TEST(testGenerateStripG1_BlockAndSeparatedIndex0AreEqual);
    CPPUNIT_TEST(testGenerateStripG1_SetBoundaryAt64);
    CPPUNIT_TEST(testGenerateStripG1_SeparatedBit0Shape);
    CPPUNIT_TEST(testGenerateStripG1_SeparatedGlyph3Shape);
    CPPUNIT_TEST(testGenerateStripG1_ReverseMappingWorks);
    CPPUNIT_TEST(testGenerateStripG1_SkipsCharsetGap);
    CPPUNIT_TEST(testGenerateStripG1_UsesUpdatedEngineMappings);

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

    static constexpr int GLYPH_WIDTH = 12;
    static constexpr int GLYPH_HEIGHT = 10;

    std::size_t mapCharsetIndex(std::size_t characterNumber) const
    {
        return (characterNumber < 32) ? characterNumber : characterNumber + 32;
    }

    std::uint16_t countFilledPixels(const std::vector<std::uint8_t>& glyph) const
    {
        std::uint16_t filled = 0;
        for (const auto& byte : glyph)
        {
            if (byte == 0xFF)
            {
                ++filled;
            }
        }
        return filled;
    }

    std::vector<std::uint8_t> buildExpectedGlyph(const std::vector<int>& segments,
                                                 bool separated) const
    {
        std::vector<std::uint8_t> glyph(GLYPH_WIDTH * GLYPH_HEIGHT, 0x00);

        const int startLeft = separated ? 1 : 0;
        const int startRight = separated ? 7 : 6;
        const int startTop = separated ? 1 : 0;
        const int startMid = separated ? 4 : 3;
        const int startBottom = 7;
        const int segmentWidth = separated ? 4 : 6;
        const int topHeight = separated ? 2 : 3;
        const int midHeight = separated ? 2 : 4;
        const int bottomHeight = separated ? 2 : 3;

        for (int segment : segments)
        {
            int offsetX = 0;
            int offsetY = 0;
            int sizeY = 0;

            switch (segment)
            {
            case 0:
                offsetX = startLeft;
                offsetY = startTop;
                sizeY = topHeight;
                break;
            case 1:
                offsetX = startRight;
                offsetY = startTop;
                sizeY = topHeight;
                break;
            case 2:
                offsetX = startLeft;
                offsetY = startMid;
                sizeY = midHeight;
                break;
            case 3:
                offsetX = startRight;
                offsetY = startMid;
                sizeY = midHeight;
                break;
            case 4:
                offsetX = startLeft;
                offsetY = startBottom;
                sizeY = bottomHeight;
                break;
            case 5:
                offsetX = startRight;
                offsetY = startBottom;
                sizeY = bottomHeight;
                break;
            default:
                continue;
            }

            for (int y = 0; y < sizeY; ++y)
            {
                for (int x = 0; x < segmentWidth; ++x)
                {
                    glyph[(offsetY + y) * GLYPH_WIDTH + (offsetX + x)] = 0xFF;
                }
            }
        }

        return glyph;
    }

    void assertGlyphEquals(const std::vector<std::uint8_t>& actual,
                           const std::vector<std::uint8_t>& expected) const
    {
        CPPUNIT_ASSERT_EQUAL(expected.size(), actual.size());
        for (std::size_t i = 0; i < expected.size(); ++i)
        {
            CPPUNIT_ASSERT_EQUAL(expected[i], actual[i]);
        }
    }

    void testGenerateStripG1_ReturnsValidPair()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        CPPUNIT_ASSERT(result.first != nullptr);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(128), result.second.getNeededGlyphCount());
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
        const auto& glyphData = mockStrip->getGlyphData(1);
        CPPUNIT_ASSERT(countFilledPixels(glyphData) > 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(1),
                             result.second.toGlyphIndex(m_ttxEngine->getCharsetMapping(
                                 ttxdecoder::Charset::G1_BLOCK_MOSAIC)[1]));
    }

    void testGenerateStripG1_SeparatedMosaicSetLoaded()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyphData = mockStrip->getGlyphData(65);
        CPPUNIT_ASSERT(countFilledPixels(glyphData) > 0);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(65),
                             result.second.toGlyphIndex(m_ttxEngine->getCharsetMapping(
                                 ttxdecoder::Charset::G1_BLOCK_MOSAIC_SEPARATED)[1]));
    }

    void testGenerateStripG1_BlockMosaicCountIs64()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        const auto& blockMapping = m_ttxEngine->getCharsetMapping(
            ttxdecoder::Charset::G1_BLOCK_MOSAIC);
        for (std::size_t i = 0; i < 64; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(i),
                                 result.second.toGlyphIndex(blockMapping[mapCharsetIndex(i)]));
        }
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
        for (int glyphIndex = 0; glyphIndex < 128; ++glyphIndex)
        {
            assertGlyphEquals(mockStrip1->getGlyphData(glyphIndex),
                              mockStrip2->getGlyphData(glyphIndex));
            CPPUNIT_ASSERT_EQUAL(result1.second.toCharacter(glyphIndex),
                                 result2.second.toCharacter(glyphIndex));
        }
    }

    void testGenerateStripG1_FontStripUsesProvidedEngine()
    {
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(0), m_gfxEngine->getCreationCount());
        GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(1), m_gfxEngine->getCreationCount());
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

        for (int y = 0; y < 10; ++y)
        {
            for (int x = 0; x < 12; ++x)
            {
                const std::uint8_t expected = (y >= 7 && x >= 6) ? 0xFF : 0x00;
                CPPUNIT_ASSERT_EQUAL(expected, glyph[y * 12 + x]);
            }
        }
    }

    void testGenerateStripG1_SegmentBit0BlockRendering()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph1 = mockStrip->getGlyphData(1);
        assertGlyphEquals(glyph1, buildExpectedGlyph({0}, false));
    }

    void testGenerateStripG1_SegmentBit1BlockRendering()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph2 = mockStrip->getGlyphData(2);
        assertGlyphEquals(glyph2, buildExpectedGlyph({1}, false));
    }

    void testGenerateStripG1_SegmentBit2BlockRendering()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph4 = mockStrip->getGlyphData(4);
        assertGlyphEquals(glyph4, buildExpectedGlyph({2}, false));
    }

    void testGenerateStripG1_SegmentBit3BlockRendering()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph8 = mockStrip->getGlyphData(8);
        assertGlyphEquals(glyph8, buildExpectedGlyph({3}, false));
    }

    void testGenerateStripG1_SegmentBit4BlockRendering()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph16 = mockStrip->getGlyphData(16);
        assertGlyphEquals(glyph16, buildExpectedGlyph({4}, false));
    }

    void testGenerateStripG1_SegmentBit5BlockRendering()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph32 = mockStrip->getGlyphData(32);
        assertGlyphEquals(glyph32, buildExpectedGlyph({5}, false));
    }

    void testGenerateStripG1_Glyph3HasTwoSegments()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph3 = mockStrip->getGlyphData(3);
        assertGlyphEquals(glyph3, buildExpectedGlyph({0, 1}, false));
    }

    void testGenerateStripG1_Glyph7HasThreeSegments()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph7 = mockStrip->getGlyphData(7);
        assertGlyphEquals(glyph7, buildExpectedGlyph({0, 1, 2}, false));
    }

    void testGenerateStripG1_BlockSegmentGlyphsProgressive()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        CPPUNIT_ASSERT(countFilledPixels(mockStrip->getGlyphData(0)) < countFilledPixels(mockStrip->getGlyphData(1)));
        CPPUNIT_ASSERT(countFilledPixels(mockStrip->getGlyphData(1)) < countFilledPixels(mockStrip->getGlyphData(3)));
        CPPUNIT_ASSERT(countFilledPixels(mockStrip->getGlyphData(3)) < countFilledPixels(mockStrip->getGlyphData(7)));
        CPPUNIT_ASSERT(countFilledPixels(mockStrip->getGlyphData(7)) < countFilledPixels(mockStrip->getGlyphData(15)));
        CPPUNIT_ASSERT(countFilledPixels(mockStrip->getGlyphData(15)) < countFilledPixels(mockStrip->getGlyphData(31)));
        CPPUNIT_ASSERT(countFilledPixels(mockStrip->getGlyphData(31)) < countFilledPixels(mockStrip->getGlyphData(63)));
    }

    void testGenerateStripG1_SeparatedSegmentGlyphsProgressive()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        CPPUNIT_ASSERT(countFilledPixels(mockStrip->getGlyphData(64)) < countFilledPixels(mockStrip->getGlyphData(65)));
        CPPUNIT_ASSERT(countFilledPixels(mockStrip->getGlyphData(65)) < countFilledPixels(mockStrip->getGlyphData(67)));
        CPPUNIT_ASSERT(countFilledPixels(mockStrip->getGlyphData(67)) < countFilledPixels(mockStrip->getGlyphData(71)));
        CPPUNIT_ASSERT(countFilledPixels(mockStrip->getGlyphData(71)) < countFilledPixels(mockStrip->getGlyphData(79)));
        CPPUNIT_ASSERT(countFilledPixels(mockStrip->getGlyphData(79)) < countFilledPixels(mockStrip->getGlyphData(95)));
        CPPUNIT_ASSERT(countFilledPixels(mockStrip->getGlyphData(95)) < countFilledPixels(mockStrip->getGlyphData(127)));
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

    void testGenerateStripG1_BlockGlyph3PixelCountGreaterThanGlyph1()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        std::uint16_t count1 = countFilledPixels(mockStrip->getGlyphData(1));
        std::uint16_t count2 = countFilledPixels(mockStrip->getGlyphData(2));
        std::uint16_t count3 = countFilledPixels(mockStrip->getGlyphData(3));
        CPPUNIT_ASSERT(count3 > count1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(count1 + count2), count3);
    }

    void testGenerateStripG1_SeparatedGlyph3PixelCountGreaterThanGlyph1()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        std::uint16_t count1 = countFilledPixels(mockStrip->getGlyphData(65));
        std::uint16_t count2 = countFilledPixels(mockStrip->getGlyphData(66));
        std::uint16_t count3 = countFilledPixels(mockStrip->getGlyphData(67));
        CPPUNIT_ASSERT(count3 > count1);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint16_t>(count1 + count2), count3);
    }

    void testGenerateStripG1_SegmentFillingConsistency()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph1 = mockStrip->getGlyphData(1);
        const auto& glyph2 = mockStrip->getGlyphData(2);
        const auto& glyph3 = mockStrip->getGlyphData(3);
        for (std::size_t i = 0; i < glyph3.size(); ++i)
        {
            const std::uint8_t expected = (glyph1[i] == 0xFF || glyph2[i] == 0xFF) ? 0xFF : 0x00;
            CPPUNIT_ASSERT_EQUAL(expected, glyph3[i]);
        }
    }

    void testGenerateStripG1_BlockAndSeparatedIndex0AreEqual()
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
        const auto& blockMapping = m_ttxEngine->getCharsetMapping(
            ttxdecoder::Charset::G1_BLOCK_MOSAIC);
        const auto& separatedMapping = m_ttxEngine->getCharsetMapping(
            ttxdecoder::Charset::G1_BLOCK_MOSAIC_SEPARATED);

        CPPUNIT_ASSERT(blockMapping[95] != separatedMapping[0]);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(63),
                             result.second.toGlyphIndex(blockMapping[95]));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(64),
                             result.second.toGlyphIndex(separatedMapping[0]));
    }

    void testGenerateStripG1_SeparatedBit0Shape()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph65 = mockStrip->getGlyphData(65);
        assertGlyphEquals(glyph65, buildExpectedGlyph({0}, true));
    }

    void testGenerateStripG1_SeparatedGlyph3Shape()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        auto mockStrip = m_gfxEngine->getLastCreatedFontStrip();
        const auto& glyph67 = mockStrip->getGlyphData(67);
        assertGlyphEquals(glyph67, buildExpectedGlyph({0, 1}, true));
    }

    void testGenerateStripG1_ReverseMappingWorks()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        const auto& blockMapping = m_ttxEngine->getCharsetMapping(
            ttxdecoder::Charset::G1_BLOCK_MOSAIC);
        const auto& separatedMapping = m_ttxEngine->getCharsetMapping(
            ttxdecoder::Charset::G1_BLOCK_MOSAIC_SEPARATED);

        for (std::size_t i = 0; i < 64; ++i)
        {
            CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(i),
                                 result.second.toGlyphIndex(blockMapping[mapCharsetIndex(i)]));
            CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(64 + i),
                                 result.second.toGlyphIndex(separatedMapping[mapCharsetIndex(i)]));
        }
    }

    void testGenerateStripG1_SkipsCharsetGap()
    {
        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);
        const auto& blockMapping = m_ttxEngine->getCharsetMapping(
            ttxdecoder::Charset::G1_BLOCK_MOSAIC);
        const auto& separatedMapping = m_ttxEngine->getCharsetMapping(
            ttxdecoder::Charset::G1_BLOCK_MOSAIC_SEPARATED);

        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(-1), result.second.toGlyphIndex(blockMapping[32]));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(-1), result.second.toGlyphIndex(blockMapping[63]));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(-1), result.second.toGlyphIndex(separatedMapping[32]));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(-1), result.second.toGlyphIndex(separatedMapping[63]));
    }

    void testGenerateStripG1_UsesUpdatedEngineMappings()
    {
        ttxdecoder::CharsetMappingArray blockMapping{};
        ttxdecoder::CharsetMappingArray separatedMapping{};

        for (std::size_t i = 0; i < blockMapping.size(); ++i)
        {
            blockMapping[i] = static_cast<std::uint16_t>(0x3000 + i);
            separatedMapping[i] = static_cast<std::uint16_t>(0x3400 + i);
        }

        m_ttxEngine->setCharsetMapping(ttxdecoder::Charset::G1_BLOCK_MOSAIC,
                                       blockMapping);
        m_ttxEngine->setCharsetMapping(ttxdecoder::Charset::G1_BLOCK_MOSAIC_SEPARATED,
                                       separatedMapping);

        auto result = GfxMosaicGenerator::generateStripG1(*m_ttxEngine, m_gfxEngine);

        CPPUNIT_ASSERT(result.first != nullptr);
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(blockMapping[0]), result.second.toCharacter(0));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(blockMapping[31]), result.second.toCharacter(31));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(blockMapping[64]), result.second.toCharacter(32));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(blockMapping[95]), result.second.toCharacter(63));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(separatedMapping[0]), result.second.toCharacter(64));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(separatedMapping[31]), result.second.toCharacter(95));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(separatedMapping[64]), result.second.toCharacter(96));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(separatedMapping[95]), result.second.toCharacter(127));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(0), result.second.toGlyphIndex(blockMapping[0]));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(31), result.second.toGlyphIndex(blockMapping[31]));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(32), result.second.toGlyphIndex(blockMapping[64]));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(63), result.second.toGlyphIndex(blockMapping[95]));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(64), result.second.toGlyphIndex(separatedMapping[0]));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(95), result.second.toGlyphIndex(separatedMapping[31]));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(96), result.second.toGlyphIndex(separatedMapping[64]));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(127), result.second.toGlyphIndex(separatedMapping[95]));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(-1), result.second.toGlyphIndex(blockMapping[32]));
        CPPUNIT_ASSERT_EQUAL(static_cast<std::int32_t>(-1), result.second.toGlyphIndex(separatedMapping[32]));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(GfxMosaicGeneratorTest);