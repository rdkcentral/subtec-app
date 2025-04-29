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

#include "AttributeHandlers.hpp"
#include "DomainValue.hpp"
#include "Timing.hpp"
#include "Outline.hpp"

#include <subttxrend/gfx/ColorArgb.hpp>
#include <set>

namespace subttxrend
{
namespace ttmlengine
{

class StyleSet
{
public:
    enum class DisplayAlign
    {
        BEFORE,
        AFTER,
        CENTER
    };

    enum class TextAlign
    {
        LEFT,
        RIGHT,
        CENTER
    };

    virtual ~StyleSet() = default;

    const gfx::ColorArgb& getColor() const;
    const gfx::ColorArgb& getBackgroundColor() const;
    const std::string& getFontFamily() const;
    TextAlign getTextAlign() const;
    DisplayAlign getDisplayAlign() const;
    DomainValue getFontSize() const;
    DomainValue getLineHeight() const;
    const Outline& getOutline() const;
    const std::string& getStyleId() const;

    void setStyleId(const std::string& styleId);
    void merge(const Attributes& attributes);
    void setRegionInfo(const std::string region);

    std::string toStr();

    friend bool operator==(const StyleSet& lhs,
                           const StyleSet& rhs);

private:
    void parseAttribute(const std::string& name,
                        const std::string& value);

    gfx::ColorArgb m_color{gfx::ColorArgb::WHITE};
    gfx::ColorArgb m_backgroundColor{gfx::ColorArgb::TRANSPARENT};
    std::string m_fontFamily{"default"};
    DomainValue m_fontSize{DomainValue::Type::CELL_HUNDREDTHS, 100};
    TextAlign m_textAlign{TextAlign::CENTER};
    DisplayAlign m_displayAlign{DisplayAlign::BEFORE};
    DomainValue m_lineHeight{DomainValue::Type::PERCENTAGE_HUNDREDTHS, 100*100};
    Outline m_textOutline;
    std::string m_styleId;
    std::string m_region;
};

std::ostream& operator<<(std::ostream& out, const StyleSet::TextAlign textAlign);
std::ostream& operator<<(std::ostream& out, const StyleSet::DisplayAlign displayAlign);

} // namespace subttxrend
} // namespace ttmlengine

