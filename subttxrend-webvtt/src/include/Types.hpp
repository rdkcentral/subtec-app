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


#pragma once

#include <ostream>

#include <subttxrend/gfx/Types.hpp>

namespace subttxrend {
namespace webvttengine {

struct Point {
    int x;
    int y;
    friend bool operator==(const Point& lhs,
                        const Point& rhs) {
        return (
            lhs.x == rhs.x &&
            lhs.y == rhs.y
            );
    }
};

struct Rectangle : public gfx::Rectangle {
    using gfx::Rectangle::Rectangle;
    using gfx::Rectangle::getSize;

    inline int left() const { return m_x; }
    inline int right() const { return m_x + m_w; }
    inline int top() const { return m_y; }
    inline int bottom() const { return m_y + m_h; }

    bool valueIsBetween(int point, int a, int b) {
        if (b > a) {
            return point >= a && point <= b;
        } else {
            return point >= b && point <= a;
        }
    }

    bool overlaps(const Rectangle &rect) {
        return (valueIsBetween(rect.left(), left(), right()) ||
                valueIsBetween(rect.right(), left(), right()) ||
                (rect.left() < left() && rect.right() > right())) 
                &&
                (valueIsBetween(rect.top(), bottom(), top()) ||
                valueIsBetween(rect.bottom(), bottom(), top()) ||
                (rect.top() > top() && rect.bottom() > bottom()));
    }

    friend std::ostream& operator<<(std::ostream &os, const Rectangle& rect) {
        return os << "{(" << rect.m_x << "," << rect.m_y << ") " << rect.m_w << "x" << rect.m_h << "}";
    }
};
}
}
