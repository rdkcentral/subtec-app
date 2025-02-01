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


#ifndef SUBTTXREND_TTXT_BROWSER_RENDERER_IMPL_HPP_
#define SUBTTXREND_TTXT_BROWSER_RENDERER_IMPL_HPP_

#include "BrowserRenderer.hpp"
#include "RendererImpl.hpp"
#include "GfxTypes.hpp"

namespace subttxrend
{
namespace ttxt
{

class TimeSource;

/**
 * Renderer for Teletext Browser - implementation.
 */
class BrowserRendererImpl : public RendererImpl,
                            virtual public BrowserRenderer
{
public:
    /**
     * Constructor.
     */
    BrowserRendererImpl();

    /**
     * Destructor.
     */
    virtual ~BrowserRendererImpl();

    virtual void resetStartPage() override;

    virtual bool start(int initialMagazine, int initialPage) override;

};

} // namespace ttxt
} // namespace subttxrend

#endif /*SUBTTXREND_TTXT_BROWSER_RENDERER_IMPL_HPP_*/
