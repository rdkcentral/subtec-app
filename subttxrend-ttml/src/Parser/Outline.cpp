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


#include "Outline.hpp"


namespace subttxrend
{
namespace ttmlengine
{

Outline::Outline(DomainValue thickness, const gfx::ColorArgb& color)
        : m_thickness{thickness},
          m_color{color}
{
}

const gfx::ColorArgb& Outline::getColor() const
{
    return m_color;
}

const DomainValue& Outline::getThickness() const
{
    return m_thickness;
}

void Outline::setColor(const gfx::ColorArgb& color)
{
    m_color = color;
}

void Outline::setThickness(DomainValue thickness)
{
    m_thickness = thickness;
}

bool operator==(const Outline& lhs, const Outline& rhs)
{
    return ((lhs.getColor() == rhs.getColor()) && (lhs.getThickness() == rhs.getThickness()));
}

} // namespace subttxrend
} // namespace ttmlengine
