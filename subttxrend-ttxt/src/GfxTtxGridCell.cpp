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


#include "GfxTtxGridCell.hpp"

namespace subttxrend
{
namespace ttxt
{

GfxTtxGridCell::GfxTtxGridCell()
{
    clear(0);
}

GfxTtxGridCell::~GfxTtxGridCell()
{
    // noop
}

void GfxTtxGridCell::clear(std::uint8_t bg)
{
    setEnabled(false);
    setHidden(false);
    setCharacter('\0');
    setSize(1, 1);
    setColors(bg, bg);

    m_dirty = true;
}

void GfxTtxGridCell::setEnabled(bool enabled)
{
    m_dirty |= checkAndSet(m_enabled, enabled);

    if (m_hidden)
    {
        m_hidden = false;
        m_dirty = true;
    }
}

void GfxTtxGridCell::setHidden(bool hidden)
{
    m_hidden = true;
    m_dirty = true;
}

void GfxTtxGridCell::setCharacter(std::uint16_t ch)
{
    m_dirty |= checkAndSet(m_char, ch);
}

void GfxTtxGridCell::setSize(std::uint8_t xMultiplier,
                             std::uint8_t yMultiplier)
{
    m_dirty |= checkAndSet(m_xMultiplier, xMultiplier);
    m_dirty |= checkAndSet(m_yMultiplier, yMultiplier);
}

void GfxTtxGridCell::setColors(std::uint8_t fgColor,
                               std::uint8_t bgColor)
{
    m_dirty |= checkAndSet(m_fgColor, fgColor);
    m_dirty |= checkAndSet(m_bgColor, bgColor);
}

void GfxTtxGridCell::markChanged()
{
    m_dirty = true;
}

void GfxTtxGridCell::markChangedByColor(std::uint8_t color)
{
    m_dirty |= ((m_fgColor == color) || (m_bgColor == color));
}

bool GfxTtxGridCell::startRedraw() const
{
    if (!m_hidden && m_dirty)
    {
        m_dirty = false;
        return true;
    }
    return false;
}

} // namespace ttxt
} // namespace subttxrend
