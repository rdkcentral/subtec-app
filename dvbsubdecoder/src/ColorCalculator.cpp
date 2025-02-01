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


#include "ColorCalculator.hpp"

#include <algorithm>

namespace dvbsubdecoder
{

namespace
{

const std::int8_t SCALE_BITS = 10;

const std::int32_t SCALE_VALUE = (1 << (SCALE_BITS));
const std::int32_t SCALE_VALUE_HALF = (1 << (SCALE_BITS - 1));

const std::int32_t Y_FULL_TRANSPARENT = 0;
const std::int32_t Y_RANGE_MIN = 16;
const std::int32_t Y_RANGE_MAX = 235;
const std::int32_t CX_RANGE_MIN = 16;
const std::int32_t CX_RANGE_MAX = 240;

const std::int32_t RGB_RANGE_MIN = 0;
const std::int32_t RGB_RANGE_MAX = 255;

std::int32_t floatToScaledInt(float coef)
{
    float value = (coef * SCALE_VALUE + 0.5f);
    return static_cast<int>(value);
}

std::int32_t scaledIntToInt(std::int32_t value)
{
    return (value + SCALE_VALUE_HALF) >> SCALE_BITS;
}

std::int32_t clampValue(std::int32_t value,
                        const std::int32_t rangeMin,
                        std::int32_t rangeMax)
{
    return std::max(rangeMin, std::min(rangeMax, value));
}

} // namespace anonymous

ColorARGB ColorCalculator::toARGB(const ColorYCrCbT ycrcbt)
{
    std::int32_t valueY = ycrcbt.m_y;
    std::int32_t valueCr = ycrcbt.m_cr;
    std::int32_t valueCb = ycrcbt.m_cb;
    std::int32_t valueT = ycrcbt.m_t;

    // fully transparent white
    std::int32_t valueA = 0x00;
    std::int32_t valueR = 0xFF;
    std::int32_t valueG = 0xFF;
    std::int32_t valueB = 0xFF;

    // values ranges:
    static const std::int32_t Y_COEF = floatToScaledInt(1.164f);
    static const std::int32_t R_CR_COEF = floatToScaledInt(1.596f);
    static const std::int32_t G_CR_COEF = floatToScaledInt(0.813f);
    static const std::int32_t G_CB_COEF = floatToScaledInt(0.392f);
    static const std::int32_t B_CB_COEF = floatToScaledInt(2.017f);

    // Conversion based on equations found here:
    // http://www.equasys.de/colorconversion.html
    // Similar equations are used by libav.

    if (valueY != Y_FULL_TRANSPARENT)
    {
        valueY = clampValue(valueY, Y_RANGE_MIN, Y_RANGE_MAX);
        valueCr = clampValue(valueCr, CX_RANGE_MIN, CX_RANGE_MAX);
        valueCb = clampValue(valueCb, CX_RANGE_MIN, CX_RANGE_MAX);

        valueY -= 16;
        valueCr -= 128;
        valueCb -= 128;

        std::int32_t partY = Y_COEF * valueY;
        std::int32_t partRCr = R_CR_COEF * valueCr;
        std::int32_t partGCr = G_CR_COEF * valueCr;
        std::int32_t partGCb = G_CB_COEF * valueCb;
        std::int32_t partBCb = B_CB_COEF * valueCb;

        std::int32_t valueRscaled = partY + partRCr;
        std::int32_t valueGscaled = partY - partGCr - partGCb;
        std::int32_t valueBscaled = partY + partBCb;

        valueA = 255 - valueT;
        valueR = scaledIntToInt(valueRscaled);
        valueG = scaledIntToInt(valueGscaled);
        valueB = scaledIntToInt(valueBscaled);

        valueA = clampValue(valueA, RGB_RANGE_MIN, RGB_RANGE_MAX);
        valueR = clampValue(valueR, RGB_RANGE_MIN, RGB_RANGE_MAX);
        valueG = clampValue(valueG, RGB_RANGE_MIN, RGB_RANGE_MAX);
        valueB = clampValue(valueB, RGB_RANGE_MIN, RGB_RANGE_MAX);
    }

    return
    {   static_cast<std::uint8_t>(valueA), static_cast<std::uint8_t>(valueR),
        static_cast<std::uint8_t>(valueG), static_cast<std::uint8_t>(valueB)};
}

} // namespace dvbsubdecoder
