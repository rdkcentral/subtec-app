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

#include <chrono>
#include <cstdio>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <subttxrend/common/ConfigProvider.hpp>

#include "TtmlEngineImpl.hpp"

using namespace std::chrono_literals;
using namespace subttxrend::ttmlengine;

namespace
{

const char* const kSinglePixelPngBase64 =
    "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR4nGNgYGBgAAAABQABpfZFQAAAAABJRU5ErkJggg==";

const char* const kTwoByTwoPngBase64 =
    "iVBORw0KGgoAAAANSUhEUgAAAAIAAAACCAYAAABytg0kAAAAFElEQVR4nGNkYPjfwMDAwMDEAAUAFicBg36MA2IAAAAASUVORK5CYII=";

struct BitmapCall
{
    subttxrend::gfx::Rectangle rectangle;
};

class RecordingDrawContext : public subttxrend::gfx::DrawContext
{
public:
    void fillRectangle(subttxrend::gfx::ColorArgb color,
                       const subttxrend::gfx::Rectangle& rectangle) override
    {
        (void)color;
        (void)rectangle;
    }

    void drawUnderline(subttxrend::gfx::ColorArgb color,
                       const subttxrend::gfx::Rectangle& rectangle) override
    {
        (void)color;
        (void)rectangle;
    }

    void drawPixmap(const subttxrend::gfx::ClutBitmap& bitmap,
                    const subttxrend::gfx::Rectangle& srcRect,
                    const subttxrend::gfx::Rectangle& dstRect) override
    {
        (void)bitmap;
        (void)srcRect;
        (void)dstRect;
    }

    void drawBitmap(const subttxrend::gfx::Bitmap& bitmap,
                    const subttxrend::gfx::Rectangle& dstRect) override
    {
        (void)bitmap;
        bitmapCalls.push_back({dstRect});
    }

    void drawGlyph(const subttxrend::gfx::FontStripPtr& fontStrip,
                   std::int32_t glyphIndex,
                   const subttxrend::gfx::Rectangle& rect,
                   subttxrend::gfx::ColorArgb fgColor,
                   subttxrend::gfx::ColorArgb bgColor) override
    {
        (void)fontStrip;
        (void)glyphIndex;
        (void)rect;
        (void)fgColor;
        (void)bgColor;
    }

    void drawString(subttxrend::gfx::PrerenderedFont& font,
                    const subttxrend::gfx::Rectangle& destinationRect,
                    const std::vector<subttxrend::gfx::GlyphData>& glyphs,
                    const subttxrend::gfx::ColorArgb fgColor,
                    const subttxrend::gfx::ColorArgb bgColor,
                    int outlineSize = 0,
                    int verticalOffset = 0) override
    {
        (void)font;
        (void)destinationRect;
        (void)glyphs;
        (void)fgColor;
        (void)bgColor;
        (void)outlineSize;
        (void)verticalOffset;
        drawStringCalls.push_back(1);
    }

    void reset()
    {
        drawStringCalls.clear();
        bitmapCalls.clear();
    }

    std::vector<int> drawStringCalls;
    std::vector<BitmapCall> bitmapCalls;
};

class RecordingWindow : public subttxrend::gfx::Window
{
public:
    RecordingWindow(subttxrend::gfx::Size preferredSize,
                    subttxrend::gfx::Size currentSize)
        : m_bounds(0, 0, currentSize.m_w, currentSize.m_h),
          m_preferredSize(preferredSize),
          m_size(currentSize)
    {
    }

    void addKeyEventListener(subttxrend::gfx::KeyEventListener* listener) override
    {
        (void)listener;
    }

    void removeKeyEventListener(subttxrend::gfx::KeyEventListener* listener) override
    {
        (void)listener;
    }

    subttxrend::gfx::Rectangle getBounds() const override
    {
        return m_bounds;
    }

    subttxrend::gfx::DrawContext& getDrawContext() override
    {
        return drawContext;
    }

    subttxrend::gfx::Size getPreferredSize() const override
    {
        return m_preferredSize;
    }

    void setSize(const subttxrend::gfx::Size& newSize) override
    {
        resizeCalls.push_back(newSize);
        m_size = newSize;
        m_bounds = subttxrend::gfx::Rectangle(0, 0, newSize.m_w, newSize.m_h);
    }

    subttxrend::gfx::Size getSize() const override
    {
        return m_size;
    }

    void setVisible(bool visible) override
    {
        visibleCalls.push_back(visible);
    }

    void clear() override
    {
        ++clearCount;
    }

    void update() override
    {
        ++updateCount;
    }

