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


#include "Blitter.hpp"

#include "ClutPixmap.hpp"
#include "ColorizedPixmap.hpp"
#include "Pixmap.hpp"

namespace subttxrend
{
namespace gfx
{

subttxrend::common::Logger Blitter::m_logger("Gfx", "Blitter");

const Blitter::DrawPosition Blitter::DrawPosition::CENTER(
        Blitter::DrawPosition::Mode::CENTER, 0, 0);

void Blitter::calculate(const Rectangle& srcPixRect,
                        const Rectangle& dstPixRect,
                        DrawPosition position,
                        ResizeMode resizeMode,
                        Rectangle& outSrcRect,
                        Rectangle& outDstRect)
{
    if ((srcPixRect.m_w == 0) || (srcPixRect.m_h == 0))
    {
        return;
    }

    if ((dstPixRect.m_w == 0) || (dstPixRect.m_h == 0))
    {
        return;
    }

    outSrcRect = srcPixRect;
    outDstRect = dstPixRect;

    if ((srcPixRect.m_w > 0xFFFF) || (srcPixRect.m_h > 0xFFFF))
    {
        m_logger.osfatal(__LOGGER_FUNC__, " - size larger than ", 0xffff, " not supported");
        return;
    }
    if ((dstPixRect.m_w > 0xFFFF) || (dstPixRect.m_h > 0xFFFF))
    {
        m_logger.osfatal(__LOGGER_FUNC__, " - size larger than ",0xffff," not supported");
        return;
    }

    switch (position.m_mode)
    {
    case DrawPosition::Mode::ABSOLUTE:
    {
        outDstRect.m_x += position.m_x;
        outDstRect.m_w -= position.m_x;

        outDstRect.m_y += position.m_y;
        outDstRect.m_h -= position.m_y;
        break;
    }
    case DrawPosition::Mode::CENTER:
        // nothing to do
        break;
    }

    const int RATIO_SHIFT = 16;

    switch (resizeMode)
    {
    case ResizeMode::NO_RESIZE:
    {
        outDstRect.m_w = outSrcRect.m_w;
        outDstRect.m_h = outSrcRect.m_h;

        // out-of-bounds are safe as drawing will fix them

        break;
    }
    case ResizeMode::UPDOWN:
    {
        int32_t wScale = outDstRect.m_w;
        wScale <<= RATIO_SHIFT;
        wScale /= outSrcRect.m_w;

        int32_t hScale = outDstRect.m_h;
        hScale <<= RATIO_SHIFT;
        hScale /= outSrcRect.m_h;

        int32_t hScalled = outSrcRect.m_h;
        hScalled *= hScale;
        hScalled >>= RATIO_SHIFT;

        int32_t wScalled = outSrcRect.m_w;
        wScalled *= wScale;
        wScalled >>= RATIO_SHIFT;

        outDstRect.m_h = hScalled;
        outDstRect.m_w = wScalled;

        break;
    }
    case ResizeMode::UPDOWN_PRESERVE_ASPECT:
    {
        int32_t wScale = outDstRect.m_w;
        wScale <<= RATIO_SHIFT;
        wScale /= outSrcRect.m_w;

        int32_t hScale = outDstRect.m_h;
        hScale <<= RATIO_SHIFT;
        hScale /= outSrcRect.m_h;

        if (wScale <= hScale)
        {
            int32_t hScalled = outSrcRect.m_h;
            hScalled *= wScale;
            hScalled >>= RATIO_SHIFT;

            outDstRect.m_h = hScalled;
        }
        else
        {
            int32_t wScalled = outSrcRect.m_w;
            wScalled *= hScale;
            wScalled >>= RATIO_SHIFT;

            outDstRect.m_w = wScalled;
        }

        break;
    }
    }

    switch (position.m_mode)
    {
    case DrawPosition::Mode::ABSOLUTE:
    {
        // nothing to do
        break;
    }

    case DrawPosition::Mode::CENTER:
    {
        int xDiff = (dstPixRect.m_w - outDstRect.m_w) / 2;
        int yDiff = (dstPixRect.m_h - outDstRect.m_h) / 2;

        outDstRect.m_x += xDiff;
        outDstRect.m_y += yDiff;
        break;
    }
    }

}

} // namespace gfx
} // namespace subttxrend
