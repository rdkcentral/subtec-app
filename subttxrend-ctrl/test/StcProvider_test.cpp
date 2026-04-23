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
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <limits>
#include <sys/time.h>
#include "StcProvider.hpp"

using namespace subttxrend::ctrl;

class StcProviderTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(StcProviderTest);

    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testInitialStcValue);
    CPPUNIT_TEST(testProcessTimestampBasic);
    CPPUNIT_TEST(testProcessTimestampMultipleUpdates);
    CPPUNIT_TEST(testProcessTimestampWithZeroStc);
    CPPUNIT_TEST(testProcessTimestampWithMaxStc);
    CPPUNIT_TEST(testProcessTimestampWithZeroTimestamp);
    CPPUNIT_TEST(testProcessTimestampWithMaxTimestamp);
    CPPUNIT_TEST(testProcessTimestampOverwrite);
    CPPUNIT_TEST(testGetStcImmediatelyAfterProcessTimestamp);
    CPPUNIT_TEST(testGetStcAfterSmallDelay);
    CPPUNIT_TEST(testGetStcAfterLargeDelay);
    CPPUNIT_TEST(testGetStcTimeDiffExactlyOneHour);
    CPPUNIT_TEST(testGetStcTimeDiffSlightlyUnderOneHour);
    CPPUNIT_TEST(testGetStcWithoutPriorProcessTimestamp);
    CPPUNIT_TEST(testGetStcCalculationAccuracy);
    CPPUNIT_TEST(testGetStcStcOverflow);
    CPPUNIT_TEST(testStcCallbackWithValidInstance);
    CPPUNIT_TEST(testStcCallbackMatchesDirectCall);
    CPPUNIT_TEST(testConcurrentProcessTimestampCalls);
    CPPUNIT_TEST(testConcurrentGetStcCalls);
    CPPUNIT_TEST(testConcurrentProcessAndGetStc);
    CPPUNIT_TEST(testMultiThreadedStressTest);
    CPPUNIT_TEST(testFullWorkflowWithRealTimeDelay);
    CPPUNIT_TEST(testMultipleUpdatesAndRetrievals);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
    {
        // No specific setup needed
    }

    void tearDown() override
    {
        // No specific cleanup needed
    }

