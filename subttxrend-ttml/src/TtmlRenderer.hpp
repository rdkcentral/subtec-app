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

#include "DataDumper.h"
#include "IntermediateDocDrawer.hpp"

#include "Parser/DocumentInstance.hpp"

#include <subttxrend/common/ConfigProvider.hpp>
#include <subttxrend/common/Logger.hpp>

#include <subttxrend/gfx/Base64ToPixmap.hpp>
#include <subttxrend/gfx/Types.hpp>
#include <subttxrend/gfx/Window.hpp>

namespace subttxrend
{
namespace ttmlengine
{

/**
 * TTML document renderer.
 */
class TtmlRenderer
{
public:

    TtmlRenderer(const common::ConfigProvider *configProvider,
                 gfx::Window *gfxWindow,
                 const DataDumper &dataDumper);

    /**
     *  Sets releated video
     *
     * @param relatedVideoSize
     */
    void setRelatedVideoSize(gfx::Size relatedVideoSize);

    /**
     * Shows ttml window.
     */
    void show();

    /**
     * Hides ttml window.
     */
    void hide();

    /**
     * Renders given window ttml surface.
     *
     * @param doc
     *      Document to render.
     */
    void renderDocument(IntermediateDocument& doc);

    /**
     * Clears screen.
     */
    void clearscreen();

    /**
     * Updates window context.
     */
    void update();

    /**
     * Resets instance to default state.
     */
    void clearState();

private:

    void decodeImages(IntermediateDocument &doc);
    void resizeWindow(IntermediateDocument& doc);

    /**
     * Debug feature - prepares png callback for image dumping.
     *
     * @param id
     *      Id of entity to get callback for.
     * @param timing
     *      Timing of entity to get callback for.
     *
     * @return
     *      Function to call with decoded image.
     */
    gfx::PngCallback preparePngCallback(const std::string& id,
                                        const std::string& timing);

    subttxrend::common::Logger m_logger{"TtmlEngine", "TtmlRenderer"};

    /** Window pointer. */
    gfx::Window* const m_gfxWindow;

    gfx::Size m_surfaceSize{ValueConverter::MAX_SURFACE_SIZE};

    /** Value converter. */
    ValueConverter m_valueConverter{};

    /** Document drawer. */
    IntermediateDocDrawer m_docDrawer;

    /** Debug data dumper. */
    const DataDumper& m_dataDumper;

};

}   // namespace ttmlengine
}   // namespace subttxrend

