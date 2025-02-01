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

#include <subttxrend/gfx/Types.hpp>
#include "Parser/DomainValue.hpp"
#include "Parser/Elements.hpp"

namespace subttxrend
{
namespace ttmlengine
{

class ValueConverter
{
public:

    void setDrawingSize(gfx::Size drawingSize);
    void setSourceSize(gfx::Size relatedVideoSize);
    void setCellResolution(gfx::Size sourceSize);
    int getCellHeight() const;

    gfx::Rectangle toTargetRectangle(DomainValue x, DomainValue y, DomainValue width, DomainValue height) const;
    gfx::Rectangle toTargetRectangle(const RegionElement& region) const;

    int sizeToPixels(DomainValue value, int relativeSizeInPixels) const;

private:

    void updateScalingFactors();

    int widthToPixels(DomainValue value) const;
    int heightToPixels(DomainValue value) const;

    int convert(DomainValue value, std::int32_t dimensionValue, float scalingFactor) const;

    // font size or line height to use when conversion could not be made
    static constexpr int FAILSAFE_SIZE = 32;

    int cellHeight{};

    gfx::Size drawingSize{};
    gfx::Size srcSize{};

    float verticalScalingFactor{1.0};
    float horizontalScalingFactor{1.0};
};

} // namespace ttmlengine
} // namespace subttxrend

