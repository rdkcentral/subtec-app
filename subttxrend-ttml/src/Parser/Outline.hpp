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

#include <subttxrend/gfx/ColorArgb.hpp>
#include "DomainValue.hpp"

namespace subttxrend
{
namespace ttmlengine
{

class Outline
{
public:
    Outline() = default;
    Outline(DomainValue thickness, const gfx::ColorArgb& color);

    const gfx::ColorArgb& getColor() const;
    const DomainValue& getThickness() const;

    void setColor(const gfx::ColorArgb& color);
    void setThickness(DomainValue thickness);

    friend bool operator==(const Outline& lhs, const Outline& rhs);

private:
    gfx::ColorArgb m_color {gfx::ColorArgb::BLACK};
    DomainValue m_thickness {DomainValue::Type::CELL_HUNDREDTHS, 0};
};

} // namespace subttxrend
} // namespace ttmlengine
