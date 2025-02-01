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

namespace subttxrend
{
namespace ttmlengine
{

/**
 * Class holding value in different units, some absolute (pixels), some relative to other
 * media features (cells, video dimensions).
 */
class DomainValue
{
public:
    enum class Type {
        PERCENTAGE_HUNDREDTHS,     // percentages times 100 - 67,66% -> 6766
        PIXEL,
        CELL_HUNDREDTHS            // cell size times 100 - 0,8c - 80
    };

    DomainValue(DomainValue::Type type = DomainValue::Type::PIXEL,
                int value = 0)
            : m_type(type),
              m_value(value)
    {
        // noop
    }

    int getValue() const
    {
        return m_value;
    }

    /**
     * Depending on value type returns:
     * - PERCENTAGE_HUNDREDTHS - percentages times 100 - 67,66% -> 6766
     * - PIXEL - number of pixels
     * - CELL_HUNDREDTHS - cell size times 100 - 0,8c - 80
     *
     * @return
     *      Value held.
     */
    Type getType() const
    {
        return m_type;
    }

    friend bool operator==(const DomainValue& lhs,
                           const DomainValue& rhs)
    {
        return ((lhs.getType() == rhs.getType()) && (lhs.getValue() == rhs.getValue()));
    }

    friend std::ostream& operator<<(std::ostream& stream, DomainValue value)
    {
        switch(value.getType())
        {
            case DomainValue::Type::PERCENTAGE_HUNDREDTHS:
            {
                stream << value.getValue() / 100.0 << "%";
                break;
            }
            case DomainValue::Type::PIXEL:
            {
                stream << value.getValue() << "px";
                break;
            }
            case DomainValue::Type::CELL_HUNDREDTHS:
            {
                stream << value.getValue() / 100.0 << "c";
                break;
            }
        }
        return stream;
    }

private:
    Type m_type;
    int m_value;
};

} // namespace subttxrend
} // namespace ttmlengine

