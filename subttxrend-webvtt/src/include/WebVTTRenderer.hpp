/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2021 RDK Management
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
*/
#pragma once

#include <atomic>

#include <WebVTTConfig.hpp>
#include <WebVTTCue.hpp>
#include <LineBuilder.hpp>

#include <subttxrend/common/Logger.hpp>

#include <subttxrend/gfx/Base64ToPixmap.hpp>
#include <subttxrend/gfx/Types.hpp>
#include <subttxrend/gfx/Window.hpp>

namespace subttxrend
{
namespace webvttengine
{

using WinPtr = std::weak_ptr<gfx::Window>;

class WebVTTRenderer {
public:

    /**
     * Constructor.
     *
     * @param gfxWindow
     *      Window to draw on.
     */
    WebVTTRenderer(WinPtr gfxWindow, const WebVTTConfig &config);

    /**
     *  Sets releated video
     *
     * @param relatedVideoSize
     */
    void setRelatedVideoSize(gfx::Size relatedVideoSize);

    /**
     * Shows WebVTT window.
     */
    void show();

    /**
     * Hides WebVTT window.
     */
    void hide();

    /**
     * Renders given window WebVTT surface.
     *
     * @param doc
     *      Document to render.
     */
    void renderDocument(const CueSharedList &webvtt_list, const RegionMap &regions);

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

    /**
     * Set WebVTT attributes.
     *
     * @param attributes
     *      Attributes.
     */
    void setAttributes(const WebVTTAttributes &attributes);

private:
    void resizeWindow();

    WinPtr              m_gfxPtr;

    const gfx::Size     DEFAULT_SURFACE_SIZE{1280, 720};
    gfx::Size           m_surfaceSize{DEFAULT_SURFACE_SIZE};
    WebVTTConfig        m_config;

    std::atomic<bool>   m_reset;
    WebVTTAttributes    m_attributes;
};

}   // namespace webvttengine
}   // namespace subttxrend

