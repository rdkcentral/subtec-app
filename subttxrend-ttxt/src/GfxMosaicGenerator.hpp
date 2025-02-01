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


#ifndef SUBTTXREND_TTXT_MOSAIC_GENERATOR_HPP_
#define SUBTTXREND_TTXT_MOSAIC_GENERATOR_HPP_

#include <utility>

#include <ttxdecoder/Engine.hpp>
#include <subttxrend/gfx/Engine.hpp>
#include <subttxrend/gfx/FontStrip.hpp>

namespace subttxrend
{
namespace ttxt
{

/**
 * Generator for mosaics.
 */
class GfxMosaicGenerator
{
public:
    /**
     * Generates block mosaic font strip.
     *
     * @param ttxEngine
     *      Teletext engine to use.
     * @param gfxEngine
     *      Graphics engine to use.
     *
     * @return
     *      Generated font strip & map.
     */
    static std::pair<gfx::FontStripPtr, gfx::FontStripMap> generateStripG1(const ttxdecoder::Engine& ttxEngine,
                                                                           const gfx::EnginePtr& gfxEngine);

};

} // namespace ttxt
} // namespace subttxrend

#endif /*SUBTTXREND_TTXT_MOSAIC_GENERATOR_HPP_*/
