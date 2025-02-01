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


#include "SubtitlesRendererImpl.hpp"

namespace subttxrend
{
namespace ttxt
{

SubtitlesRendererImpl::SubtitlesRendererImpl() :
        RendererImpl(true)
{
    // noop
}

SubtitlesRendererImpl::~SubtitlesRendererImpl()
{
    // noop
}

bool SubtitlesRendererImpl::start(std::uint32_t magazineNumber,
                                  std::uint32_t pageNumber)
{
    /** As per ETS 300 706 Enhanced Teletext specification, p 3.1:
        Magazine number 8:  A packet with a magazine value of 0 is referred to as belonging to magazine 8. */
    if (magazineNumber == 0)
    {
        magazineNumber = 8;
    }
    setCurrentPage(
            ttxdecoder::PageId(magazineNumber * 0x0100 + pageNumber,
                    ttxdecoder::PageId::ANY_SUBPAGE));
    return startInternal();
}

/**
 * Called when new header data is ready.
 */
void SubtitlesRendererImpl::headerReady()
{
    // nothing to do
}

} // namespace ttxt
} // namespace subttxrend
