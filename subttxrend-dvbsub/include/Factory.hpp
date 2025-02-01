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


#ifndef SUBTTXREND_DVBSUB_FACTORY_HPP_
#define SUBTTXREND_DVBSUB_FACTORY_HPP_

#include <memory>

#include "SubtitlesRenderer.hpp"

namespace subttxrend
{
namespace dvbsub
{

/**
 * Factory for creating DVB SUB elements.
 */
class Factory
{
    Factory() = delete;

public:
    /**
     * Creates subtitles renderer.
     *
     * @return
     *      Created renderer.
     */
    static std::unique_ptr<SubtitlesRenderer> createSubtitlesRenderer();
};

} // namespace dvbsub
} // namespace subttxrend

#endif /*SUBTTXREND_DVBSUB_FACTORY_HPP_*/
