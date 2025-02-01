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


#include "GfxConfig.hpp"

namespace subttxrend
{
namespace ttxt
{

void GfxConfig::init(const common::ConfigProvider* configProvider)
{
    m_windowSizePixels.m_w = configProvider->getInt("WINDOW.W", 1280);
    m_windowSizePixels.m_h = configProvider->getInt("WINDOW.H", 720);

    m_gridCellSizePixels.m_w = configProvider->getInt("GRID_CELL.W", 22);
    m_gridCellSizePixels.m_h = configProvider->getInt("GRID_CELL.H", 26);

    m_fontInfoG0G2.m_name = configProvider->get("FONT_G0_G2.NAME",
            "Bitstream Vera Sans Mono Bold");
    m_fontInfoG0G2.m_glyphSize = m_gridCellSizePixels;
    m_fontInfoG0G2.m_charSize.m_w = configProvider->getInt("FONT_G0_G2.W", 34);
    m_fontInfoG0G2.m_charSize.m_h = configProvider->getInt("FONT_G0_G2.H", 24);

    const std::int32_t MIN_FLASH_PERIOD = 200;
    m_flashPeriodMs = configProvider->getInt("FLASH_PERIOD_MS", 1000);
    if (m_flashPeriodMs < MIN_FLASH_PERIOD)
    {
        m_flashPeriodMs = MIN_FLASH_PERIOD;
    }

    m_bgAlpha = configProvider->getInt("BG_ALPHA", 0xFF);
}

} // namespace ttxt
} // namespace subttxrend
