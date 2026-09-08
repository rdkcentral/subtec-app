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

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>

#include <WebVTTAttributes.hpp>
#include <WebvttEngineImpl.hpp>

using namespace std::chrono_literals;
using namespace subttxrend::webvttengine;

namespace subttxrend
{
namespace gfx
{

const ColorArgb ColorArgb::TRANSPARENT(0, 0, 0, 0);

} // namespace gfx
} // namespace subttxrend

class WebvttEngineImplTest : public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(WebvttEngineImplTest);
    CPPUNIT_TEST(testInitInitializesRendererAndClearsState);
    CPPUNIT_TEST(testStartClearsQueuedDataAndShowsWindow);
    CPPUNIT_TEST(testStopClearsQueuedDataAndHidesWindow);
    CPPUNIT_TEST(testMuteAndUnmuteToggleVisibility);
    CPPUNIT_TEST(testGetWaitTimeIsZeroWithoutRunnableWork);
    CPPUNIT_TEST(testGetWaitTimeReflectsPendingAndPausedStates);
    CPPUNIT_TEST(testAddDataNormalizesLineEndingsAndTrailingPadding);
    CPPUNIT_TEST(testAddDataAppliesDisplayOffset);
    CPPUNIT_TEST(testAddDataMalformedDocumentDoesNotDiscardValidTimeline);
    CPPUNIT_TEST(testProcessDoesNothingBeforeTimestamp);
    CPPUNIT_TEST(testProcessRendersActiveCueOnlyOnce);
    CPPUNIT_TEST(testProcessClearsExpiredCueOnTimeJump);
    CPPUNIT_TEST(testProcessKeepsFutureCuePendingUntilStartTime);
    CPPUNIT_TEST(testAddDataMergesPacketsByCueStartTime);
    CPPUNIT_TEST(testPauseResumeFreezesMediaTimeProgress);
    CPPUNIT_TEST(testPauseResumeIsIdempotent);
    CPPUNIT_TEST(testCurrentMediatimeResetsPausedDelay);
    CPPUNIT_TEST(testSetAttributesAffectRenderedWindowColor);
    CPPUNIT_TEST(testProcessSuppressesDuplicateActiveCues);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp()
    {
    }

    void tearDown()
    {
    }

