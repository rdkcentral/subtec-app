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


#ifndef SUBTTXREND_TTXT_GFX_RENDERER_CLIENT_HPP_
#define SUBTTXREND_TTXT_GFX_RENDERER_CLIENT_HPP_

#include <cstdint>
#include <subttxrend/common/ConfigProvider.hpp>
#include <subttxrend/gfx/Window.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <ttxdecoder/Engine.hpp>

namespace subttxrend
{
namespace ttxt
{

/**
 * Client interface.
 */
class GfxRendererClient
{
public:
    /**
     * Constructor.
     *
     * @param subtitlesRenderer
     *      Subtitles renderer client flag.
     */
    GfxRendererClient(bool subtitlesRenderer) :
            m_subtitlesRenderer(subtitlesRenderer)
    {
        // noop
    }

    /**
     * Destructor.
     */
    virtual ~GfxRendererClient() = default;

    /**
     * Checks if it is subtitles renderer client.
     *
     * @return
     *      True if it is subtitles renderer client, false otherwise.
     */
    bool isSubtitlesRenderer() const
    {
        return m_subtitlesRenderer;
    }

    /**
     * Returns data source.
     *
     * @return
     *      Data source.
     */
    virtual const ttxdecoder::Engine* getDataSource() const = 0;

    /**
     * Returns configuration.
     *
     * @return
     *      Configuration provider.
     */
    virtual const common::ConfigProvider* getConfiguration() const = 0;

    /**
     * Returns window to draw on.
     *
     * @return
     *      Window.
     */
    virtual gfx::Window* getGfxWindow() const = 0;

    /**
     * Returns graphics engine.
     *
     * @return
     *      Graphics engine.
     */
    virtual const gfx::EnginePtr& getGfxEngine() const = 0;

    /**
     * Sets current page.
     *
     * @param pageId
     *      New current page.
     */
    virtual void setCurrentPage(const ttxdecoder::PageId& pageId) = 0;

private:
    /** Rendering mode flag. */
    const bool m_subtitlesRenderer;
};

} // namespace ttxt
} // namespace subttxrend

#endif /*SUBTTXREND_TTXT_GFX_RENDERER_CLIENT_HPP_*/