    void setDrawDirection(subttxrend::gfx::DrawDirection dir) override
    {
        (void)dir;
    }

    void resetObservations()
    {
        visibleCalls.clear();
        resizeCalls.clear();
        clearCount = 0;
        updateCount = 0;
        drawContext.reset();
    }

    RecordingDrawContext drawContext;
    std::vector<bool> visibleCalls;
    std::vector<subttxrend::gfx::Size> resizeCalls;
    int clearCount {0};
    int updateCount {0};

private:
    subttxrend::gfx::Rectangle m_bounds;
    subttxrend::gfx::Size m_preferredSize;
    subttxrend::gfx::Size m_size;
};

class MapConfigProvider : public subttxrend::common::ConfigProvider
{
public:
    void setValue(const std::string& key,
                  const std::string& value)
    {
        m_values[key] = value;
    }

protected:
    const char* getValue(const std::string& key) const override
    {
        const auto found = m_values.find(key);
        return found == m_values.end() ? nullptr : found->second.c_str();
    }

private:
    std::map<std::string, std::string> m_values;
};

class ScopedTempFile
{
public:
    explicit ScopedTempFile(const std::string& contents)
        : m_path(makePath())
    {
        std::ofstream stream(m_path, std::ios::binary);
        stream << contents;
        CPPUNIT_ASSERT(stream.good());
    }

    ~ScopedTempFile()
    {
        std::remove(m_path.c_str());
    }

    const std::string& path() const
    {
        return m_path;
    }

private:
    static std::string makePath()
    {
        static unsigned counter = 0;
        ++counter;
        return "ttml_engine_impl_test_" + std::to_string(counter) + ".ttml";
    }

    std::string m_path;
};

std::string makeTimestamp(std::uint64_t totalMs)
{
    const std::uint64_t hours = totalMs / 3600000;
    totalMs %= 3600000;
    const std::uint64_t minutes = totalMs / 60000;
    totalMs %= 60000;
    const std::uint64_t seconds = totalMs / 1000;
    const std::uint64_t milliseconds = totalMs % 1000;

    std::ostringstream stream;
    stream << std::setw(2) << std::setfill('0') << hours
           << ':' << std::setw(2) << std::setfill('0') << minutes
           << ':' << std::setw(2) << std::setfill('0') << seconds
           << '.' << std::setw(3) << std::setfill('0') << milliseconds;
    return stream.str();
}

std::string makeSimpleTtmlDocument(std::uint64_t startMs,
                                   std::uint64_t endMs,
                                   const std::string& text,
                                   const std::string& spanAttributes = std::string(),
                                   const std::string& regionAttributes = std::string("tts:origin=\"0% 80%\" tts:extent=\"100% 20%\" tts:textAlign=\"center\""))
{
    std::ostringstream stream;
    stream << "<?xml version=\"1.0\"?>\n"
           << "<tt xml:lang=\"en\" xmlns=\"http://www.w3.org/ns/ttml\" xmlns:tts=\"http://www.w3.org/ns/ttml#styling\" xmlns:ttm=\"http://www.w3.org/ns/ttml#metadata\" xmlns:ttp=\"http://www.w3.org/ns/ttml#parameter\" ttp:profile=\"http://www.w3.org/ns/ttml/profile/imsc1/text\" ttp:cellResolution=\"40 24\">\n"
           << "  <head>\n"
           << "    <metadata />\n"
           << "    <layout>\n"
           << "      <region xml:id=\"r1\" " << regionAttributes << " />\n"
           << "    </layout>\n"
           << "  </head>\n"
           << "  <body>\n"
           << "    <div>\n"
           << "      <p begin=\"" << makeTimestamp(startMs) << "\" end=\"" << makeTimestamp(endMs) << "\" region=\"r1\">\n";
    if (spanAttributes.empty())
    {
        stream << "        <span tts:fontFamily=\"sans-serif\">" << text << "</span>\n";
    }
    else
    {
        stream << "        <span tts:fontFamily=\"sans-serif\" " << spanAttributes << ">" << text << "</span>\n";
    }
    stream << "      </p>\n"
           << "    </div>\n"
           << "  </body>\n"
           << "</tt>\n";
    return stream.str();
}

std::string makeEmptyTtmlDocument()
{
    return "<?xml version=\"1.0\"?>\n"
           "<tt xml:lang=\"en\" xmlns=\"http://www.w3.org/ns/ttml\" xmlns:tts=\"http://www.w3.org/ns/ttml#styling\" xmlns:ttm=\"http://www.w3.org/ns/ttml#metadata\" xmlns:ttp=\"http://www.w3.org/ns/ttml#parameter\" ttp:profile=\"http://www.w3.org/ns/ttml/profile/imsc1/text\" ttp:cellResolution=\"40 24\">\n"
           "  <body><div /></body>\n"
           "</tt>\n";
}

std::string makeImageTtmlDocument(std::uint64_t startMs,
                                  std::uint64_t endMs,
                                  const std::string& imageId,
                                  const std::string& regionAttributes = std::string("tts:origin=\"10% 20%\" tts:extent=\"30% 20%\""),
                                  const std::string& imageData = kSinglePixelPngBase64)
{
    std::ostringstream stream;
    stream << "<?xml version=\"1.0\"?>\n"
           << "<tt xml:lang=\"en\" xmlns=\"http://www.w3.org/ns/ttml\" xmlns:tts=\"http://www.w3.org/ns/ttml#styling\" xmlns:ttm=\"http://www.w3.org/ns/ttml#metadata\" xmlns:ttp=\"http://www.w3.org/ns/ttml#parameter\" ttp:profile=\"http://www.w3.org/ns/ttml/profile/imsc1/text\" ttp:cellResolution=\"40 24\">\n"
           << "  <head>\n"
           << "    <metadata>\n"
           << "      <image xml:id=\"" << imageId << "\">" << imageData << "</image>\n"
           << "    </metadata>\n"
           << "    <layout>\n"
           << "      <region xml:id=\"r1\" " << regionAttributes << " />\n"
           << "    </layout>\n"
           << "  </head>\n"
           << "  <body>\n"
           << "    <div>\n"
           << "      <p begin=\"" << makeTimestamp(startMs) << "\" end=\"" << makeTimestamp(endMs)
           << "\" region=\"r1\" backgroundImage=\"#" << imageId << "\"></p>\n"
           << "    </div>\n"
           << "  </body>\n"
           << "</tt>\n";
    return stream.str();
}

std::vector<std::uint8_t> makeBuffer(const std::string& text)
{
    return std::vector<std::uint8_t>(text.begin(), text.end());
}

void addDocument(TtmlEngineImpl& engine,
                 const std::string& document,
                 std::int64_t displayOffsetMs = 0)
{
    const auto buffer = makeBuffer(document);
    CPPUNIT_ASSERT(!buffer.empty());
    engine.addData(buffer.data(), buffer.size(), displayOffsetMs);
}

} // namespace

class TtmlEngineImplTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(TtmlEngineImplTest);
    CPPUNIT_TEST(testInitClearsAndIdles);
    CPPUNIT_TEST(testStartShowsAndClears);
    CPPUNIT_TEST(testStopHidesAndClears);
    CPPUNIT_TEST(testMuteAndUnmuteToggleVisibility);
    CPPUNIT_TEST(testGetWaitTimeIsZeroWithoutTimestamp);
    CPPUNIT_TEST(testGetWaitTimeUsesMinInterval);
    CPPUNIT_TEST(testProcessDoesNothingBeforeTimestamp);
    CPPUNIT_TEST(testProcessRendersImageCue);
    CPPUNIT_TEST(testAddDataAppliesPositiveDisplayOffset);
    CPPUNIT_TEST(testAddDataAppliesNegativeDisplayOffset);
    CPPUNIT_TEST(testEmptyDocKeepsQueuedCue);
    CPPUNIT_TEST(testOutOfOrderPacketsUseTimeline);
    CPPUNIT_TEST(testFlushClearsQueueAndDisplay);
    CPPUNIT_TEST(testPauseResumeFreezesMediaTimeProgress);
    CPPUNIT_TEST(testCurrentMediatimeResetsPausedDelay);
    CPPUNIT_TEST(testRenderResizesWindow);
    CPPUNIT_TEST(testSetSubtitleInfoLoadsFile);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
    }

    void tearDown() override
    {
    }