protected:
    struct FillCall
    {
        subttxrend::gfx::ColorArgb color;
        subttxrend::gfx::Rectangle rectangle;
    };

    struct DrawStringCall
    {
        subttxrend::gfx::Rectangle rectangle;
    };

    class RecordingDrawContext : public subttxrend::gfx::DrawContext
    {
    public:
        std::vector<FillCall> fillCalls;
        std::vector<DrawStringCall> drawStringCalls;

        void fillRectangle(subttxrend::gfx::ColorArgb color,
                           const subttxrend::gfx::Rectangle& rectangle) override
        {
            fillCalls.push_back({color, rectangle});
        }

        void drawString(subttxrend::gfx::PrerenderedFont& font,
                        const subttxrend::gfx::Rectangle& rectangle,
                        const std::vector<subttxrend::gfx::GlyphData>& glyphs,
                        const subttxrend::gfx::ColorArgb foreground,
                        const subttxrend::gfx::ColorArgb background,
                        int outlineSize = 0,
                        int verticalOffset = 0) override
        {
            (void)font;
            (void)glyphs;
            (void)outlineSize;
            (void)verticalOffset;
            (void)foreground;
            (void)background;
            drawStringCalls.push_back({rectangle});
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
            (void)dstRect;
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

        void reset()
        {
            fillCalls.clear();
            drawStringCalls.clear();
        }
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
            clearCount = 0;
            updateCount = 0;
            drawContext.reset();
        }

        RecordingDrawContext drawContext;
        std::vector<bool> visibleCalls;
        int clearCount {0};
        int updateCount {0};

    private:
        subttxrend::gfx::Rectangle m_bounds;
        subttxrend::gfx::Size m_preferredSize;
        subttxrend::gfx::Size m_size;
    };

    struct TestContext
    {
        TestContext()
            : window(std::make_shared<RecordingWindow>(subttxrend::gfx::Size(1280, 720),
                                                       subttxrend::gfx::Size(1280, 720)))
        {
            engine.init(&configProvider, window);
        }

        subttxrend::common::ConfigProvider configProvider;
        std::shared_ptr<RecordingWindow> window;
        WebvttEngineImpl engine;
    };

    static void startForPlayback(TestContext& context)
    {
        context.engine.start();
        context.window->resetObservations();
    }

    static std::string makeTimestamp(std::uint64_t totalMs)
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

    static std::string makeCueDocument(std::uint64_t startMs,
                                       std::uint64_t endMs,
                                       const std::string& cueText)
    {
        std::ostringstream stream;
        stream << "WEBVTT\n\n"
               << makeTimestamp(startMs)
               << " --> "
               << makeTimestamp(endMs)
               << "\n"
               << cueText
               << "\n\n";
        return stream.str();
    }

    static std::vector<std::uint8_t> makeBuffer(const std::string& text,
                                                std::size_t trailingNulls = 0)
    {
        std::vector<std::uint8_t> buffer(text.begin(), text.end());
        buffer.insert(buffer.end(), trailingNulls, static_cast<std::uint8_t>(0));
        return buffer;
    }

    static void addDocument(WebvttEngineImpl& engine,
                            const std::string& text,
                            std::int64_t displayOffsetMs = 0,
                            std::size_t trailingNulls = 0)
    {
        const auto buffer = makeBuffer(text, trailingNulls);
        CPPUNIT_ASSERT(!buffer.empty());
        engine.addData(buffer.data(), buffer.size(), displayOffsetMs);
    }

    static bool hasFillColor(const RecordingDrawContext& drawContext,
                             const subttxrend::gfx::ColorArgb& expectedColor)
    {
        return std::any_of(drawContext.fillCalls.begin(),
                           drawContext.fillCalls.end(),
                           [&expectedColor](const FillCall& fillCall) {
                               return fillCall.color == expectedColor;
                           });
    }

    void testInitInitializesRendererAndClearsState()
    {
        TestContext context;

        CPPUNIT_ASSERT_EQUAL(std::size_t(1), context.window->visibleCalls.size());
        CPPUNIT_ASSERT(static_cast<bool>(context.window->visibleCalls.front()));
        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(0LL, static_cast<long long>(context.engine.getWaitTime().count()));
    }

    void testStartClearsQueuedDataAndShowsWindow()
    {
        TestContext context;
        addDocument(context.engine, makeCueDocument(0, 300, "stale cue"));
        context.window->resetObservations();

        context.engine.start();
        context.engine.currentMediatime(0);
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(std::size_t(1), context.window->visibleCalls.size());
        CPPUNIT_ASSERT(static_cast<bool>(context.window->visibleCalls.front()));
        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(0LL, static_cast<long long>(context.engine.getWaitTime().count()));
    }

    void testStopClearsQueuedDataAndHidesWindow()
    {
        TestContext context;
        addDocument(context.engine, makeCueDocument(0, 300, "stale cue"));
        context.window->resetObservations();

        context.engine.stop();
        context.engine.currentMediatime(0);
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(std::size_t(1), context.window->visibleCalls.size());
        CPPUNIT_ASSERT(!static_cast<bool>(context.window->visibleCalls.front()));
        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(0LL, static_cast<long long>(context.engine.getWaitTime().count()));
    }

    void testMuteAndUnmuteToggleVisibility()
    {
        TestContext context;
        startForPlayback(context);
        addDocument(context.engine, makeCueDocument(0, 300, "mute restore"));
        context.engine.currentMediatime(0);
        context.window->resetObservations();

        context.engine.mute();
        context.engine.unmute();
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(std::size_t(2), context.window->visibleCalls.size());
        CPPUNIT_ASSERT(!static_cast<bool>(context.window->visibleCalls[0]));
        CPPUNIT_ASSERT(static_cast<bool>(context.window->visibleCalls[1]));
        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(std::size_t(2), context.window->drawContext.drawStringCalls.size());
    }

    void testGetWaitTimeIsZeroWithoutRunnableWork()
    {
        TestContext context;

        CPPUNIT_ASSERT_EQUAL(0LL, static_cast<long long>(context.engine.getWaitTime().count()));

        addDocument(context.engine, makeCueDocument(0, 300, "no timestamp"));

        CPPUNIT_ASSERT_EQUAL(0LL, static_cast<long long>(context.engine.getWaitTime().count()));
    }

    void testGetWaitTimeReflectsPendingAndPausedStates()
    {
        TestContext context;
        addDocument(context.engine, makeCueDocument(200, 500, "wait state"));
        context.engine.currentMediatime(0);

        CPPUNIT_ASSERT_EQUAL(25LL, static_cast<long long>(context.engine.getWaitTime().count()));

        context.engine.pause();
        CPPUNIT_ASSERT_EQUAL(250LL, static_cast<long long>(context.engine.getWaitTime().count()));

        context.engine.resume();
        CPPUNIT_ASSERT_EQUAL(25LL, static_cast<long long>(context.engine.getWaitTime().count()));
    }

    void testAddDataNormalizesLineEndingsAndTrailingPadding()
    {
        TestContext context;
        startForPlayback(context);
        const std::string document = "WEBVTT\r\n\r\n00:00:00.000 --> 00:00:00.300\r\nnormalized text\r\n\r\n";
        addDocument(context.engine, document, 0, 2);
        context.engine.currentMediatime(0);
        context.window->resetObservations();

        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(std::size_t(2), context.window->drawContext.drawStringCalls.size());
    }

    void testAddDataAppliesDisplayOffset()
    {
        TestContext context;
        startForPlayback(context);
        addDocument(context.engine,
                    "WEBVTT\n"
                    "X-TIMESTAMP-MAP=MPEGTS:18000,LOCAL:00:00:00.000\n\n"
                    "00:00:00.000 --> 00:00:00.100\n"
                    "offset cue\n\n",
                    0);
        context.engine.currentMediatime(0);
        context.window->resetObservations();

        context.engine.process();
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(0, context.window->updateCount);

        context.engine.currentMediatime(250);
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(std::size_t(2), context.window->drawContext.drawStringCalls.size());
    }

    void testAddDataMalformedDocumentDoesNotDiscardValidTimeline()
    {
        TestContext context;
        startForPlayback(context);
        addDocument(context.engine, makeCueDocument(0, 300, "valid cue"));
        addDocument(context.engine,
                    "WEBVTT\n\n00:00:00.000 00:00:00.200\nmissing arrow\n\n");
        context.engine.currentMediatime(0);
        context.window->resetObservations();

        CPPUNIT_ASSERT_NO_THROW(context.engine.process());

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(std::size_t(2), context.window->drawContext.drawStringCalls.size());
    }

    void testProcessDoesNothingBeforeTimestamp()
    {
        TestContext context;
        startForPlayback(context);
        addDocument(context.engine, makeCueDocument(0, 300, "queued cue"));
        context.window->resetObservations();

        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(0, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(0, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
    }

    void testProcessRendersActiveCueOnlyOnce()
    {
        TestContext context;
        startForPlayback(context);
        addDocument(context.engine, makeCueDocument(0, 500, "visible once"));
        context.engine.currentMediatime(0);
        context.window->resetObservations();

        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(std::size_t(2), context.window->drawContext.drawStringCalls.size());

        context.window->resetObservations();
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(0, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(0, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
    }

    void testProcessClearsExpiredCueOnTimeJump()
    {
        TestContext context;
        startForPlayback(context);
        addDocument(context.engine, makeCueDocument(0, 50, "short cue"));
        context.engine.currentMediatime(0);
        context.engine.process();
        context.window->resetObservations();

        context.engine.currentMediatime(75);
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
    }

    void testProcessKeepsFutureCuePendingUntilStartTime()
    {
        TestContext context;
        startForPlayback(context);
        addDocument(context.engine, makeCueDocument(200, 400, "future cue"));
        context.engine.currentMediatime(0);
        context.window->resetObservations();

        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(0, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(0, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());
        CPPUNIT_ASSERT_EQUAL(25LL, static_cast<long long>(context.engine.getWaitTime().count()));

        context.engine.currentMediatime(250);
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(std::size_t(2), context.window->drawContext.drawStringCalls.size());
    }

    void testAddDataMergesPacketsByCueStartTime()
    {
        TestContext context;
        startForPlayback(context);
        addDocument(context.engine, makeCueDocument(150, 350, "late packet cue"));
        addDocument(context.engine, makeCueDocument(0, 60, "early cue"));
        context.engine.currentMediatime(0);
        context.window->resetObservations();

        context.engine.process();
        CPPUNIT_ASSERT_EQUAL(std::size_t(2), context.window->drawContext.drawStringCalls.size());

        context.window->resetObservations();
        context.engine.currentMediatime(200);
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(std::size_t(3), context.window->drawContext.drawStringCalls.size());
    }

    void testPauseResumeFreezesMediaTimeProgress()
    {
        TestContext context;
        startForPlayback(context);
        addDocument(context.engine, makeCueDocument(0, 60, "pause works"));
        context.engine.currentMediatime(0);
        context.engine.pause();
        context.window->resetObservations();

        std::this_thread::sleep_for(120ms);
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(0, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(0, context.window->updateCount);
        CPPUNIT_ASSERT(context.window->drawContext.drawStringCalls.empty());

        context.engine.resume();
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(std::size_t(2), context.window->drawContext.drawStringCalls.size());
    }

    void testPauseResumeIsIdempotent()
    {
        TestContext context;
        startForPlayback(context);
        addDocument(context.engine, makeCueDocument(0, 250, "idempotent pause"));
        context.engine.currentMediatime(0);

        context.engine.pause();
        std::this_thread::sleep_for(60ms);
        context.engine.pause();
        std::this_thread::sleep_for(60ms);
        context.engine.resume();
        context.engine.resume();

        context.window->resetObservations();
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(std::size_t(2), context.window->drawContext.drawStringCalls.size());
    }

    void testCurrentMediatimeResetsPausedDelay()
    {
        TestContext context;
        startForPlayback(context);
        addDocument(context.engine, makeCueDocument(90, 240, "timestamp reset"));
        context.engine.currentMediatime(0);
        context.engine.pause();
        std::this_thread::sleep_for(120ms);
        context.engine.resume();

        context.window->resetObservations();
        context.engine.currentMediatime(100);
        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(std::size_t(2), context.window->drawContext.drawStringCalls.size());
    }

    void testSetAttributesAffectRenderedWindowColor()
    {
        TestContext context;
        startForPlayback(context);
        WebVTTAttributes attributes;
        attributes.setInteger(WebVTTAttributes::AttributeType::WINDOW_COLOR, 0x00010203);
        attributes.setInteger(WebVTTAttributes::AttributeType::WINDOW_OPACITY,
                              static_cast<std::uint32_t>(WebVTTAttributes::Opacity::SOLID));

        context.engine.setAttributes(attributes);
        addDocument(context.engine, makeCueDocument(0, 300, "color cue"));
        context.engine.currentMediatime(0);
        context.window->resetObservations();

        context.engine.process();

        CPPUNIT_ASSERT(hasFillColor(context.window->drawContext,
                                    subttxrend::gfx::ColorArgb(255, 1, 2, 3)));
    }

    void testProcessSuppressesDuplicateActiveCues()
    {
        TestContext context;
        startForPlayback(context);
        const std::string duplicateDocument = makeCueDocument(0, 300, "same cue");
        addDocument(context.engine, duplicateDocument);
        addDocument(context.engine, duplicateDocument);
        context.engine.currentMediatime(0);
        context.window->resetObservations();

        context.engine.process();

        CPPUNIT_ASSERT_EQUAL(1, context.window->clearCount);
        CPPUNIT_ASSERT_EQUAL(1, context.window->updateCount);
        CPPUNIT_ASSERT_EQUAL(std::size_t(2), context.window->drawContext.drawStringCalls.size());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(WebvttEngineImplTest);