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


#include "TtmlRenderer.hpp"
#include <subttxrend/gfx/Base64ToPixmap.hpp>
#include <subttxrend/gfx/ColorArgb.hpp>
#include <subttxrend/gfx/Types.hpp>

#include <subttxrend/common/Logger.hpp>

namespace subttxrend
{
namespace ttmlengine
{

namespace {

const gfx::Size MAX_SURFACE_SIZE{1280, 720};
// 1x1 is special case for UI
const gfx::Size MIN_SURFACE_SIZE{2, 2};

bool isFullScreen(gfx::Rectangle const& rect, gfx::Size const& windowSize)
{
    return (rect.m_x == 0)
       and (rect.m_y == 0)
       and (rect.m_w == windowSize.m_w)
       and (rect.m_h == windowSize.m_h);
}

} // anonymous

TtmlRenderer::TtmlRenderer(const common::ConfigProvider *configProvider,
                           gfx::Window* gfxWindow,
                           const DataDumper& dataDumper)
        : m_gfxWindow(gfxWindow),
          m_docDrawer(configProvider,m_valueConverter),
          m_dataDumper(dataDumper)
{
    assert(m_gfxWindow);
}

void TtmlRenderer::setRelatedVideoSize(gfx::Size relatedVideoSize)
{
    // size from  IPPlayer -> size of max video representation
    m_valueConverter.setSourceSize(relatedVideoSize);

    if ((relatedVideoSize.m_h > MAX_SURFACE_SIZE.m_h)
            or (relatedVideoSize.m_w > MAX_SURFACE_SIZE.m_w))
    {
        m_surfaceSize = MAX_SURFACE_SIZE;
    }
    else
    {
        m_surfaceSize = relatedVideoSize;
    }
    m_valueConverter.setDrawingSize(m_surfaceSize);
}

void TtmlRenderer::update()
{
    m_gfxWindow->update();
}

void TtmlRenderer::clearscreen()
{
    m_gfxWindow->clear();
}

void TtmlRenderer::renderDocument(IntermediateDocument &doc)
{
    decodeImages(doc);
    resizeWindow(doc);
    m_valueConverter.setCellResolution(doc.m_cellResolution);

    m_docDrawer.draw(doc, m_gfxWindow->getDrawContext());
}

void TtmlRenderer::decodeImages(IntermediateDocument &doc)
{
    for (auto &entity : doc.m_entites) {
        if (entity.m_imageChunk.m_image && entity.m_imageChunk.m_image->getId() != "") {
            auto b64 = entity.m_imageChunk.m_image->getBase64Data();
            auto pngCallback = preparePngCallback(entity.m_imageChunk.m_image->getId(), doc.m_timing.toStr());

            entity.m_imageChunk.m_bmp = gfx::base64toPixmap(*b64, true, pngCallback);
        }
    }
}

void TtmlRenderer::resizeWindow(IntermediateDocument& doc)
{
    for (auto& entity : doc.m_entites)
    {
        if (entity.m_imageChunk.m_bmp)
        {
            gfx::Rectangle dest = m_valueConverter.toTargetRectangle(*entity.m_region);
            gfx::Size windowSize = m_gfxWindow->getSize();
            auto const bmpSize = entity.m_imageChunk.m_bmp->getSize();

            if (isFullScreen(dest, m_surfaceSize))
            {
                if (m_surfaceSize != bmpSize) {
                    m_logger.osinfo("surface size different than bmp size:", m_surfaceSize, " vs:", bmpSize);

                    m_surfaceSize = bmpSize;
                    m_valueConverter.setSourceSize(m_surfaceSize);
                    m_valueConverter.setDrawingSize(m_surfaceSize);
                }
            }
            else
            {
                auto const destSize = dest.getSize();
                if (destSize != bmpSize)
                {
                    m_logger.oswarning("sw scaling: bmp:", bmpSize, " window:", windowSize, " dest:", destSize);
                }
            }
        }
        if (m_surfaceSize != m_gfxWindow->getSize())
        {
            m_gfxWindow->setSize(m_surfaceSize);
        }
    }
}

void TtmlRenderer::show()
{
    // set surface size to minimum viable size to prevent draws on not yet scaled surface visible
    // context: surface resize is done just before drawing, resize triggers UI to update compositor with
    // corrected position and scaling to make subtitle surface fullscreen. If compositor is not yet
    // updated it causes subtitles to appear in wrong position and wrong size.
    // m_gfxWindow->setSize(MIN_SURFACE_SIZE);
    m_gfxWindow->setVisible(true);
    //m_gfxWindow->update();
}

void TtmlRenderer::hide()
{
    m_gfxWindow->setVisible(false);
    //m_gfxWindow->update();
}

void TtmlRenderer::clearState()
{
    m_docDrawer.clearState();
}

gfx::PngCallback TtmlRenderer::preparePngCallback(const std::string &id,
                                                  const std::string &timing)
{
    gfx::PngCallback pngCallback = nullptr;
    if (m_dataDumper.imageDumpEnabled())
    {
        std::string dumpFileName(id);
        dumpFileName.append("_");
        dumpFileName.append(timing);

        std::replace(dumpFileName.begin(), dumpFileName.end(), ':', '_');
        std::replace(dumpFileName.begin(), dumpFileName.end(), '.', '_');

        dumpFileName.append(".png");

        const DataDumper &dataDumper = m_dataDumper;
        pngCallback = [dumpFileName, &dataDumper](const std::uint8_t* buffer, std::size_t bufferSize) -> void {
            dataDumper.toFile(dumpFileName, buffer, bufferSize);
        };
    }
    return pngCallback;
}

}   // namespace ttmlengine
}   // namespace subttxrend

