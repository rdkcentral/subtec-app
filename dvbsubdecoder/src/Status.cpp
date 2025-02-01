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


#include "Status.hpp"

namespace dvbsubdecoder
{

Status::Status(Specification specVersion) :
        m_specVersion(specVersion),
        m_pageIdsSet(false),
        m_compositionPageId(0),
        m_ancillaryPageId(0)
{
    // nothing to do
}

void Status::setPageIds(std::uint16_t compositionPageId,
                        std::uint16_t ancillaryPageId)
{
    m_pageIdsSet = true;
    m_compositionPageId = compositionPageId;
    m_ancillaryPageId = ancillaryPageId;
}

bool Status::isSelectedPage(std::uint16_t pageId) const
{
    if (m_pageIdsSet)
    {
        if ((m_compositionPageId == pageId) || (m_ancillaryPageId == pageId))
        {
            return true;
        }
    }

    return false;
}

bool Status::isCompositionPage(std::uint16_t pageId) const
{
    if (m_pageIdsSet)
    {
        if (m_compositionPageId == pageId)
        {
            return true;
        }
    }

    return false;
}

bool Status::isAncillaryPage(std::uint16_t pageId) const
{
    if (m_pageIdsSet)
    {
        if (m_ancillaryPageId == pageId)
        {
            return true;
        }
    }

    return false;
}

} // namespace dvbsubdecoder
