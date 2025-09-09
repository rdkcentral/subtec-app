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


#include <cmath>
#include <sstream>
#include <subttxrend/common/Logger.hpp>

#include "ValueConverter.hpp"

namespace subttxrend
{
namespace ttmlengine
{

namespace
{
common::Logger logger("TtmlEngine", "ValueConverter");
}

gfx::Size const ValueConverter::MAX_SURFACE_SIZE{1920, 1080};

gfx::Size ValueConverter::setSourceSize(gfx::Size const& sourceSize)
{
    srcSize = sourceSize;
    if ((sourceSize.m_h > MAX_SURFACE_SIZE.m_h) || (sourceSize.m_w > MAX_SURFACE_SIZE.m_w))
    {
        drawingSize = MAX_SURFACE_SIZE;
    }
    else
    {
        drawingSize = sourceSize;
    }
    updateScalingFactors();

    logger.info("%s source: %ux%u drawing: %ux%u scaling[hor: %f ver: %f])",
        __LOGGER_FUNC__,
        srcSize.m_w,
        srcSize.m_h,
        drawingSize.m_w,
        drawingSize.m_h,
        horizontalScalingFactor,
        verticalScalingFactor);
    return drawingSize;
}

void ValueConverter::setCellResolution(gfx::Size cellResolution)
{
    assert(cellResolution.m_h);
    assert(drawingSize.m_h);

    cellHeight = drawingSize.m_h / cellResolution.m_h;

    logger.trace("%s cellResolution: (%ux%u) cellHeight: %d",
        __LOGGER_FUNC__,
        cellResolution.m_w,
        cellResolution.m_h,
        cellHeight);
}

int ValueConverter::getCellHeight() const
{
    assert(cellHeight);
    return cellHeight;
}

gfx::Rectangle ValueConverter::toTargetRectangle(DomainValue x,
                                 DomainValue y,
                                 DomainValue width,
                                 DomainValue height) const
{
    auto rect = gfx::Rectangle(widthToPixels(x), heightToPixels(y), widthToPixels(width), heightToPixels(height));
    logger.trace("%s rect: (%d, %d, %d %d) -> (%d, %d, %d %d))",
        __LOGGER_FUNC__,
        x.getValue(), y.getValue(), width.getValue(), height.getValue(),
        rect.m_x, rect.m_y, rect.m_w, rect.m_h);
    return rect;
}

gfx::Rectangle ValueConverter::toTargetRectangle(const RegionElement& region) const
{
    return toTargetRectangle(region.getX(),
        region.getY(),
        region.getWidth(),
        region.getHeight());
}

int ValueConverter::sizeToPixels(DomainValue value, int relativeSizeInPixels) const
{
    int pixelSize = FAILSAFE_SIZE;
    const int PixelDefault = (drawingSize.m_h / DEFAULT_CELL_RESOLUTION.m_h);
    switch (value.getType())
    {
        case DomainValue::Type::PERCENTAGE_HUNDREDTHS:
        {
            pixelSize = std::lround(relativeSizeInPixels*value.getValue() / 10000.0);
            break;
        }
        case DomainValue::Type::PIXEL:
        {
            pixelSize = std::lround(value.getValue() * verticalScalingFactor);
            break;
        }
        case DomainValue::Type::CELL_HUNDREDTHS:
        {
            pixelSize = std::lround(value.getValue() * getCellHeight() / 100.0);
            break;
        }
        default:
        {
            logger.error("%s value type (%d) not supported, using default size",
            __LOGGER_FUNC__, static_cast<int>(value.getType()));
            break;
        }
    }

    //Set a max text height at 1.5 times the TTML default for badly formed Viper TTML content
    if (pixelSize > static_cast<int>(PixelDefault * 1.5))
    {
        logger.debug("%s pixelSize %d trimmed to %d", __LOGGER_FUNC__, pixelSize, PixelDefault);
        pixelSize = PixelDefault;
    }

    logger.debug("%s size %d (from %d)", __LOGGER_FUNC__, pixelSize, static_cast<int>(value.getValue()));

    return pixelSize;
}

void ValueConverter::updateScalingFactors()
{
    if ((srcSize.m_w == 0) || (srcSize.m_h == 0))
    {
        logger.error("%s srcSize %ux%u invalid, scaling disabled", __LOGGER_FUNC__, srcSize.m_w, srcSize.m_h);
        horizontalScalingFactor = 1;
        verticalScalingFactor = 1;
    }
    else
    {
        horizontalScalingFactor = (static_cast<float>(drawingSize.m_w) / srcSize.m_w);
        verticalScalingFactor = (static_cast<float>(drawingSize.m_h) / srcSize.m_h);
    }
}

int ValueConverter::widthToPixels(DomainValue value) const
{
    return convert(value, drawingSize.m_w, horizontalScalingFactor);
}

int ValueConverter::heightToPixels(DomainValue value) const
{
    return convert(value, drawingSize.m_h, verticalScalingFactor);
}

int ValueConverter::convert(DomainValue value,
                            std::int32_t dimensionValue,
                            float scalingFactor) const
{
    int convertedValue = 0;
    switch (value.getType())
    {
        case DomainValue::Type::PERCENTAGE_HUNDREDTHS:
        {
            convertedValue = std::lround(value.getValue() * dimensionValue / 10000.0);
            break;
        }
        case DomainValue::Type::PIXEL:
        {
            convertedValue = std::lround(value.getValue() * scalingFactor);
            break;
        }
        case DomainValue::Type::CELL_HUNDREDTHS:
        default:
        {
            logger.warning("%s value (%d) not supported for dimension",
            __LOGGER_FUNC__, static_cast<int>(value.getValue()));
            break;
        }
    }
    logger.debug("%s to %d from %d)", __LOGGER_FUNC__, convertedValue, static_cast<int>(value.getValue()));

    return convertedValue;
}

} // namespace ttmlengine
} // namespace subttxrend

