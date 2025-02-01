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

#include "ScteRenderer.hpp"
#include "ScteOutliner.hpp"

#include <subttxrend/gfx/ColorArgb.hpp>
#include <subttxrend/gfx/Types.hpp>
#include <subttxrend/common/Logger.hpp>

#include <cassert>
namespace subttxrend
{
namespace scte
{

namespace
{

common::Logger g_logger("Scte", "ScteRenderer");

const int DEFAULT_WINDOW_W = 1920;
const int DEFAULT_WINDOW_H = 1080;

gfx::Rectangle dstRect{0, 0, DEFAULT_WINDOW_W, DEFAULT_WINDOW_H};

class Indexer
{
public:
    Indexer(Coords ft, Coords fb, size_t width):
        idx(ft.y * width + ft.x), line_begin(ft.y * width + ft.x), line_end(ft.y * width + fb.x), width(width)
    {
    }

    operator size_t () const
    {
        return idx;
    }

    void operator++()
    {
        idx++;
        if (idx >= line_end)
        {
            line_begin += width;
            line_end += width;
            idx = line_begin;
        }
    }
private:
    size_t idx;
    size_t width;
    size_t line_begin;
    size_t line_end;
};

}

Renderer::Renderer(gfx::Window* gfxWindow) :
        m_gfxWindow(gfxWindow)
{
    assert(gfxWindow);
}

void Renderer::update()
{
    m_gfxWindow->update();
}

void Renderer::clearscreen()
{
    m_gfxWindow->getDrawContext().fillRectangle(gfx::ColorArgb::TRANSPARENT, m_gfxWindow->getBounds());
}

unsigned Renderer::yuv2rgb(Color color)
{
   unsigned int pixel32;
   unsigned char *pixel = (unsigned char *)&pixel32;
   int r, g, b;

   auto y = float(color.y)/31.0 *255;
   auto u = float(color.cr)/31.0 *255;
   auto v = float(color.cb)/31.0 *255;

   r = y + (1.370705 * (v-128));
   g = y - (0.698001 * (v-128)) - (0.337633 * (u-128));
   b = y + (1.732446 * (u-128));

   r = std::max(std::min(r, 255), 0);
   g = std::max(std::min(g, 255), 0);
   b = std::max(std::min(b, 255), 0);

   pixel[0] = r * 220 / 256;
   pixel[1] = g * 220 / 256;
   pixel[2] = b * 220 / 256;
   pixel[3] = color.opaqueEnabled? 0xff: 0x80;

   return pixel32;
}


void Renderer::render(const SimpleBitmap& bm, size_t width, size_t height)
{
    auto size = width * height;
    auto& bitmap = bm.getBitmap().getRawData();

    std::uint32_t clut[COLOR_LAST];
    clut[COLOR_TRANSPARENT] = 0;
    clut[COLOR_CHARACTER] = yuv2rgb(bm.getCharacterColor());
    clut[COLOR_FRAME] = yuv2rgb(bm.getFrameColor());
    clut[COLOR_OUTLINE] =  yuv2rgb(bm.getOutlineColor());
    clut[COLOR_SHADOW] = yuv2rgb(bm.getShadowColor());

    auto bytemap = std::make_unique<std::uint8_t[]>(std::max(size, static_cast<size_t>(1)));
    std::fill_n(bytemap.get(), size, 0);

    auto framed = bm.getBackgroundStyle() == BackgroundStyle::FRAMED;

    if(framed)
    {
        int start = bm.getFrameTop().y * width + bm.getFrameTop().x;
        int lines = bm.getFrameBottom().y - bm.getFrameTop().y + 1;
        int length = bm.getFrameBottom().x - bm.getFrameTop().x + 1;

        for(int i=0; i<lines; i++)
        {
            memset(&bytemap[start+width*i], 2, length);
        }
    }

    auto top = bm.getCharacterTop();
    auto bottom = bm.getCharacterBottom();
    auto bgcolor = framed ? COLOR_FRAME: COLOR_TRANSPARENT;

    if (bm.getOutlineStyle() == OutlineStyle::DROP_SHADOW)
    {
        auto shR = bm.getShadowRight();
        auto shB = bm.getShadowBottom();
        Coords topLeft {static_cast<uint16_t>(top.x + shR), static_cast<uint16_t>(top.y + shB)};
        Coords bottomRight {static_cast<uint16_t>(bottom.x + shR), static_cast<uint16_t>(bottom.y + shB)};
        render(bitmap, bytemap.get(), topLeft, bottomRight, COLOR_SHADOW, bgcolor, width, size);
    }
    render(bitmap, bytemap.get(), top, bottom, COLOR_CHARACTER, bgcolor, width, size);

    if (bm.getOutlineStyle() == OutlineStyle::OUTLINE)
    {
        Outliner outliner(bytemap.get(), width, height);
        outliner.setRange(bm.getCharacterTop(), bm.getCharacterBottom());
        outliner.outline(bm.getOutlineThickness(), COLOR_OUTLINE);
    }

    gfx::ClutBitmap pixmap(width, height, width, bytemap.get(), clut, sizeof(clut)/sizeof(*clut));

    m_gfxWindow->getDrawContext().drawPixmap(pixmap, gfx::Rectangle(0, 0, width, height), dstRect);
    update();
}


void Renderer::render(const RawBitmap::Data& bitmap, uint8_t* bytemap, Coords topLeft, Coords bottomRight, uint8_t onColor, uint8_t offColor, size_t width, size_t size)
{
    Indexer idx(topLeft, bottomRight, width);
    if (idx >= size)
    {
        return;
    }
    for (auto& byte: bitmap)
    {
        if (byte)
            bytemap[idx] = onColor;
        else if (bytemap[idx] == COLOR_TRANSPARENT)
            bytemap[idx] = offColor;
        ++idx;
        if (idx >= size)
        {
            break;
        }
    }
}

void Renderer::show()
{
    g_logger.debug("%s - showing window: w:%d, h:%d" , __func__, DEFAULT_WINDOW_W, DEFAULT_WINDOW_H);

    m_gfxWindow->setSize(gfx::Size(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H));
    clearscreen();

    m_gfxWindow->setVisible(true);
    m_gfxWindow->update();
}

void Renderer::hide()
{
    g_logger.debug("%s - hiding window" , __func__);

    m_gfxWindow->setVisible(false);
    m_gfxWindow->update();
}

}   // namespace ttmlengine
}   // namespace subttxrend

