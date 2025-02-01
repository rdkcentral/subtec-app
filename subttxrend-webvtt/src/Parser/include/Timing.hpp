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

#pragma once

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <sstream>

namespace subttxrend
{
namespace webvttengine
{

struct Time
{
    int hours;
    int minutes;
    int seconds;
    int milliseconds;
};

using namespace std::chrono_literals;

/**
 * Class representing point in time.
 */
class TimePoint
{
public:

    TimePoint() = default;

    explicit TimePoint(const std::uint64_t timeMs)
            : timestamp(timeMs)
    {
        // noop
    }

    TimePoint(std::chrono::hours hours,
              std::chrono::minutes minutes,
              std::chrono::seconds seconds,
              std::chrono::milliseconds fraction = {})
    {
        // in case values are out of available range stick to defaults
        if (minutes >  59min)
            return;
        if (seconds > 60s)
            return; /* 60 to include leap second */

        timestamp = hours + minutes + seconds + fraction;
    }

    /**
     * Apply offset to the stored time
     *
     * @param offsetMs offset in ms to be added
     */
    void applyOffset(std::int64_t offsetMs)
    {
        timestamp += std::chrono::milliseconds{offsetMs};
    }
    
    

    /**
     * Equality operator.
     *
     * @param lhs
     * @param rhs
     * @return
     *      True of both instances contain the same values.
     */
    friend bool operator==(const TimePoint& lhs,
                           const TimePoint& rhs)
    {
        return lhs.timestamp == rhs.timestamp;
    }

    /**
     * Inequality operator.
     *
     * @param lhs
     * @param rhs
     * @return
     *      True class instances contain different values.
     */
    friend bool operator!=(const TimePoint& lhs,
                           const TimePoint& rhs)
    {
        return !(lhs == rhs);
    }

    /**
     * 'Less than' operator.
     *
     * @param lhs
     * @param rhs
     * @return
     *      True if first instance contain value prior to second one. False otherwise.
     */
    friend bool operator<(const TimePoint& lhs,
                          const TimePoint& rhs)
    {
        return lhs.timestamp < rhs.timestamp;
    }

    /**
     * 'Greater than' operator.
     *
     * @param lhs
     * @param rhs
     * @return
     *      True if first instance contain value later in time than second one. False otherwise.
     */
    friend bool operator>(const TimePoint& lhs,
                          const TimePoint& rhs)
    {
        return rhs < lhs;
    }

    /**
     * 'Less or equal than' operator.
     *
     * @param lhs
     * @param rhs
     * @return
     *      True if first instance contain value prior or equal to second one. False otherwise.
     */
    friend bool operator<=(const TimePoint& lhs,
                           const TimePoint& rhs)
    {
        return !(lhs > rhs);
    }

    /**
     * 'Greater or equal than' operator.
     *
     * @param lhs
     * @param rhs
     * @return
     *      True if first instance contain value later in time or equal than second one. False otherwise.
     */
    friend bool operator>=(const TimePoint& lhs,
                           const TimePoint& rhs)
    {
        return !(lhs < rhs);
    }
    
    static std::ostream& getOstream(std::ostream &os, std::chrono::milliseconds timestamp) {
        using namespace std::chrono;
        auto ms = timestamp;
        auto hrs = duration_cast<hours>(ms);
        ms -= duration_cast<milliseconds>(hrs);
        auto mins = duration_cast<minutes>(ms);
        ms -= duration_cast<milliseconds>(mins);
        auto secs = duration_cast<seconds>(ms);
        ms -= duration_cast<milliseconds>(secs);

        os << std::setfill('0') << std::setw(2) << hrs.count() << ":";
        os << std::setfill('0') << std::setw(2) << mins.count() << ":";
        os << std::setfill('0') << std::setw(2) << secs.count() << ".";
        os << std::setfill('0') << std::setw(3) << ms.count();
        return os;
    }

    /**
     * Returns string representation of the object. Using for debugging purposes.
     *
     * @return
     *      String representation of the object.
     */
    friend std::ostream& operator<<(std::ostream &os, const TimePoint &tp) {
        return getOstream(os, tp.timestamp);
    }

    /**
     * Returns string representation of the object. Using for debugging purposes.
     *
     * @return
     *      String representation of the object.
     */
    friend std::ostream& operator<<(std::ostream &os, TimePoint &tp) {
        return getOstream(os, tp.timestamp);
    }

    std::chrono::milliseconds toMilliseconds() const
    {
        return timestamp;
    }

private:
    std::chrono::milliseconds timestamp{};
};

/**
 * Represents
 */
class Timing
{
public:
    /**
     * Constructor.
     */
    Timing() = default;

    /**
     * Constructor.
     */
    Timing(const TimePoint& begin,
           const TimePoint& end) :
            m_begin(begin),
            m_end(end)
    {
        // noop
    }

    /**
     * Start time getter.
     *
     * @return
     *      Start time.
     */
    TimePoint& getStartTimeRef()
    {
        return m_begin;
    }
    const TimePoint& getStartTimeRef() const
    {
        return m_begin;
    }

    /**
     * End time getter.
     *
     * @return
     *      End time.
     */
    TimePoint& getEndTimeRef()
    {
        return m_end;
    }
    const TimePoint& getEndTimeRef() const
    {
        return m_end;
    }

    /**
     * Checks if two objects are overlapping.
     *
     * @param other
     *      Another object to compare.
     * @return
     *      True if time periods overlap.
     */
    bool isOverlapping(const Timing& other) const
    {
        return std::max(m_begin, other.m_begin) < std::min(m_end, other.m_end);
    }

    /**
     * Equality operator.
     *
     * @param lhs
     * @param rhs
     * @return
     *      True of both instances contain the same values.
     */
    friend bool operator==(const Timing& lhs,
                           const Timing& rhs)
    {
        return (lhs.m_begin == rhs.m_begin) && (lhs.m_end == rhs.m_end);
    }

    /**
     * 'Less than' operator.
     *
     * @param lhs
     * @param rhs
     * @return
     *      True if first time instance starts before the second one. False otherwise.
     */
    friend bool operator<(const Timing& lhs,
                          const Timing& rhs)
    {
        return ((lhs.m_begin < rhs.m_begin) || ((lhs.m_begin == rhs.m_begin) && (lhs.m_end < rhs.m_end)));
    }

    /**
     * Apply offset to the stored start / end
     *
     * @param offsetMs offset in ms to be added
     */
    void applyOffset(std::int64_t offsetMs)
    {
        m_begin.applyOffset(offsetMs);
        m_end.applyOffset(offsetMs);
    }

    /**
     * Returns string representation of the object. Using for debugging purposes.
     *
     * @return
     *      String representation of the object.
     */    
    friend std::ostream& operator<<(std::ostream &os, Timing &t) {
        os << t.m_begin << "-" << t.m_end;
        return os;
    }

private:

    /** Start time. */
    TimePoint m_begin;

    /** End time. */
    TimePoint m_end;
};

} // namespace subttxrend
} // namespace webvttengine