protected:
    // Helper function to get current timestamp in milliseconds
    std::uint64_t getCurrentTimestampMs() const
    {
        struct timeval tv = timeval();
        gettimeofday(&tv, NULL);
        const std::uint64_t clockSec = tv.tv_sec;
        const std::uint64_t clockUsec = tv.tv_usec;
        return (clockSec * 1000) + (clockUsec / 1000);
    }

    void testConstructor()
    {
        // Test that constructor completes without errors
        StcProvider provider;
        // No exception means success - object is created
        CPPUNIT_ASSERT(true);
    }

    void testInitialStcValue()
    {
        // Test that getStc returns 0 or valid value immediately after construction
        StcProvider provider;
        std::uint32_t stc = provider.getStc();

        // Initial STC should be 0 since no timestamp has been processed
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), stc);
    }

    void testProcessTimestampBasic()
    {
        StcProvider provider;
        std::uint64_t currentTime = getCurrentTimestampMs() - 5; // 5ms in the past
        std::uint32_t testStc = 1000;

        provider.processTimestamp(testStc, currentTime);

        // Get STC immediately - should be approximately the same as testStc plus small delta
        std::uint32_t retrievedStc = provider.getStc();

        // Allow for 5ms offset: 1000 + (5ms * 45) = 1225, plus execution time
        CPPUNIT_ASSERT(retrievedStc >= 1000 && retrievedStc <= 1500);
    }

    void testProcessTimestampMultipleUpdates()
    {
        StcProvider provider;
        std::uint64_t currentTime = getCurrentTimestampMs() - 50; // Start in the past

        provider.processTimestamp(1000, currentTime);
        provider.processTimestamp(2000, currentTime + 10);
        provider.processTimestamp(3000, currentTime + 20);

        std::uint32_t stc = provider.getStc();

        // Should use the most recent values (3000) plus time delta from currentTime+20
        // Delta is ~30-40ms, so ~1350-1800 STC units
        CPPUNIT_ASSERT(stc >= 3000 && stc <= 5000);
    }

    void testProcessTimestampWithZeroStc()
    {
        StcProvider provider;
        std::uint64_t currentTime = getCurrentTimestampMs();

        provider.processTimestamp(0, currentTime);
        std::uint32_t stc = provider.getStc();

        // Zero is a valid STC value
        CPPUNIT_ASSERT(stc < 100); // Should be close to 0
    }

    void testProcessTimestampWithMaxStc()
    {
        StcProvider provider;
        std::uint64_t currentTime = getCurrentTimestampMs();
        std::uint32_t maxStc = std::numeric_limits<std::uint32_t>::max();

        provider.processTimestamp(maxStc, currentTime);
        std::uint32_t stc = provider.getStc();

        // Should accept and store max value
        // Allow small delta for execution time
        CPPUNIT_ASSERT(stc >= maxStc - 1000 || stc < 1000); // Account for overflow
    }

    void testProcessTimestampWithZeroTimestamp()
    {
        StcProvider provider;

        provider.processTimestamp(5000, 0);

        // Should accept zero timestamp
        // Note: getStc will calculate large diff from current time to timestamp 0
        // This should trigger the > 1 hour check and return just the base STC
        std::uint32_t stc = provider.getStc();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(5000), stc);
    }

    void testProcessTimestampWithMaxTimestamp()
    {
        StcProvider provider;
        std::uint64_t maxTimestamp = std::numeric_limits<std::uint64_t>::max();

        provider.processTimestamp(7000, maxTimestamp);

        // Should store the timestamp - calculation in getStc will handle it
        // The diff will underflow and become huge, triggering > 1 hour check
        std::uint32_t stc = provider.getStc();
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(7000), stc);
    }

    void testProcessTimestampOverwrite()
    {
        StcProvider provider;
        std::uint64_t currentTime = getCurrentTimestampMs();

        provider.processTimestamp(1000, currentTime);
        // Overwrite with same timestamp
        provider.processTimestamp(2000, currentTime);

        std::uint32_t stc = provider.getStc();

        // Should reflect the second call (2000)
        CPPUNIT_ASSERT(stc >= 1900 && stc <= 2100);
    }

    void testGetStcImmediatelyAfterProcessTimestamp()
    {
        StcProvider provider;
        std::uint64_t currentTime = getCurrentTimestampMs() - 5; // 5ms in the past
        std::uint32_t baseStc = 10000;

        provider.processTimestamp(baseStc, currentTime);
        std::uint32_t stc = provider.getStc();

        // Should be close to baseStc plus small delta: 10000 + (5ms * 45) = 10225
        CPPUNIT_ASSERT(stc >= 10000 && stc <= 10500);
    }

    void testGetStcAfterSmallDelay()
    {
        StcProvider provider;
        std::uint64_t startTime = getCurrentTimestampMs();
        std::uint32_t baseStc = 50000;

        // Set timestamp to current time to avoid negative deltas
        provider.processTimestamp(baseStc, startTime);

        // Wait 100ms
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Measure actual elapsed time
        std::uint64_t endTime = getCurrentTimestampMs();
        std::uint64_t actualElapsedMs = endTime - startTime;

        std::uint32_t stc = provider.getStc();

        // Calculate expected STC based on actual elapsed time
        // Expected increment: actualElapsedMs * 45 (45kHz units)
        std::uint32_t expectedIncrement = actualElapsedMs * 45;
        std::uint32_t expectedStc = baseStc + expectedIncrement;

        // Allow ±20ms tolerance (±900 STC units) for timing variations and system overhead
        std::uint32_t tolerance = 20 * 45;  // 900 units
        std::uint32_t minStc = (expectedStc > tolerance) ? (expectedStc - tolerance) : 0;
        std::uint32_t maxStc = expectedStc + tolerance;

        CPPUNIT_ASSERT(stc >= minStc && stc <= maxStc);
    }

    void testGetStcAfterLargeDelay()
    {
        StcProvider provider;
        std::uint64_t oldTime = getCurrentTimestampMs() - 7200000; // 2 hours ago (well over 1 hour threshold)

        provider.processTimestamp(20000, oldTime);

        std::uint32_t stc = provider.getStc();

        // Time diff > 1 hour (3600000ms), so should skip update and return just base STC
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(20000), stc);
    }

    void testGetStcTimeDiffExactlyOneHour()
    {
        StcProvider provider;
        std::uint64_t oneHourAgo = getCurrentTimestampMs() - (60 * 60 * 1000);

        provider.processTimestamp(30000, oneHourAgo);

        std::uint32_t stc = provider.getStc();

        // At exactly 1 hour, the condition is NOT > 1 hour, so update IS applied
        // Expected: 30000 + (3600000ms * 45) = 30000 + 162000000 = 162030000
        // Allow tolerance for timing variations (±1 second = ±45000)
        CPPUNIT_ASSERT(stc >= 161985000 && stc <= 162075000);
    }

    void testGetStcTimeDiffSlightlyUnderOneHour()
    {
        StcProvider provider;
        // Clearly under 1 hour: 55 minutes (3300000 ms), well below 3600000 ms threshold
        std::uint64_t underOneHour = getCurrentTimestampMs() - (55 * 60 * 1000);

        provider.processTimestamp(40000, underOneHour);

        std::uint32_t stc = provider.getStc();

        // Should apply the update: 3300000 ms * 45 = 148,500,000
        // Expected: 40000 + 148,500,000 = 148,540,000
        // Allow tolerance for timing variations (±5 minutes worth)
        std::uint32_t expectedMin = 40000 + 135000000;  // ~50 min
        std::uint32_t expectedMax = 40000 + 162000000;  // ~60 min
        CPPUNIT_ASSERT(stc >= expectedMin && stc <= expectedMax);
    }

    void testGetStcWithoutPriorProcessTimestamp()
    {
        StcProvider provider;

        // Call getStc without calling processTimestamp first
        std::uint32_t stc = provider.getStc();

        // Should handle gracefully - members are initialized to 0
        // Time diff will be large, triggering > 1 hour check
        CPPUNIT_ASSERT_EQUAL(static_cast<std::uint32_t>(0), stc);
    }

    void testGetStcCalculationAccuracy()
    {
        StcProvider provider;
        std::uint64_t currentTime = getCurrentTimestampMs();
        std::uint32_t baseStc = 100000;

        provider.processTimestamp(baseStc, currentTime - 1000); // 1 second ago (safely in past)

        std::uint32_t stc = provider.getStc();

        // Expected: baseStc + (1000ms * 45) = 100000 + 45000 = 145000
        // Allow ±10% tolerance for timing variations
        CPPUNIT_ASSERT(stc >= 140000 && stc <= 150000);
    }

    void testGetStcStcOverflow()
    {
        StcProvider provider;
        std::uint64_t currentTime = getCurrentTimestampMs();

        // Set STC near max value
        std::uint32_t nearMaxStc = std::numeric_limits<std::uint32_t>::max() - 1000;
        provider.processTimestamp(nearMaxStc, currentTime - 100); // 100ms ago

        std::uint32_t stc = provider.getStc();

        // Expected: nearMaxStc + (100 * 45) = nearMaxStc + 4500
        // This will overflow uint32_t and wrap around
        std::uint32_t expectedStc = nearMaxStc + 4500; // Will wrap

        // Allow tolerance for timing
        std::uint32_t delta = (stc > expectedStc) ? (stc - expectedStc) : (expectedStc - stc);
        CPPUNIT_ASSERT(delta < 1000);
    }

    void testStcCallbackWithValidInstance()
    {
        StcProvider provider;
        std::uint64_t currentTime = getCurrentTimestampMs();

        provider.processTimestamp(60000, currentTime);

        std::uint32_t callbackResult = StcProvider::stcCallback(&provider);

        // Should return valid STC value
        CPPUNIT_ASSERT(callbackResult >= 59000 && callbackResult <= 61000);
    }

    void testStcCallbackMatchesDirectCall()
    {
        StcProvider provider;
        std::uint64_t currentTime = getCurrentTimestampMs();

        provider.processTimestamp(70000, currentTime);

        std::uint32_t directCall = provider.getStc();
        std::uint32_t callbackCall = StcProvider::stcCallback(&provider);

        // Both should return very similar values (allow small timing difference)
        std::uint32_t delta = (directCall > callbackCall) ?
                              (directCall - callbackCall) : (callbackCall - directCall);
        CPPUNIT_ASSERT(delta < 100);
    }

    void testConcurrentProcessTimestampCalls()
    {
        StcProvider provider;
        std::atomic<bool> testComplete{false};
        std::atomic<int> writeCount{0};

        auto writer = [&provider, &testComplete, &writeCount]() {
            for (int i = 0; i < 100; ++i) {
                std::uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                provider.processTimestamp(1000 + i, timestamp);
                writeCount++;
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
            testComplete = true;
        };

        std::thread t1(writer);
        std::thread t2(writer);

        t1.join();
        t2.join();

        // Should complete without crashes
        CPPUNIT_ASSERT_EQUAL(true, testComplete.load());
        CPPUNIT_ASSERT_EQUAL(200, writeCount.load());
    }

    void testConcurrentGetStcCalls()
    {
        StcProvider provider;
        std::uint64_t currentTime = getCurrentTimestampMs();
        provider.processTimestamp(80000, currentTime);

        std::atomic<bool> testComplete{false};
        std::atomic<int> readCount{0};
        std::atomic<bool> hasError{false};

        auto reader = [&provider, &testComplete, &readCount, &hasError]() {
            while (!testComplete) {
                std::uint32_t stc = provider.getStc();
                // STC should be reasonable
                if (stc < 79000 || stc > 200000) {
                    hasError = true;
                }
                readCount++;
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        };

        std::thread t1(reader);
        std::thread t2(reader);
        std::thread t3(reader);

        // Let readers run for a short time
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        testComplete = true;

        t1.join();
        t2.join();
        t3.join();

        // No errors should occur
        CPPUNIT_ASSERT_EQUAL(false, hasError.load());
        CPPUNIT_ASSERT(readCount.load() > 0);
    }

    void testConcurrentProcessAndGetStc()
    {
        StcProvider provider;
        std::uint64_t baseTime = getCurrentTimestampMs();
        provider.processTimestamp(90000, baseTime);

        std::atomic<bool> testComplete{false};
        std::atomic<bool> hasInconsistency{false};

        auto writer = [&provider, &testComplete, baseTime]() {
            for (int i = 0; i < 200; ++i) {
                provider.processTimestamp(90000 + i * 100, baseTime + i * 10);
                std::this_thread::sleep_for(std::chrono::microseconds(50));
            }
            testComplete = true;
        };

        auto reader = [&provider, &testComplete, &hasInconsistency]() {
            std::uint32_t previousStc = 0;
            while (!testComplete) {
                std::uint32_t currentStc = provider.getStc();
                // STC should generally increase (or wrap around)
                // Just verify we don't crash and get reasonable values
                if (currentStc > 0 && currentStc < 1000000) {
                    // Value looks reasonable
                }
                previousStc = currentStc;
                std::this_thread::sleep_for(std::chrono::microseconds(50));
            }
        };

        std::thread t1(writer);
        std::thread t2(reader);
        std::thread t3(reader);

        t1.join();
        t2.join();
        t3.join();

        // Should complete without crashes
        CPPUNIT_ASSERT_EQUAL(true, testComplete.load());
    }

    void testMultiThreadedStressTest()
    {
        StcProvider provider;
        std::uint64_t baseTime = getCurrentTimestampMs();
        provider.processTimestamp(100000, baseTime);

        std::atomic<bool> testComplete{false};
        std::atomic<int> totalOperations{0};

        auto mixedOperations = [&provider, &testComplete, &totalOperations, baseTime]() {
            for (int i = 0; i < 50; ++i) {
                // Mix of operations
                provider.processTimestamp(100000 + i * 50, baseTime + i * 5);
                totalOperations++;

                std::uint32_t stc = provider.getStc();
                totalOperations++;
                (void)stc; // Suppress unused warning

                void* inst = &provider;
                std::uint32_t cbStc = StcProvider::stcCallback(inst);
                totalOperations++;
                (void)cbStc; // Suppress unused warning

                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        };

        std::vector<std::thread> threads;
        for (int i = 0; i < 4; ++i) {
            threads.emplace_back(mixedOperations);
        }

        for (auto& thread : threads) {
            thread.join();
        }

        testComplete = true;

        // Should complete without crashes
        CPPUNIT_ASSERT_EQUAL(600, totalOperations.load()); // 4 threads * 50 iterations * 3 ops
    }

    void testFullWorkflowWithRealTimeDelay()
    {
        StcProvider provider;

        // Simulate receiving a timestamp packet (use current timestamp)
        std::uint64_t timestamp1 = getCurrentTimestampMs();
        provider.processTimestamp(50000, timestamp1);

        // Verify immediate retrieval
        std::uint32_t stc1 = provider.getStc();
        // Should be very close to base value (allow 30ms tolerance for execution time delta)
        std::uint32_t tolerance1 = 30 * 45;  // 1350 units
        CPPUNIT_ASSERT(stc1 >= 50000 && stc1 <= 50000 + tolerance1);

        // Wait 50ms
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Measure actual elapsed time from timestamp1
        std::uint64_t timestamp2_before = getCurrentTimestampMs();
        std::uint64_t actualElapsedMs = timestamp2_before - timestamp1;

        // Get STC - should reflect time passage
        std::uint32_t stc2 = provider.getStc();

        // Calculate expected STC based on actual elapsed time
        std::uint32_t expectedIncrement = actualElapsedMs * 45;
        std::uint32_t expectedStc2 = 50000 + expectedIncrement;

        // Allow ±25ms tolerance (±1125 STC units) for timing variations
        std::uint32_t tolerance2 = 25 * 45;  // 1125 units
        std::uint32_t minStc2 = (expectedStc2 > tolerance2) ? (expectedStc2 - tolerance2) : 0;
        std::uint32_t maxStc2 = expectedStc2 + tolerance2;

        CPPUNIT_ASSERT(stc2 >= minStc2 && stc2 <= maxStc2);

        // Process new timestamp (use current timestamp)
        std::uint64_t timestamp2 = getCurrentTimestampMs();
        provider.processTimestamp(60000, timestamp2);

        // Get STC again
        std::uint32_t stc3 = provider.getStc();
        // Should be very close to new base value (allow 30ms tolerance for execution time delta)
        std::uint32_t tolerance3 = 30 * 45;  // 1350 units
        CPPUNIT_ASSERT(stc3 >= 60000 && stc3 <= 60000 + tolerance3);
    }

    void testMultipleUpdatesAndRetrievals()
    {
        StcProvider provider;
        std::uint64_t baseTime = getCurrentTimestampMs() - 100; // Start 100ms in the past

        // Simulate a series of timestamp updates
        for (int i = 0; i < 10; ++i) {
            provider.processTimestamp(1000 * i, baseTime + i * 100);

            std::uint32_t stc = provider.getStc();

            // Each retrieval should give a reasonable value
            CPPUNIT_ASSERT(stc < 100000); // Sanity check

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // Final check
        std::uint32_t finalStc = provider.getStc();
        // Should be close to the last set value (9000) plus time delta
        // With accumulated delays, allow wider range
        CPPUNIT_ASSERT(finalStc >= 8000 && finalStc <= 15000);
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(StcProviderTest);