protected:
    struct TestContext
    {
        TestContext(subttxrend::gfx::Size preferredSize = subttxrend::gfx::Size(1920, 1080),
                    subttxrend::gfx::Size currentSize = subttxrend::gfx::Size(1920, 1080))
            : window(std::make_shared<RecordingWindow>(preferredSize, currentSize))
        {
        }

        void init(subttxrend::common::Properties properties = subttxrend::common::Properties())
        {
            engine.init(&configProvider, window.get(), properties);
        }

        MapConfigProvider configProvider;
        std::shared_ptr<RecordingWindow> window;
        TtmlEngineImpl engine;
    };

    static void startForPlayback(TestContext& context)
    {
        context.engine.setRelatedVideoSize(context.window->getPreferredSize());
        context.engine.start();
        context.window->resetObservations();
    }

    void testInitClearsAndIdles()
    {
        TestContext context;

        context.init();

        CPPUNIT_ASSERT(context.window->visibleCalls.empty());
        CPPUNIT_ASSERT(context.window->resizeCalls.empty());
        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(0LL, static_cast<long long>(context.engine.getWaitTime().count()));
    }

    void testStartShowsAndClears()
    {
        TestContext context;
        context.init();
        context.window->resetObservations();

        context.engine.start();

        CPPUNIT_ASSERT_EQUAL(std::size_t(1), context.window->visibleCalls.size());
        CPPUNIT_ASSERT(static_cast<bool>(context.window->visibleCalls.front()));
        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
    }

    void testStopHidesAndClears()
    {
        TestContext context;
        context.init();
        context.window->resetObservations();

        context.engine.stop();

        CPPUNIT_ASSERT_EQUAL(std::size_t(1), context.window->visibleCalls.size());
        CPPUNIT_ASSERT(!static_cast<bool>(context.window->visibleCalls.front()));
        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
    }

    void testMuteAndUnmuteToggleVisibility()
    {
        TestContext context;
        context.init();
        startForPlayback(context);

        context.engine.mute();
        context.engine.unmute();

        CPPUNIT_ASSERT_EQUAL(std::size_t(2), context.window->visibleCalls.size());
        CPPUNIT_ASSERT(!static_cast<bool>(context.window->visibleCalls[0]));
        CPPUNIT_ASSERT(static_cast<bool>(context.window->visibleCalls[1]));
    }

    void testGetWaitTimeIsZeroWithoutTimestamp()
    {
        TestContext context;
        context.init();
        startForPlayback(context);

        addDocument(context.engine, makeSimpleTtmlDocument(0, 300, "queued cue"));

        CPPUNIT_ASSERT_EQUAL(0LL, static_cast<long long>(context.engine.getWaitTime().count()));
    }

    void testGetWaitTimeUsesMinInterval()
    {
        TestContext context;
        context.init();
        startForPlayback(context);
        addDocument(context.engine, makeSimpleTtmlDocument(200, 400, "future cue"));

        context.engine.currentMediatime(0);

        CPPUNIT_ASSERT_EQUAL(25LL, static_cast<long long>(context.engine.getWaitTime().count()));
    }

    void testProcessDoesNothingBeforeTimestamp()
    {
        TestContext context;
        context.init();
        startForPlayback(context);
        addDocument(context.engine, makeSimpleTtmlDocument(0, 300, "queued cue"));
        context.window->resetObservations();

        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(0, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(0, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
    }

    void testProcessRendersImageCue()
    {
        TestContext context;
        context.init();
        startForPlayback(context);
        addDocument(context.engine, makeImageTtmlDocument(0, 300, "img-visible"));
        context.engine.currentMediatime(0);
        context.window->resetObservations();

        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), context.window->drawContext.bitmapCalls.size());
    }

    void testAddDataAppliesPositiveDisplayOffset()
    {
        TestContext context;
        context.init();
        startForPlayback(context);
        addDocument(context.engine, makeImageTtmlDocument(0, 100, "img-pos"), 200);
        context.engine.currentMediatime(150);
        context.window->resetObservations();

        context.engine.process();

        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(0, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(0, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(25LL, static_cast<long long>(context.engine.getWaitTime().count()));

        context.window->resetObservations();
        context.engine.currentMediatime(250);
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), context.window->drawContext.bitmapCalls.size());
    }

    void testAddDataAppliesNegativeDisplayOffset()
    {
        TestContext context;
        context.init();
        startForPlayback(context);
        addDocument(context.engine, makeImageTtmlDocument(200, 350, "img-neg"), -150);
        context.engine.currentMediatime(0);
        context.window->resetObservations();

        context.engine.process();

        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(0, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(0, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(25LL, static_cast<long long>(context.engine.getWaitTime().count()));

        context.window->resetObservations();
        context.engine.currentMediatime(100);
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), context.window->drawContext.bitmapCalls.size());
    }

    void testEmptyDocKeepsQueuedCue()
    {
        TestContext context;
        context.init();
        startForPlayback(context);
        addDocument(context.engine, makeImageTtmlDocument(200, 300, "img-keep"));
        addDocument(context.engine, makeEmptyTtmlDocument());
        context.engine.currentMediatime(0);
        context.window->resetObservations();

        CPPUNIT_ASSERT_NO_THROW(context.engine.process());

        CPPUNIT_ASSERT_EQUAL(0, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(0, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(25LL, static_cast<long long>(context.engine.getWaitTime().count()));

        context.window->resetObservations();
        context.engine.currentMediatime(250);
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), context.window->drawContext.bitmapCalls.size());
    }

    void testOutOfOrderPacketsUseTimeline()
    {
        TestContext context;
        context.init();
        startForPlayback(context);
        addDocument(context.engine,
                    makeImageTtmlDocument(150,
                                          350,
                                          "img-late",
                                          "tts:origin=\"0% 55%\" tts:extent=\"20% 20%\""));
        addDocument(context.engine,
                    makeImageTtmlDocument(0,
                                          60,
                                          "img-early",
                                          "tts:origin=\"0% 5%\" tts:extent=\"20% 20%\""));
        context.engine.currentMediatime(0);
        context.window->resetObservations();

        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), context.window->drawContext.bitmapCalls.size());
        const auto firstRect = context.window->drawContext.bitmapCalls.front().rectangle;

        context.window->resetObservations();
        context.engine.currentMediatime(200);
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), context.window->drawContext.bitmapCalls.size());
        CPPUNIT_ASSERT(firstRect.m_y < context.window->drawContext.bitmapCalls.front().rectangle.m_y);
    }

    void testFlushClearsQueueAndDisplay()
    {
        TestContext context;
        context.init();
        startForPlayback(context);
        addDocument(context.engine, makeImageTtmlDocument(0, 300, "img-flush"));
        context.engine.currentMediatime(0);
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), context.window->drawContext.bitmapCalls.size());
        context.window->resetObservations();

        context.engine.flush();
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(0LL, static_cast<long long>(context.engine.getWaitTime().count()));
    }

    void testPauseResumeFreezesMediaTimeProgress()
    {
        TestContext context;
        context.init();
        startForPlayback(context);
        addDocument(context.engine, makeSimpleTtmlDocument(50, 100, "pause works"));
        context.engine.currentMediatime(0);
        context.engine.pause();
        context.window->resetObservations();

        std::this_thread::sleep_for(120ms);
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(0, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(0, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(25LL, static_cast<long long>(context.engine.getWaitTime().count()));

        context.engine.resume();
        context.window->resetObservations();
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(0, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(0, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(25LL, static_cast<long long>(context.engine.getWaitTime().count()));
    }

    void testCurrentMediatimeResetsPausedDelay()
    {
        TestContext context;
        context.init();
        startForPlayback(context);
        addDocument(context.engine, makeImageTtmlDocument(150, 250, "img-reset"));
        context.engine.currentMediatime(0);
        context.engine.pause();
        std::this_thread::sleep_for(120ms);
        context.engine.resume();
        context.window->resetObservations();

        context.engine.currentMediatime(160);
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), context.window->drawContext.bitmapCalls.size());
    }

    void testRenderResizesWindow()
    {
        TestContext context;
        context.init();
        startForPlayback(context);
        addDocument(context.engine,
                    makeImageTtmlDocument(0,
                                          300,
                                          "img-resize",
                                          "tts:origin=\"0% 0%\" tts:extent=\"100% 100%\"",
                                          kTwoByTwoPngBase64));
        context.engine.currentMediatime(0);
        context.window->resetObservations();

        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(std::size_t(1), context.window->resizeCalls.size());
        CPPUNIT_ASSERT_EQUAL(2, context.window->resizeCalls.front().m_w);
        CPPUNIT_ASSERT_EQUAL(2, context.window->resizeCalls.front().m_h);
        CPPUNIT_ASSERT_EQUAL(2, context.window->getSize().m_w);
        CPPUNIT_ASSERT_EQUAL(2, context.window->getSize().m_h);
    }

    void testSetSubtitleInfoLoadsFile()
    {
        const auto fileDocument = makeImageTtmlDocument(0, 300, "img-file");
        ScopedTempFile file(fileDocument);

        TestContext context;
        context.configProvider.setValue("READ_FROM_FILE", file.path());
        context.init();
        startForPlayback(context);
        context.engine.currentMediatime(0);

        addDocument(context.engine, makeSimpleTtmlDocument(0, 300, "ignored cue"));
        CPPUNIT_ASSERT_EQUAL(25LL, static_cast<long long>(context.engine.getWaitTime().count()));

        context.window->resetObservations();
        context.engine.process();
        CPPUNIT_ASSERT_EQUAL(0, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(0, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());

        context.engine.setSubtitleInfo("vod", "eng nrm");
        CPPUNIT_ASSERT_EQUAL(25LL, static_cast<long long>(context.engine.getWaitTime().count()));

        context.window->resetObservations();
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(std::size_t(1), context.window->drawContext.bitmapCalls.size());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TtmlEngineImplTest);